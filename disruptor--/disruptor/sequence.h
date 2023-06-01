// Copyright (c) 2011-2015, Francois Saint-Jacques
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the disruptor-- nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL FRANCOIS SAINT-JACQUES BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef CACHE_LINE_SIZE_IN_BYTES     // NOLINT
#define CACHE_LINE_SIZE_IN_BYTES 64  // NOLINT
#endif                               // NOLINT
#define ATOMIC_SEQUENCE_PADDING_LENGTH (CACHE_LINE_SIZE_IN_BYTES - sizeof(std::atomic<int64_t>)) / 8
#define SEQUENCE_PADDING_LENGTH (CACHE_LINE_SIZE_IN_BYTES - sizeof(int64_t)) / 8

#pragma once

#include <atomic>
#include <climits>
#include <vector>

#include "disruptor/utils.h"

namespace disruptor {

// special cursor values
constexpr int64_t kInitialCursorValue = -1L;
constexpr int64_t kAlertedSignal = -2L;
constexpr int64_t kTimeoutSignal = -3L;
constexpr int64_t kFirstSequenceValue = kInitialCursorValue + 1L;

/**
 * @brief 跟踪 RingBuffer 中生产者和消费者处理事件的进度
 *
 * @note Sequence 的设计旨在提供一种高效的方式来追踪事件处理的进度, 并且支持并发访问, 确保事件以正确的顺序进行处理
 */
class Sequence {
 public:
  /**
   * @brief 初始化序列号的值, 未提供初始值时使用 kInitialCursorValue 即 -1
   *
   * @param initial_value 序列号初始值
   */
  explicit Sequence(int64_t initial_value = kInitialCursorValue) : sequence_(initial_value) {
  }

 public:
  /**
   * @brief 获取序列号的值
   *
   * @return int64_t 序列号值
   */
  int64_t sequence() const {
    return sequence_.load(std::memory_order::memory_order_acquire);
  }

  /**
   * @brief 设置序列号的值
   *
   * @param value
   */
  void set_sequence(int64_t value) {
    sequence_.store(value, std::memory_order::memory_order_release);
  }

  /**
   * @brief 增加序列号的值并返回增加后的新值
   *
   * @param increment
   * @return int64_t
   */
  int64_t IncrementAndGet(const int64_t& increment) {
    return sequence_.fetch_add(increment, std::memory_order::memory_order_release) + increment;
  }

 private:
  // 内存填充
  int64_t padding0_[ATOMIC_SEQUENCE_PADDING_LENGTH];
  // 序列号
  std::atomic<int64_t> sequence_;
  // 内存填充
  int64_t padding1_[ATOMIC_SEQUENCE_PADDING_LENGTH];

  DISALLOW_COPY_MOVE_AND_ASSIGN(Sequence);
};

/**
 * @brief 辅助函数, 用于获取一组 Sequence 对象中的最小值
 *
 * @param sequences
 * @return int64_t
 */
inline int64_t GetMinimumSequence(const std::vector<Sequence*>& sequences) {
  int64_t minimum = LONG_MAX;

  for (Sequence* sequence_ : sequences) {
    const int64_t sequence = sequence_->sequence();
    minimum = minimum < sequence ? minimum : sequence;
  }

  return minimum;
}

};  // namespace disruptor
