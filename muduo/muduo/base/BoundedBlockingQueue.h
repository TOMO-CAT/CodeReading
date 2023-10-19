// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#pragma once

#include <assert.h>

#include <utility>

#include "boost/circular_buffer.hpp"
#include "muduo/base/Condition.h"
#include "muduo/base/Mutex.h"

namespace muduo {

/**
 * @brief 限制最大长度的多线程并发安全阻塞队列
 *
 * @tparam T
 */
template <typename T>
class BoundedBlockingQueue : noncopyable {
 public:
  explicit BoundedBlockingQueue(int maxSize) : mutex_(), notEmpty_(mutex_), notFull_(mutex_), queue_(maxSize) {
  }

  /**
   * @brief 入队 (队列满时阻塞)
   *
   * @param x
   */
  void put(const T& x) {
    MutexLockGuard lock(mutex_);
    while (queue_.full()) {
      notFull_.wait();
    }
    assert(!queue_.full());
    queue_.push_back(x);
    notEmpty_.notify();
  }

  void put(T&& x) {
    MutexLockGuard lock(mutex_);
    while (queue_.full()) {
      notFull_.wait();
    }
    assert(!queue_.full());
    queue_.push_back(std::move(x));
    notEmpty_.notify();
  }

  /**
   * @brief 出队, 队列空时阻塞
   *
   * @return T
   */
  T take() {
    MutexLockGuard lock(mutex_);
    while (queue_.empty()) {
      notEmpty_.wait();
    }
    assert(!queue_.empty());
    T front(std::move(queue_.front()));
    queue_.pop_front();
    notFull_.notify();
    return front;
  }

  bool empty() const {
    MutexLockGuard lock(mutex_);
    return queue_.empty();
  }

  bool full() const {
    MutexLockGuard lock(mutex_);
    return queue_.full();
  }

  size_t size() const {
    MutexLockGuard lock(mutex_);
    return queue_.size();
  }

  size_t capacity() const {
    MutexLockGuard lock(mutex_);
    return queue_.capacity();
  }

 private:
  mutable MutexLock mutex_;
  Condition notEmpty_ GUARDED_BY(mutex_);
  Condition notFull_ GUARDED_BY(mutex_);
  boost::circular_buffer<T> queue_ GUARDED_BY(mutex_);  // 基于唤醒数组实现
};

}  // namespace muduo
