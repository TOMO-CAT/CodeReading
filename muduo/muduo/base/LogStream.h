// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#pragma once

#include <assert.h>
#include <string.h>  // memcpy

#include <string>

#include "muduo/base/StringPiece.h"
#include "muduo/base/Types.h"
#include "muduo/base/noncopyable.h"

namespace muduo {

namespace detail {

const int kSmallBuffer = 4000;
const int kLargeBuffer = 4000 * 1000;

/**
 * @brief 固定大小的字符缓冲区, 大小由模板参数 SIZE 指定
 *
 * @tparam SIZE
 */
template <int SIZE>
class FixedBuffer : noncopyable {
 public:
  FixedBuffer() : cur_(data_) {
    setCookie(cookieStart);
  }

  ~FixedBuffer() {
    setCookie(cookieEnd);
  }

 public:
  /**
   * @brief 将 buf 写入到缓冲区 cur 指向的位置, 并且调整 cur_ 位置指向最新的 buf header
   *
   * @note cookie 可以保证每条内存消息中的日志信息都带有 cookie, 其值为某个函数的地址, 这样可以在 coredump
   * 文件中通过查找 cookie 值找到所有尚未来得及写入磁盘的消息
   */
  void append(const char* /*restrict*/ buf, size_t len) {
    // FIXME: append partially
    if (implicit_cast<size_t>(avail()) > len) {
      ::memcpy(cur_, buf, len);
      cur_ += len;
    }
  }

  const char* data() const {
    return data_;
  }
  int length() const {
    return static_cast<int>(cur_ - data_);
  }

  // write to data_ directly
  char* current() {
    return cur_;
  }

  /**
   * @brief 剩余的缓冲区大小
   *
   * @return int
   */
  int avail() const {
    return static_cast<int>(end() - cur_);
  }

  /**
   * @brief 调整 cur_ 指针位置 (这是为了做什么?)
   *
   * @param len
   */
  void add(size_t len) {
    cur_ += len;
  }

  /**
   * @brief 清空缓冲区
   *
   */
  void reset() {
    cur_ = data_;
  }

  /**
   * @brief 将缓冲区数据置零
   *
   */
  void bzero() {
    memZero(data_, sizeof data_);
  }

  // for used by GDB
  const char* debugString();

  void setCookie(void (*cookie)()) {
    cookie_ = cookie;
  }

  // for used by unit test
  string toString() const {
    return string(data_, length());
  }

  StringPiece toStringPiece() const {
    return StringPiece(data_, length());
  }

 private:
  const char* end() const {
    return data_ + sizeof data_;
  }
  // Must be outline function for cookies.
  static void cookieStart();
  static void cookieEnd();

  void (*cookie_)();

 private:
  char data_[SIZE];
  char* cur_;
};

}  // namespace detail

/**
 * @brief 替代 iostream 以实现高性能
 *
 * @tparam SIZE
 */
class LogStream : noncopyable {
  typedef LogStream self;

 public:
  typedef detail::FixedBuffer<detail::kSmallBuffer> Buffer;

  self& operator<<(bool v) {
    buffer_.append(v ? "1" : "0", 1);
    return *this;
  }

  self& operator<<(short);
  self& operator<<(unsigned short);
  self& operator<<(int);
  self& operator<<(unsigned int);
  self& operator<<(long);
  self& operator<<(unsigned long);
  self& operator<<(long long);
  self& operator<<(unsigned long long);

  self& operator<<(const void*);

  self& operator<<(float v) {
    *this << static_cast<double>(v);
    return *this;
  }
  self& operator<<(double);
  // self& operator<<(long double);

  self& operator<<(char v) {
    buffer_.append(&v, 1);
    return *this;
  }

  // self& operator<<(signed char);
  // self& operator<<(unsigned char);

  self& operator<<(const char* str) {
    if (str) {
      buffer_.append(str, strlen(str));
    } else {
      buffer_.append("(null)", 6);
    }
    return *this;
  }

  self& operator<<(const unsigned char* str) {
    return operator<<(reinterpret_cast<const char*>(str));
  }

  self& operator<<(const string& v) {
    buffer_.append(v.c_str(), v.size());
    return *this;
  }

  self& operator<<(const StringPiece& v) {
    buffer_.append(v.data(), v.size());
    return *this;
  }

  self& operator<<(const Buffer& v) {
    *this << v.toStringPiece();
    return *this;
  }

  void append(const char* data, int len) {
    buffer_.append(data, len);
  }
  const Buffer& buffer() const {
    return buffer_;
  }
  void resetBuffer() {
    buffer_.reset();
  }

 private:
  void staticCheck();

  template <typename T>
  void formatInteger(T);

  Buffer buffer_;

  static const int kMaxNumericSize = 48;
};

class Fmt {
 public:
  template <typename T>
  Fmt(const char* fmt, T val);

  const char* data() const {
    return buf_;
  }
  int length() const {
    return length_;
  }

 private:
  char buf_[32];
  int length_;
};

inline LogStream& operator<<(LogStream& s, const Fmt& fmt) {
  s.append(fmt.data(), fmt.length());
  return s;
}

// Format quantity n in SI units (k, M, G, T, P, E).
// The returned string is at most 5 characters long.
// Requires n >= 0
string formatSI(int64_t n);

// Format quantity n in IEC (binary) units (Ki, Mi, Gi, Ti, Pi, Ei).
// The returned string is at most 6 characters long.
// Requires n >= 0
string formatIEC(int64_t n);

}  // namespace muduo
