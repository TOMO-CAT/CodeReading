#include "cpp_free_mock/runtime_patch.h"

#include <iostream>
#include <vector>

#include "cpp_free_mock/cpp_free_mock.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::testing::_;

struct TestRuntimePatcher : public ::testing::Test {
  virtual void TearDown() {
    CLEAR_MOCKER();
  }
};

TEST_F(TestRuntimePatcher, GraftFunctionFailed) {
  auto abortMocker = MOCKER(std::abort);
  EXPECT_CALL(*abortMocker, MOCK_FUNCTION()).Times(::testing::Exactly(1));
  auto unprotectMemoryForOnePageMocker = MOCKER(cpp_free_mock::RuntimePatcherImpl::UnprotectMemoryForOnePage);
  EXPECT_CALL(*unprotectMemoryForOnePageMocker, MOCK_FUNCTION(_))
      .Times(::testing::Exactly(1))
      .WillOnce(::testing::Return(6));
  std::vector<char> binary_backup;
  char data[16];
  char data2[16];
  cpp_free_mock::RuntimePatcher::GraftFunction(data, data2, binary_backup);
}

TEST_F(TestRuntimePatcher, GraftFunctionSuccess) {
  auto abortMocker = MOCKER(std::abort);
  EXPECT_CALL(*abortMocker, MOCK_FUNCTION()).Times(::testing::Exactly(0));
  auto setJumpMocker = MOCKER(cpp_free_mock::RuntimePatcherImpl::SetJump);
  EXPECT_CALL(*setJumpMocker, MOCK_FUNCTION(_, _, _)).Times(::testing::Exactly(1)).WillOnce(::testing::Return(0));
  std::vector<char> binary_backup;
  char data[16];
  char data2[16];
  cpp_free_mock::RuntimePatcher::GraftFunction(data, data2, binary_backup);
}
