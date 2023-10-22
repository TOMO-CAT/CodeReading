// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "muduo/base/LogFile.h"

#include <assert.h>
#include <stdio.h>
#include <time.h>

#include "muduo/base/FileUtil.h"
#include "muduo/base/ProcessInfo.h"

using muduo::LogFile;

LogFile::LogFile(const string& basename, off_t rollSize, bool threadSafe, int flushInterval, int checkEveryN)
    : basename_(basename),
      rollSize_(rollSize),
      flushInterval_(flushInterval),
      checkEveryN_(checkEveryN),
      count_(0),
      mutex_(threadSafe ? new MutexLock : NULL),
      startOfPeriod_(0),
      lastRoll_(0),
      lastFlush_(0) {
  assert(basename.find('/') == string::npos);
  rollFile();  // 创建新日志文件
}

LogFile::~LogFile() = default;

/**
 * @brief 添加日志, 异步日志不需加锁, 同步日志需要加锁
 *
 * @param logline
 * @param len
 */
void LogFile::append(const char* logline, int len) {
  if (mutex_) {
    MutexLockGuard lock(*mutex_);
    append_unlocked(logline, len);
  } else {
    append_unlocked(logline, len);
  }
}

void LogFile::flush() {
  if (mutex_) {
    MutexLockGuard lock(*mutex_);
    file_->flush();
  } else {
    file_->flush();
  }
}

/**
 * @brief 往 file_ 中写入日志
 *
 * @param logline
 * @param len
 */
void LogFile::append_unlocked(const char* logline, int len) {
  file_->append(logline, len);

  // 如果当前累计的日志超过配置的 rollSize_, 进行日志切割
  if (file_->writtenBytes() > rollSize_) {
    rollFile();
  } else {
    ++count_;
    if (count_ >= checkEveryN_) {
      count_ = 0;
      time_t now = ::time(NULL);
      // 计算当前整点的秒数, 如果不等于 startOfPeriod_ 说明开始了新的一天, 进行日志切割
      time_t thisPeriod_ = now / kRollPerSeconds_ * kRollPerSeconds_;
      if (thisPeriod_ != startOfPeriod_) {
        rollFile();
      } else if (now - lastFlush_ > flushInterval_) {
        lastFlush_ = now;
        file_->flush();
      }
    }
  }
}

bool LogFile::rollFile() {
  time_t now = 0;
  string filename = getLogFileName(basename_, &now);

  // 当天零点对应的秒数
  time_t start = now / kRollPerSeconds_ * kRollPerSeconds_;

  if (now > lastRoll_) {
    lastRoll_ = now;
    lastFlush_ = now;
    startOfPeriod_ = start;                           // 表明今天已经创建了新文件
    file_.reset(new FileUtil::AppendFile(filename));  // 创建新文件
    return true;
  }
  return false;
}

/**
 * @brief 获取日志文件名和当前时间
 *
 * @param basename
 * @param now
 * @return muduo::string
 */
// 文件名格式: logfile_test.20231019-150731.hostname.1537089.log
// * `logfile_test`：进程名称，容易区分是哪个服务进程的日志
// * `20231019-150731`：文件创建时间（GMT 时区）
// * `hostname`：机器名称，这样即使把日志文件拷贝到其他机器也可以追溯其来源
// * `1537089`：进程 ID，这样如果服务在短时间内反复重启就会生成多份日志文件
// * `log`：统一的后缀，方便周边配套脚本的编写
muduo::string LogFile::getLogFileName(const string& basename, time_t* now) {
  string filename;
  filename.reserve(basename.size() + 64);
  filename = basename;

  char timebuf[32];
  struct tm tm;
  *now = time(NULL);
  gmtime_r(now, &tm);
  strftime(timebuf, sizeof timebuf, ".%Y%m%d-%H%M%S.", &tm);
  filename += timebuf;

  filename += ProcessInfo::hostname();

  char pidbuf[32];
  snprintf(pidbuf, sizeof pidbuf, ".%d", ProcessInfo::pid());
  filename += pidbuf;

  filename += ".log";

  return filename;
}
