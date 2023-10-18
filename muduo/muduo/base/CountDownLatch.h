// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#pragma once

#include "muduo/base/Condition.h"
#include "muduo/base/Mutex.h"

namespace muduo {

/**
 * @brief 倒计时, 是一种常用且易用的同步手段, 主要有两种用途:
 *   1) 主线程发起多个子线程, 等这些子线程都各自完成一定的任务后主线程才继续执行
 *   2) 主线程发起多个子线程,子线程都等待主线程完成其他一些任务后通知所有子线程执行
 *
 * @note 类似于 Golang 的 WaitGroup
 */
class CountDownLatch : noncopyable {
 public:
  // 设置倒数多少次
  explicit CountDownLatch(int count);

 public:
  // 阻塞等待计数值变为 0
  void wait();

  // 计数减一
  void countDown();

  // 获取当前计数
  int getCount() const;

 private:
  mutable MutexLock mutex_;
  Condition condition_ GUARDED_BY(mutex_);
  int count_ GUARDED_BY(mutex_);
};

}  // namespace muduo
