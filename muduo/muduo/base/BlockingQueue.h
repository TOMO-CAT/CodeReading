// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#pragma once

#include <assert.h>

#include <deque>
#include <utility>

#include "muduo/base/Condition.h"
#include "muduo/base/Mutex.h"

namespace muduo {

/**
 * @brief 多线程并发安全阻塞队列
 *
 * @tparam T
 */
template <typename T>
class BlockingQueue : noncopyable {
 public:
  using queue_type = std::deque<T>;

  BlockingQueue() : mutex_(), notEmpty_(mutex_), queue_() {
  }

  /**
   * @brief 入队
   *
   * @param x
   */
  void put(const T& x) {
    MutexLockGuard lock(mutex_);
    queue_.push_back(x);
    notEmpty_.notify();  // wait morphing saves us
    // http://www.domaigne.com/blog/computing/condvars-signal-with-mutex-locked-or-not/
  }

  void put(T&& x) {
    MutexLockGuard lock(mutex_);
    queue_.push_back(std::move(x));
    notEmpty_.notify();
  }

  /**
   * @brief 出队 (获取并返回队列头元素)
   *
   * @note 如果没有数据会阻塞直到数据可用
   * @return T
   */
  T take() {
    MutexLockGuard lock(mutex_);
    // always use a while-loop, due to spurious wakeup
    while (queue_.empty()) {
      notEmpty_.wait();
    }
    assert(!queue_.empty());
    T front(std::move(queue_.front()));
    queue_.pop_front();
    return front;
  }

  /**
   * @brief 一次性获取队列中所有元素
   *
   * @return queue_type
   */
  queue_type drain() {
    std::deque<T> queue;
    {
      MutexLockGuard lock(mutex_);
      queue = std::move(queue_);
      assert(queue_.empty());
    }
    return queue;
  }

  size_t size() const {
    MutexLockGuard lock(mutex_);
    return queue_.size();
  }

 private:
  mutable MutexLock mutex_;
  Condition notEmpty_ GUARDED_BY(mutex_);
  queue_type queue_ GUARDED_BY(mutex_);
};  // __attribute__ ((aligned (64)));

}  // namespace muduo
