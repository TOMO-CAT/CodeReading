#include <functional>
#include <iostream>

#include "cpp_free_mock/cpp_free_mock.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

std::string func0() {
  return "Non mocked.";
}

// throw() make compiler don't generate code for catch.
std::string func0NoException() noexcept {
  return "Non mocked.";
}

void func1() {
  func0NoException();
  abort();
}

TEST(TestThrowException, ThrowBasicType) {
  auto mocker = MOCKER(func0);
  EXPECT_CALL(*mocker, MOCK_FUNCTION())
      .Times(::testing::Exactly(2))
      .WillOnce(::testing::Throw(0))
      .WillOnce(::testing::Throw(""));
  EXPECT_THROW(func0(), int);
  try {
    func0();
    abort();
  } catch (const char*) {
  }
  mocker->RestoreToReal();
  EXPECT_EQ("Non mocked.", func0());
}

TEST(TestThrowException, ThrowFromNoExceptionFunction) {
  auto mocker = MOCKER(func0NoException);
  EXPECT_CALL(*mocker, MOCK_FUNCTION()).Times(::testing::Exactly(1)).WillOnce(::testing::Throw(0));
  EXPECT_THROW(func1(), int);
}
