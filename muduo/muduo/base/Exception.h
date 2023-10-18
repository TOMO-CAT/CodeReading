// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#pragma once

#include <exception>

#include "muduo/base/Types.h"

namespace muduo {

class Exception : public std::exception {
 public:
  explicit Exception(string what);
  ~Exception() noexcept override = default;

 public:
  // default copy-ctor and operator= are okay.

  const char* what() const noexcept override { return message_.c_str(); }

  const char* stackTrace() const noexcept { return stack_.c_str(); }

 private:
  string message_;
  string stack_;
};

}  // namespace muduo
