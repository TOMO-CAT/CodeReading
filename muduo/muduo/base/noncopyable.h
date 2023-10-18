#pragma once

namespace muduo {

/**
 * @brief 通过继承 noncopyable 类可以保证其是不可拷贝的
 *
 */
class noncopyable {
 public:
  noncopyable(const noncopyable&) = delete;
  void operator=(const noncopyable&) = delete;

 protected:
  noncopyable() = default;
  ~noncopyable() = default;
};

}  // namespace muduo
