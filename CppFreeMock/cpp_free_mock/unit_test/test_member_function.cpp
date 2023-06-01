// CppFreeMock: a tool for mock global function, member function, class static function.
//
// Test member function:
//  1, multi parameter;
//  2, reference parameter;
//  3, function pointer;
//  4, check this pointer;
//  5, rollback to no mock;

#include <functional>
#include <iostream>

#include "cpp_free_mock/cpp_free_mock.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::testing::_;

struct TestMockStruct {
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
};

struct TestStubStruct {
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

  static int testReferenceStubP1P2(bool& boolValue, char& charValue) {
    boolValue = !boolValue;
    charValue += 1;
    return 12;
  }

  static int testReferenceStubP3P4P5(int& intValue, std::string& stringValue, const std::string& stringConstValue) {
    --intValue;
    stringValue.resize(0);
    return 345;
  }
};

struct TestMemberFunction : public ::testing::Test {
  TestMemberFunction() : boolValue(false), charValue('0'), intValue(0), stringValue(""), stringConstValue("HELLO") {
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

TEST_F(TestMemberFunction, RestoreToReal) {
  TestMockStruct mockStruct;
  auto mocker = MOCKER(&TestMockStruct::func0Parameter);
  EXPECT_CALL(*mocker, MOCK_FUNCTION(&mockStruct))
      .Times(::testing::Exactly(1))
      .WillOnce(::testing::Return("Hello world."));
  EXPECT_EQ("Hello world.", mockStruct.func0Parameter());
  mocker->RestoreToReal();
  EXPECT_EQ("Non mocked.", mockStruct.func0Parameter());
}

TEST_F(TestMemberFunction, MultiParameter) {
  TestMockStruct mockStruct;
  auto call = bind(&TestMockStruct::func5Parameter, &mockStruct, false, '0', 0, "LOUIX", "");
  auto mocker = MOCKER(&TestMockStruct::func5Parameter);
  EXPECT_CALL(*mocker, MOCK_FUNCTION(&mockStruct, _, _, _, _, _))
      .Times(::testing::Exactly(1))
      .WillOnce(::testing::Return("Hello world."));
  EXPECT_EQ("Hello world.", call());
  EXPECT_CALL(*mocker, MOCK_FUNCTION(_, false, _, 0, _, _))
      .Times(::testing::Exactly(1))
      .WillOnce(::testing::Return("With false and 0."));
  EXPECT_EQ("With false and 0.", call());
  mocker->RestoreToReal();
  EXPECT_EQ("Non mocked.", call());
}

TEST_F(TestMemberFunction, MultiReferenceParameter) {
  TestMockStruct mockStruct;
  auto call = bind(&TestMockStruct::func5ReferenceParameter, &mockStruct, std::ref(boolValue), std::ref(charValue),
                   std::ref(intValue), std::ref(stringValue), std::ref(stringConstValue));
  auto mocker = MOCKER(&TestMockStruct::func5ReferenceParameter);
  EXPECT_CALL(*mocker, MOCK_FUNCTION(_, _, _, _, _, _)).Times(::testing::Exactly(1)).WillOnce(::testing::Return(1));
  EXPECT_EQ(1, call());
  boolValue = true;
  intValue = 1;
  EXPECT_CALL(*mocker, MOCK_FUNCTION(_, ::testing::Ref(boolValue), _, ::testing::Ref(intValue), _, _))
      .Times(::testing::Exactly(2))
      .WillRepeatedly(::testing::Return(2));
  EXPECT_EQ(2, mockStruct.func5ReferenceParameter(boolValue, charValue, intValue, stringValue, ""));
  EXPECT_EQ(2, call());
  mocker->RestoreToReal();
  EXPECT_EQ(0, call());
}

TEST_F(TestMemberFunction, MultiReferenceParameterWithArgsAction) {
  TestMockStruct mockStruct;
  auto call = bind(&TestMockStruct::func5ReferenceParameter, &mockStruct, std::ref(boolValue), std::ref(charValue),
                   std::ref(intValue), std::ref(stringValue), std::ref(stringConstValue));
  auto mocker = MOCKER(&TestMockStruct::func5ReferenceParameter);
  EXPECT_CALL(*mocker, MOCK_FUNCTION(&mockStruct, ::testing::Ref(boolValue), ::testing::Ref(charValue),
                                     ::testing::Ref(intValue), ::testing::Ref(stringValue), _))
      .Times(::testing::AtLeast(1))
      .WillOnce(::testing::WithArgs<1, 2>(::testing::Invoke(TestStubStruct::testReferenceStubP1P2)));
  EXPECT_EQ(12, call());
  EXPECT_TRUE(boolValue);
  EXPECT_EQ('1', charValue);
  EXPECT_CALL(*mocker, MOCK_FUNCTION(_, ::testing::Ref(boolValue), ::testing::Ref(charValue), ::testing::Ref(intValue),
                                     ::testing::Ref(stringValue), _))
      .Times(::testing::AtLeast(1))
      .WillOnce(::testing::WithArgs<3, 4, 5>(::testing::Invoke(TestStubStruct::testReferenceStubP3P4P5)));
  intValue = 9047;
  EXPECT_EQ(345, call());
  EXPECT_EQ(9046, intValue);
  EXPECT_STREQ("", stringValue.c_str());
}

TEST_F(TestMemberFunction, FunctionPointer) {
  auto function = &TestMockStruct::func0Parameter;
  auto mocker1 = MOCKER(function);
  EXPECT_CALL(*mocker1, MOCK_FUNCTION(_)).Times(::testing::Exactly(2)).WillRepeatedly(::testing::Return("Func1"));
  TestMockStruct mockStruct;
  EXPECT_EQ("Func1", mockStruct.func0Parameter());
  EXPECT_EQ("Func1", (mockStruct.*function)());
  function = &TestMockStruct::func0Parameter2;
  auto mocker2 = MOCKER(function);
  EXPECT_CALL(*mocker2, MOCK_FUNCTION(&mockStruct))
      .Times(::testing::Exactly(2))
      .WillRepeatedly(::testing::Return("Func2"));
  EXPECT_EQ("Func2", mockStruct.TestMockStruct::func0Parameter2());
  EXPECT_EQ("Func2", (mockStruct.*function)());
  function = &TestMockStruct::func0Parameter3;
  auto mocker3 = MOCKER(function);
  EXPECT_CALL(*mocker3, MOCK_FUNCTION(_)).Times(::testing::Exactly(2)).WillRepeatedly(::testing::Return("Func3"));
  EXPECT_EQ("Func3", mockStruct.func0Parameter3());
  EXPECT_EQ("Func3", (mockStruct.*function)());
}

TEST_F(TestMemberFunction, CheckThisPoint) {
  auto function = &TestMockStruct::func0Parameter;
  TestMockStruct mockStruct1;
  TestMockStruct mockStruct2;
  TestMockStruct mockStruct3;
  auto mocker1 = MOCKER(function);
  EXPECT_CALL(*mocker1, MOCK_FUNCTION(_)).Times(::testing::Exactly(1)).WillRepeatedly(::testing::Return("Object_"));
  EXPECT_CALL(*mocker1, MOCK_FUNCTION(&mockStruct1))
      .Times(::testing::Exactly(1))
      .WillRepeatedly(::testing::Return("Object1"));
  EXPECT_CALL(*mocker1, MOCK_FUNCTION(&mockStruct2))
      .Times(::testing::Exactly(1))
      .WillRepeatedly(::testing::Return("Object2"));
  auto call = bind(&TestMockStruct::func0Parameter, &mockStruct3);
  EXPECT_EQ("Object1", mockStruct1.func0Parameter());
  EXPECT_EQ("Object2", (mockStruct2.*function)());
  EXPECT_EQ("Object_", call());
}
