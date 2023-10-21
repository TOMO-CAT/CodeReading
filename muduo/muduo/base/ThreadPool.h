// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#pragma once

#include <deque>
#include <memory>
#include <string>
#include <vector>

#include "muduo/base/Condition.h"
#include "muduo/base/Mutex.h"
#include "muduo/base/Thread.h"
#include "muduo/base/Types.h"

namespace muduo {

/**
 * @brief 线程池
 *
 */
class ThreadPool : noncopyable {
 public:
  typedef std::function<void()> Task;

  explicit ThreadPool(const string& nameArg = string("ThreadPool"));
  ~ThreadPool();

 public:
  /**
   * @brief 设置任务队列最大长度
   *
   * @param maxSize
   */
  // Must be called before start().
  void setMaxQueueSize(int maxSize) {
    maxQueueSize_ = maxSize;
  }
  void setThreadInitCallback(const Task& cb) {
    threadInitCallback_ = cb;
  }

  /**
   * @brief 启动线程池
   *
   * @param numThreads
   */
  void start(int numThreads);
  void stop();

  const string& name() const {
    return name_;
  }

  size_t queueSize() const;

  // Could block if maxQueueSize > 0
  // Call after stop() will return immediately.
  // There is no move-only version of std::function in C++ as of C++14.
  // So we don't need to overload a const& and an && versions
  // as we do in (Bounded)BlockingQueue.
  // https://stackoverflow.com/a/25408989
  void run(Task f);

 private:
  /**
   * @brief 判断任务队列是否满了
   *
   */
  bool isFull() const REQUIRES(mutex_);
  /**
   * @brief 每个线程执行的死循环函数
   *
   */
  void runInThread();
  /**
   * @brief 从任务队列中取出一个任务
   *
   * @return Task
   */
  Task take();

 private:
  mutable MutexLock mutex_;
  Condition notEmpty_ GUARDED_BY(mutex_);
  Condition notFull_ GUARDED_BY(mutex_);
  string name_;
  Task threadInitCallback_;                              // 工作线程线程初始化函数
  std::vector<std::unique_ptr<muduo::Thread>> threads_;  // 所有的工作线程
  std::deque<Task> queue_ GUARDED_BY(mutex_);            // 任务队列
  size_t maxQueueSize_;                                  // 最大任务数量
  bool running_;
};

}  // namespace muduo
