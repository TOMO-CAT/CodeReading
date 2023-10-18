// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#pragma once

#include <pthread.h>

#include <functional>
#include <memory>
#include <string>

#include "muduo/base/Atomic.h"
#include "muduo/base/CountDownLatch.h"
#include "muduo/base/Types.h"

namespace muduo {

/**
 * @brief 抽象出一个 Thread 类型
 *
 */
class Thread : noncopyable {
 public:
  typedef std::function<void()> ThreadFunc;

 public:
  explicit Thread(ThreadFunc, const string& name = string());
  ~Thread();

 public:
  void start();
  int join();

  bool started() const {
    return started_;
  }
  pid_t tid() const {
    return tid_;
  }
  const string& name() const {
    return name_;
  }

  static int numCreated() {
    return numCreated_.get();
  }

 private:
  void setDefaultName();

  bool started_;
  bool joined_;
  pthread_t pthreadId_;
  pid_t tid_;
  ThreadFunc func_;
  std::string name_;
  CountDownLatch latch_;

 private:
  static AtomicInt32 numCreated_;
};

}  // namespace muduo
