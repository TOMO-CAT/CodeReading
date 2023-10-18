// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#pragma once

#include <pthread.h>

#include "muduo/base/Mutex.h"

namespace muduo {

/**
 * @brief 封装条件变量
 *
 */
class Condition : noncopyable {
 public:
  explicit Condition(MutexLock& mutex) : mutex_(mutex) {
    MCHECK(pthread_cond_init(&pcond_, NULL));
  }

  ~Condition() {
    MCHECK(pthread_cond_destroy(&pcond_));
  }

 public:
  void wait() {
    MutexLock::UnassignGuard ug(mutex_);
    MCHECK(pthread_cond_wait(&pcond_, mutex_.getPthreadMutex()));
  }

  // returns true if time out, false otherwise.
  bool waitForSeconds(double seconds);

  void notify() {
    MCHECK(pthread_cond_signal(&pcond_));
  }

  void notifyAll() {
    MCHECK(pthread_cond_broadcast(&pcond_));
  }

 private:
  MutexLock& mutex_;
  pthread_cond_t pcond_;
};

}  // namespace muduo
