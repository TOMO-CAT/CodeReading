#include <functional>
#include <iostream>

#include "cpp_free_mock/cpp_free_mock.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

std::string func0Parameter() {
  return "Non mocked.";
}

std::string func0Parameter2() {
  return "Non mocked.";
}

struct TestMockStruct {
  std::string func0Parameter() {
    return "Non mocked.";
  }
};

using ::testing::_;

struct TestAnonymousMocker : public ::testing::Test {
  virtual void TearDown() {
    CLEAR_MOCKER();
  }
};

TEST_F(TestAnonymousMocker, TestSimpleSingleton) {
  int a = 0;
  ::cpp_free_mock::SimpleSingleton<int*>::getInstance() = &a;
  EXPECT_EQ(&a, ::cpp_free_mock::SimpleSingleton<int*>::getInstance());
  *::cpp_free_mock::SimpleSingleton<int*>::getInstance() = 2;
  EXPECT_EQ(2, a);
}

TEST_F(TestAnonymousMocker, AnonymousMocker) {
  EXPECT_CALL(*MOCKER(func0Parameter), MOCK_FUNCTION())
      .Times(::testing::Exactly(1))
      .WillOnce(::testing::Return("Hello world."));
  EXPECT_EQ("Hello world.", func0Parameter());
}

TEST_F(TestAnonymousMocker, AnonymousMemberMocker) {
  EXPECT_EQ("Non mocked.", TestMockStruct().func0Parameter());
  EXPECT_CALL(*MOCKER(&TestMockStruct::func0Parameter), MOCK_FUNCTION(_))
      .Times(::testing::Exactly(1))
      .WillOnce(::testing::Return("Hello world."));
  EXPECT_EQ("Hello world.", TestMockStruct().func0Parameter());
}

TEST_F(TestAnonymousMocker, NormalMocker) {
  EXPECT_EQ("Non mocked.", func0Parameter());
  auto mocker = MOCKER(func0Parameter);
  EXPECT_CALL(*mocker, MOCK_FUNCTION()).Times(::testing::Exactly(1)).WillOnce(::testing::Return("Hello world."));
  EXPECT_EQ("Hello world.", func0Parameter());
}

TEST_F(TestAnonymousMocker, SameMocker) {
  EXPECT_EQ(MOCKER(func0Parameter), MOCKER(func0Parameter));
  EXPECT_EQ(MOCKER(&TestMockStruct::func0Parameter), MOCKER(&TestMockStruct::func0Parameter));
}
