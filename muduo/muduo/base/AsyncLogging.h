// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#pragma once

#include <atomic>
#include <memory>
#include <string>
#include <vector>

#include "muduo/base/BlockingQueue.h"
#include "muduo/base/BoundedBlockingQueue.h"
#include "muduo/base/CountDownLatch.h"
#include "muduo/base/LogStream.h"
#include "muduo/base/Mutex.h"
#include "muduo/base/Thread.h"

namespace muduo {

class AsyncLogging : noncopyable {
 public:
  AsyncLogging(const string& basename, off_t rollSize, int flushInterval = 3);

  ~AsyncLogging() {
    if (running_) {
      stop();
    }
  }

 public:
  void append(const char* logline, int len);

  void start() {
    running_ = true;
    thread_.start();
    latch_.wait();
  }

  void stop() NO_THREAD_SAFETY_ANALYSIS {
    running_ = false;
    cond_.notify();
    thread_.join();
  }

 private:
  void threadFunc();

  typedef muduo::detail::FixedBuffer<muduo::detail::kLargeBuffer> Buffer;  // 4M
  typedef std::vector<std::unique_ptr<Buffer>> BufferVector;
  typedef BufferVector::value_type BufferPtr;

  const int flushInterval_;
  std::atomic<bool> running_;
  const string basename_;
  const off_t rollSize_;
  muduo::Thread thread_;
  muduo::CountDownLatch latch_;
  muduo::MutexLock mutex_;
  muduo::Condition cond_ GUARDED_BY(mutex_);
  BufferPtr currentBuffer_ GUARDED_BY(mutex_);  // 当前缓冲 4M
  BufferPtr nextBuffer_ GUARDED_BY(mutex_);     // 预备缓冲 4M
  BufferVector buffers_ GUARDED_BY(mutex_);     // 待写入文件的已填满的 buffer 列表
};

}  // namespace muduo
