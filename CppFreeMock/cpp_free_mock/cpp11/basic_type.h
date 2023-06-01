#pragma once

namespace cpp_free_mock {

template <int uniq>
struct TypeForUniqMocker {};

template <typename T>
struct MockerBase {};

template <typename T>
struct Mocker : public MockerBase<T> {};

template <typename T>
class MockerEntryPoint {};

template <typename T>
struct SimpleSingleton {
  static T& getInstance() {
    static T value;
    return value;
  }
};

}  // namespace cpp_free_mock
