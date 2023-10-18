// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#pragma once

#include <assert.h>
#include <pthread.h>
#include <stdlib.h>  // atexit

#include "muduo/base/noncopyable.h"

namespace muduo {

namespace detail {

// This doesn't detect inherited member functions!
// http://stackoverflow.com/questions/1966362/sfinae-to-check-for-inherited-member-functions
/**
 * @brief C++ 模板元编程, 用于检测一个类是否具有 `no_destroy` 的静态成员
 *
 * @tparam T
 */
template <typename T>
struct has_no_destroy {
  // 模板函数 test 接受一个模板参数 C, 使用 `decltype` 关键字来获取 C 类型的静态成员 `no_destroy` 类型
  // 如果 `no_destroy` 存在, 这个函数返回 `char` 类型, 否则将返回另一个重载的 `test` 函数
  template <typename C>
  static char test(decltype(&C::no_destroy));
  // 备用版本, 它接受任意参数 (`...` 表示可变参数列表), 并返回 int32_t 类型
  template <typename C>
  static int32_t test(...);
  // 使用 `test` 函数来测试模板参数 `T` 是否具有静态成员函数 `no_destroy`
  // 1. 如果有的话返回 `char` 类型, sizeof(char) 就返回 1
  // 2. 如果没有的话返回 `int32_t`, sizeof(int32_t) 结果肯定大于 1
  static const bool value = sizeof(test<T>(0)) == 1;
};

}  // namespace detail

/**
 * @brief 通过继承 Singeton 类可以实现懒汉单例模式
 *
 * @tparam T
 */
template <typename T>
class Singleton : noncopyable {
 public:
  // private 构造函数: 保证对象只能在内部私有化, 外界只能通过 instance 方法获取实例对象
  // private 析构函数: 保证对象只能在内部析构
  Singleton() = delete;
  ~Singleton() = delete;

 public:
  // 获取单例对象
  static T& instance() {
    pthread_once(&ponce_, &Singleton::init);
    assert(value_ != NULL);
    return *value_;
  }

 private:
  static void init() {
    value_ = new T();
    // 如果 T 类型没有静态 `no_destroy` 成员, 那么就会注册 `destroy` 方法在程序退出时调用析构函数
    if (!detail::has_no_destroy<T>::value) {
      ::atexit(destroy);
    }
  }

  static void destroy() {
    // 编译时检查 T 是否完全定义的技巧, 定义了一个新类型 T_must_be_complete_type
    // 1. 它在 T 完全定义时数组大小为 1, 是合法的
    // 2. 它在 T 未完全定义时数组大小未 -1, 会报编译错误
    typedef char T_must_be_complete_type[sizeof(T) == 0 ? -1 : 1];
    T_must_be_complete_type dummy;
    (void)dummy;

    // 调用析构函数
    delete value_;
    value_ = NULL;
  }

 private:
  static pthread_once_t ponce_;  // 用于标识函数是否已经被执行过, 用于多线程环境下确保函数只执行一次
  static T* value_;              // 单例对象
};

template <typename T>
pthread_once_t Singleton<T>::ponce_ = PTHREAD_ONCE_INIT;

template <typename T>
T* Singleton<T>::value_ = NULL;

}  // namespace muduo
