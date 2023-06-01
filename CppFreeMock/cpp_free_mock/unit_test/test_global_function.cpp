// CppFreeMock: a tool for mock global function, member function, class static function.
//
// Test global function:
//  1, multi parameter;
//  2, reference parameter;
//  3, different namespace & same function name;
//  4, function pointer;
//  5, rollback to no mock;

#include <functional>
#include <iostream>

#include "cpp_free_mock/cpp_free_mock.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::testing::_;

std::string func0Parameter() {
  return "Non mocked.";
}

std::string func0Parameter2() {
  return "Non mocked.";
}

std::string func0Parameter3() {
  return "Non mocked.";
}

std::string func5Parameter(bool, char, int, std::string, const std::string&) {
  return "Non mocked.";
}

int func5ReferenceParameter(bool&, char&, int&, std::string&, const std::string&) {
  return 0;
}

int testReferenceStubP1P2(bool& boolValue, char& charValue) {
  boolValue = !boolValue;
  charValue += 1;
  return 12;
}

int testReferenceStubP3P4P5(int& intValue, std::string& stringValue, const std::string& stringConstValue) {
  --intValue;
  stringValue.resize(0);
  return 345;
}

namespace TestNamespace {
std::string func0Parameter() {
  return "Non mocked.";
}
}  // namespace TestNamespace

struct TestGlobalFunction : public ::testing::Test {
  TestGlobalFunction() : boolValue(false), charValue('0'), intValue(0), stringValue(""), stringConstValue("HELLO") {
  }
  virtual void SetUp() {
  }
  virtual void TearDown() {
    CLEAR_MOCKER();
  }

  bool boolValue;
  char charValue;
  int intValue;
  std::string stringValue;
  const std::string stringConstValue;
};

TEST_F(TestGlobalFunction, RestoreToReal) {
  auto mocker = MOCKER(func0Parameter);
  EXPECT_CALL(*mocker, MOCK_FUNCTION()).Times(::testing::Exactly(1)).WillOnce(::testing::Return("Hello world."));
  EXPECT_EQ("Hello world.", func0Parameter());
  mocker->RestoreToReal();
  EXPECT_EQ("Non mocked.", func0Parameter());
}

TEST_F(TestGlobalFunction, MultiParameter) {
  auto call = bind(func5Parameter, false, '0', 0, "LOUIX", "");
  auto mocker = MOCKER(func5Parameter);
  EXPECT_CALL(*mocker, MOCK_FUNCTION(_, _, _, _, _))
      .Times(::testing::Exactly(1))
      .WillOnce(::testing::Return("Hello world."));
  EXPECT_EQ("Hello world.", call());
  EXPECT_CALL(*mocker, MOCK_FUNCTION(false, _, 0, _, _))
      .Times(::testing::Exactly(1))
      .WillOnce(::testing::Return("With false and 0."));
  EXPECT_EQ("With false and 0.", call());
  mocker->RestoreToReal();
  EXPECT_EQ("Non mocked.", call());
}

TEST_F(TestGlobalFunction, MultiReferenceParameter) {
  auto call = bind(func5ReferenceParameter, std::ref(boolValue), std::ref(charValue), std::ref(intValue),
                   std::ref(stringValue), std::ref(stringConstValue));
  auto mocker = MOCKER(func5ReferenceParameter);
  EXPECT_CALL(*mocker, MOCK_FUNCTION(_, _, _, _, _)).Times(::testing::Exactly(1)).WillOnce(::testing::Return(1));
  EXPECT_EQ(1, call());
  boolValue = true;
  intValue = 1;
  EXPECT_CALL(*mocker, MOCK_FUNCTION(::testing::Ref(boolValue), _, ::testing::Ref(intValue), _, _))
      .Times(::testing::Exactly(2))
      .WillRepeatedly(::testing::Return(2));
  EXPECT_EQ(2, func5ReferenceParameter(boolValue, charValue, intValue, stringValue, ""));
  EXPECT_EQ(2, call());
  mocker->RestoreToReal();
  EXPECT_EQ(0, call());
}

TEST_F(TestGlobalFunction, MultiReferenceParameterWithArgsAction) {
  auto call = bind(func5ReferenceParameter, std::ref(boolValue), std::ref(charValue), std::ref(intValue),
                   std::ref(stringValue), std::ref(stringConstValue));
  auto mocker = MOCKER(func5ReferenceParameter);
  EXPECT_CALL(*mocker, MOCK_FUNCTION(::testing::Ref(boolValue), ::testing::Ref(charValue), ::testing::Ref(intValue),
                                     ::testing::Ref(stringValue), _))
      .Times(::testing::AtLeast(1))
      .WillOnce(::testing::WithArgs<0, 1>(::testing::Invoke(testReferenceStubP1P2)));
  EXPECT_EQ(12, call());
  EXPECT_TRUE(boolValue);
  EXPECT_EQ('1', charValue);
  EXPECT_CALL(*mocker, MOCK_FUNCTION(::testing::Ref(boolValue), ::testing::Ref(charValue), ::testing::Ref(intValue),
                                     ::testing::Ref(stringValue), _))
      .Times(::testing::AtLeast(1))
      .WillOnce(::testing::WithArgs<2, 3, 4>(::testing::Invoke(testReferenceStubP3P4P5)));
  intValue = 9047;
  EXPECT_EQ(345, call());
  EXPECT_EQ(9046, intValue);
  EXPECT_STREQ("", stringValue.c_str());
}

TEST_F(TestGlobalFunction, FunctionPointer) {
  auto function = func0Parameter;
  auto mocker1 = MOCKER(func0Parameter);
  EXPECT_CALL(*mocker1, MOCK_FUNCTION()).Times(::testing::Exactly(1)).WillOnce(::testing::Return("Func1"));
  EXPECT_EQ("Func1", func0Parameter());
  function = func0Parameter2;
  auto mocker2 = MOCKER(function);
  EXPECT_CALL(*mocker2, MOCK_FUNCTION()).Times(::testing::Exactly(1)).WillOnce(::testing::Return("Func2"));
  EXPECT_EQ("Func2", func0Parameter2());
  function = func0Parameter3;
  auto mocker3 = MOCKER(function);
  EXPECT_CALL(*mocker3, MOCK_FUNCTION()).Times(::testing::Exactly(1)).WillOnce(::testing::Return("Func3"));
  EXPECT_EQ("Func3", func0Parameter3());
}

TEST_F(TestGlobalFunction, TestNamespace) {
  auto mocker1 = MOCKER(func0Parameter);
  auto mocker2 = MOCKER(TestNamespace::func0Parameter);
  EXPECT_CALL(*mocker1, MOCK_FUNCTION()).Times(::testing::Exactly(1)).WillOnce(::testing::Return("Func1"));
  EXPECT_CALL(*mocker2, MOCK_FUNCTION()).Times(::testing::Exactly(1)).WillOnce(::testing::Return("Func2"));
  EXPECT_EQ("Func1", func0Parameter());
  EXPECT_EQ("Func2", TestNamespace::func0Parameter());
}
