// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#pragma once

#include <memory>
#include <string>

#include "muduo/base/Mutex.h"
#include "muduo/base/Types.h"

namespace muduo {

namespace FileUtil {
class AppendFile;
}

/**
 * @brief 用于将日志写入磁盘文件
 *
 */
class LogFile : noncopyable {
 public:
  /**
   * @brief Construct a new Log File object
   *
   * @param basename 日志文件名前缀
   * @param rollSize 配置按照文件大小的滚动日志, file_ 中的 buffer_ 数据超过该值就会进行日志切割
   * @param threadSafe 同步日志需要加锁, 因为日志可能来自多个线程, 异步日志无需加锁
   * @param flushInterval
   * @param checkEveryN
   */
  LogFile(const string& basename, off_t rollSize, bool threadSafe = true, int flushInterval = 3,
          int checkEveryN = 1024);
  ~LogFile();

 public:
  /**
   * @brief 添加日志
   *
   * @param logline
   * @param len
   */
  void append(const char* logline, int len);
  /**
   * @brief 把文件缓冲区的日志强制写入到文件中
   *
   */
  void flush();
  /**
   * @brief 日志切割, 按照 rollSize 配置的大小滚动日志, 同时每天零点也会创建新的日志
   *
   * @return true
   * @return false
   */
  bool rollFile();

 private:
  void append_unlocked(const char* logline, int len);

  static string getLogFileName(const string& basename, time_t* now);

  const string basename_;
  const off_t rollSize_;
  const int flushInterval_;
  const int checkEveryN_;

  int count_;

  std::unique_ptr<MutexLock> mutex_;
  time_t startOfPeriod_;  // 记录前一天时间零点的秒数, 用于表明今天已经创建了日志文件
  time_t lastRoll_;       // 上次日志切割的时间
  time_t lastFlush_;      // 记录上次 flush 的时间, 配合 checkEveryN_ 进行周期性 flush
  std::unique_ptr<FileUtil::AppendFile> file_;

  static const int kRollPerSeconds_ = 60 * 60 * 24;
};

}  // namespace muduo
