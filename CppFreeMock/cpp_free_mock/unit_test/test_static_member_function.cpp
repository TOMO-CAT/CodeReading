// CppFreeMock: a tool for mock global function, member function, class static function.
//
// Test static member function:
//  1, multi parameter;
//  2, reference parameter;
//  3, function pointer;
//  4, rollback to no mock;

#include <functional>
#include <iostream>

#include "cpp_free_mock/cpp_free_mock.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::testing::_;

int testReferenceStubP3P4P5(int& intValue, std::string& stringValue, const std::string& stringConstValue) {
  --intValue;
  stringValue.resize(0);
  return 345;
}

struct TestStaticMemberFunction : public ::testing::Test {
  TestStaticMemberFunction()
      : boolValue(false), charValue('0'), intValue(0), stringValue(""), stringConstValue("HELLO") {
  }
  virtual void SetUp() {
  }
  virtual void TearDown() {
    CLEAR_MOCKER();
  }

  static std::string func0Parameter() {
    return "Non mocked.";
  }

  static std::string func0Parameter2() {
    return "Non mocked.";
  }

  static std::string func0Parameter3() {
    return "Non mocked.";
  }

  static std::string func5Parameter(bool, char, int, std::string, const std::string&) {
    return "Non mocked.";
  }

  static int func5ReferenceParameter(bool&, char&, int&, std::string&, const std::string&) {
    return 0;
  }

  static int testReferenceStubP1P2(bool& boolValue, char& charValue) {
    boolValue = !boolValue;
    charValue += 1;
    return 12;
  }

  bool boolValue;
  char charValue;
  int intValue;
  std::string stringValue;
  const std::string stringConstValue;
};

TEST_F(TestStaticMemberFunction, RestoreToReal) {
  auto mocker = MOCKER(TestStaticMemberFunction::func0Parameter);
  EXPECT_CALL(*mocker, MOCK_FUNCTION()).Times(::testing::Exactly(1)).WillOnce(::testing::Return("Hello world."));
  EXPECT_EQ("Hello world.", TestStaticMemberFunction::func0Parameter());
  mocker->RestoreToReal();
  EXPECT_EQ("Non mocked.", TestStaticMemberFunction::func0Parameter());
}

TEST_F(TestStaticMemberFunction, MultiParameter) {
  auto call = bind(TestStaticMemberFunction::func5Parameter, false, '0', 0, "LOUIX", "");
  auto mocker = MOCKER(TestStaticMemberFunction::func5Parameter);
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

TEST_F(TestStaticMemberFunction, MultiReferenceParameter) {
  auto call = bind(TestStaticMemberFunction::func5ReferenceParameter, std::ref(boolValue), std::ref(charValue),
                   std::ref(intValue), std::ref(stringValue), std::ref(stringConstValue));
  auto mocker = MOCKER(TestStaticMemberFunction::func5ReferenceParameter);
  EXPECT_CALL(*mocker, MOCK_FUNCTION(_, _, _, _, _)).Times(::testing::Exactly(1)).WillOnce(::testing::Return(1));
  EXPECT_EQ(1, call());
  boolValue = true;
  intValue = 1;
  EXPECT_CALL(*mocker, MOCK_FUNCTION(::testing::Ref(boolValue), _, ::testing::Ref(intValue), _, _))
      .Times(::testing::Exactly(2))
      .WillRepeatedly(::testing::Return(2));
  EXPECT_EQ(2, TestStaticMemberFunction::func5ReferenceParameter(boolValue, charValue, intValue, stringValue, ""));
  EXPECT_EQ(2, call());
  mocker->RestoreToReal();
  EXPECT_EQ(0, call());
}

TEST_F(TestStaticMemberFunction, MultiReferenceParameterWithArgsAction) {
  auto call = bind(TestStaticMemberFunction::func5ReferenceParameter, std::ref(boolValue), std::ref(charValue),
                   std::ref(intValue), std::ref(stringValue), std::ref(stringConstValue));
  auto mocker = MOCKER(TestStaticMemberFunction::func5ReferenceParameter);
  EXPECT_CALL(*mocker, MOCK_FUNCTION(::testing::Ref(boolValue), ::testing::Ref(charValue), ::testing::Ref(intValue),
                                     ::testing::Ref(stringValue), _))
      .Times(::testing::AtLeast(1))
      .WillOnce(::testing::WithArgs<0, 1>(::testing::Invoke(TestStaticMemberFunction::testReferenceStubP1P2)));
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

TEST_F(TestStaticMemberFunction, FunctionPointer) {
  auto function = TestStaticMemberFunction::func0Parameter;
  auto mocker1 = MOCKER(TestStaticMemberFunction::func0Parameter);
  EXPECT_CALL(*mocker1, MOCK_FUNCTION()).Times(::testing::Exactly(1)).WillOnce(::testing::Return("Func1"));
  EXPECT_EQ("Func1", TestStaticMemberFunction::func0Parameter());
  function = TestStaticMemberFunction::func0Parameter2;
  auto mocker2 = MOCKER(function);
  EXPECT_CALL(*mocker2, MOCK_FUNCTION()).Times(::testing::Exactly(1)).WillOnce(::testing::Return("Func2"));
  EXPECT_EQ("Func2", TestStaticMemberFunction::func0Parameter2());
  function = TestStaticMemberFunction::func0Parameter3;
  auto mocker3 = MOCKER(function);
  EXPECT_CALL(*mocker3, MOCK_FUNCTION()).Times(::testing::Exactly(1)).WillOnce(::testing::Return("Func3"));
  EXPECT_EQ("Func3", TestStaticMemberFunction::func0Parameter3());
}
