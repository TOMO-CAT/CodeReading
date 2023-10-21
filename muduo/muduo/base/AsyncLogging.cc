// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "muduo/base/AsyncLogging.h"

#include <stdio.h>

#include <utility>

#include "muduo/base/LogFile.h"
#include "muduo/base/Timestamp.h"

using muduo::AsyncLogging;

AsyncLogging::AsyncLogging(const string& basename, off_t rollSize, int flushInterval)
    : flushInterval_(flushInterval),
      running_(false),
      basename_(basename),
      rollSize_(rollSize),
      thread_(std::bind(&AsyncLogging::threadFunc, this), "Logging"),
      latch_(1),
      mutex_(),
      cond_(mutex_),
      currentBuffer_(new Buffer),  // 4M
      nextBuffer_(new Buffer),     // 4M
      buffers_() {
  currentBuffer_->bzero();
  nextBuffer_->bzero();
  buffers_.reserve(16);
}

/**
 * @brief 前端线程提交日志消息
 *
 * @param logline
 * @param len
 */
void AsyncLogging::append(const char* logline, int len) {
  muduo::MutexLockGuard lock(mutex_);

  // 最常见的情况: currentBuffer 还未满, 此时直接拷贝数据到 currentBuffer 即可
  if (currentBuffer_->avail() > len) {
    currentBuffer_->append(logline, len);
  } else {
    // 如果 currentBuffer 已满那么将其加入到 buffers_
    buffers_.push_back(std::move(currentBuffer_));
    if (nextBuffer_) {
      // 将预备好的另一块缓冲 nextBuffer_ 设置为 currentBuffer_
      currentBuffer_ = std::move(nextBuffer_);
    } else {
      // 极少发生: 前端写入速度太快一下子将两块缓冲都用完了, 只好重新分配一块新 buffer
      currentBuffer_.reset(new Buffer);  // Rarely happens
    }
    currentBuffer_->append(logline, len);
    cond_.notify();
  }
}

/**
 * @brief 后端线程主要工作内容
 *
 */
void AsyncLogging::threadFunc() {
  assert(running_ == true);
  latch_.countDown();

  // 由于只有当前线程会写入到日志文件, 因此 threadSafe 标志设置为 false
  LogFile output(basename_, rollSize_, false);

  // 准备好两块空闲的 buffer
  BufferPtr newBuffer1(new Buffer);
  BufferPtr newBuffer2(new Buffer);
  newBuffer1->bzero();
  newBuffer2->bzero();

  BufferVector buffersToWrite;
  buffersToWrite.reserve(16);
  while (running_) {
    assert(newBuffer1 && newBuffer1->length() == 0);
    assert(newBuffer2 && newBuffer2->length() == 0);
    assert(buffersToWrite.empty());

    {
      // 在临界区内等待条件触发, 这里的条件包括两种:
      // 1. 超时
      // 2. 前端写满了一个或多个 buffer
      muduo::MutexLockGuard lock(mutex_);
      // unusual usage!
      if (buffers_.empty()) {
        cond_.waitForSeconds(flushInterval_);
      }

      // 条件满足时先将当前缓冲 currentBuffer_ 塞入 buffers, 并立即用 newBuffer1 移动 (move) 为当前缓冲
      buffers_.push_back(std::move(currentBuffer_));
      currentBuffer_ = std::move(newBuffer1);
      // 交换 buffers_ 和 buffersToWrite, 后续准备将 buffersToWrite 写入到日志
      buffersToWrite.swap(buffers_);
      // 用 newBuffer2 替换 nextBuffer_, 这样可以保证前端始终有一个预备 buffer 可供调配
      // 减少前端临界区分配内存的概率, 缩短前端临界区长度
      if (!nextBuffer_) {
        nextBuffer_ = std::move(newBuffer2);
      }
    }

    assert(!buffersToWrite.empty());

    // 如果待写入的 buffer 数量超过 25 个, 即 100 M
    // 记录错误日志并只保留 buffersToWrite 中两个 buffer 准备写入文件
    if (buffersToWrite.size() > 25) {
      char buf[256];
      snprintf(buf, sizeof buf, "Dropped log messages at %s, %zd larger buffers\n",
               Timestamp::now().toFormattedString().c_str(), buffersToWrite.size() - 2);
      fputs(buf, stderr);
      output.append(buf, static_cast<int>(strlen(buf)));
      buffersToWrite.erase(buffersToWrite.begin() + 2, buffersToWrite.end());
    }

    // 将 buffersToWrite 中的所有 buffer 都写入到文件中
    for (const auto& buffer : buffersToWrite) {
      output.append(buffer->data(), buffer->length());
    }

    // 重新构造空闲的 newBuffer1 和 newBuffer2, 下次执行时可以有两个空闲的 buffer 用于替换前端的当前缓冲和预备缓冲
    if (buffersToWrite.size() > 2) {
      // drop non-bzero-ed buffers, avoid trashing
      buffersToWrite.resize(2);
    }

    if (!newBuffer1) {
      assert(!buffersToWrite.empty());
      newBuffer1 = std::move(buffersToWrite.back());
      buffersToWrite.pop_back();
      newBuffer1->reset();
    }

    if (!newBuffer2) {
      assert(!buffersToWrite.empty());
      newBuffer2 = std::move(buffersToWrite.back());
      buffersToWrite.pop_back();
      newBuffer2->reset();
    }

    buffersToWrite.clear();
    output.flush();
  }
  output.flush();
}
