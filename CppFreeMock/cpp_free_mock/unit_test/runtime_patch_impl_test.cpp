// CppFreeMock: a tool for mock global function, member function, class static function.
//
// cpp_free_mock::RuntimePatcherImpl under posix test.

#include <iostream>
#include <vector>

#include "cpp_free_mock/cpp_free_mock.h"
#include "cpp_free_mock/runtime_patch.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::testing::_;

struct TestRuntimePatcherImpl : public ::testing::Test {
  virtual void TearDown() {
    CLEAR_MOCKER();
  }
};

TEST_F(TestRuntimePatcherImpl, AlignAddress) {
  std::cout << "PageSize: " << cpp_free_mock::RuntimePatcherImpl::PageSize << std::endl;
  for (size_t i = 0; i < cpp_free_mock::RuntimePatcherImpl::PageSize; ++i) {
    EXPECT_EQ(0u, cpp_free_mock::RuntimePatcherImpl::AlignAddress(i, cpp_free_mock::RuntimePatcherImpl::PageSize));
  }
}

TEST_F(TestRuntimePatcherImpl, mprotect) {
  auto mprotectMocker = MOCKER(mprotect);
  EXPECT_CALL(*mprotectMocker, MOCK_FUNCTION(nullptr, 0, 0))
      .Times(::testing::Exactly(1))
      .WillOnce(::testing::Return(9047));
  EXPECT_EQ(9047, mprotect(nullptr, 0, 0));
}

TEST_F(TestRuntimePatcherImpl, UnprotectMemory) {
  auto alignAddressMocker = MOCKER(cpp_free_mock::RuntimePatcherImpl::AlignAddress);
  void* address = nullptr;
  size_t length = 0;
  EXPECT_CALL(*alignAddressMocker, MOCK_FUNCTION(0, cpp_free_mock::RuntimePatcherImpl::PageSize))
      .Times(::testing::Exactly(1))
      .WillOnce(::testing::Return(0));
  EXPECT_EQ(0, cpp_free_mock::RuntimePatcherImpl::UnprotectMemory(address, length));
}

TEST_F(TestRuntimePatcherImpl, UnprotectMemoryForOnePage) {
  auto unprotectMemoryMocker = MOCKER(cpp_free_mock::RuntimePatcherImpl::UnprotectMemory);
  EXPECT_CALL(*unprotectMemoryMocker, MOCK_FUNCTION(nullptr, cpp_free_mock::RuntimePatcherImpl::PageSize))
      .Times(::testing::Exactly(1))
      .WillOnce(::testing::Return(9047));
  EXPECT_EQ(9047, cpp_free_mock::RuntimePatcherImpl::UnprotectMemoryForOnePage(nullptr));
}

TEST_F(TestRuntimePatcherImpl, IsDistanceOverflow) {
  // Non overflow.
  for (long long i = INT32_MIN; i <= INT32_MAX; i += 0x10000000) {
    EXPECT_FALSE(cpp_free_mock::RuntimePatcherImpl::IsDistanceOverflow(i));
  }
  EXPECT_FALSE(cpp_free_mock::RuntimePatcherImpl::IsDistanceOverflow(INT32_MAX));
  // Overflow/
  long long distance = INT32_MIN;
  --distance;
  EXPECT_TRUE(cpp_free_mock::RuntimePatcherImpl::IsDistanceOverflow(distance));
  distance = INT32_MAX;
  ++distance;
  EXPECT_TRUE(cpp_free_mock::RuntimePatcherImpl::IsDistanceOverflow(distance));
  EXPECT_TRUE(cpp_free_mock::RuntimePatcherImpl::IsDistanceOverflow(INT64_MIN));
  EXPECT_TRUE(cpp_free_mock::RuntimePatcherImpl::IsDistanceOverflow(INT64_MAX));
}

TEST_F(TestRuntimePatcherImpl, SetJump64bitAddress) {
  char data[16];
  void* function = reinterpret_cast<void*>(data);
  auto patchFunction64bitAddressMocker = MOCKER(cpp_free_mock::RuntimePatcherImpl::PatchFunction64bitAddress);
  auto calculateDistanceMocker = MOCKER(cpp_free_mock::RuntimePatcherImpl::CalculateDistance);
  EXPECT_CALL(*calculateDistanceMocker, MOCK_FUNCTION(function, nullptr))
      .Times(::testing::Exactly(1))
      .WillOnce(::testing::Return(INT64_MAX));
  EXPECT_CALL(*patchFunction64bitAddressMocker, MOCK_FUNCTION(reinterpret_cast<char*>(function), nullptr))
      .Times(::testing::Exactly(1));
  std::vector<char> binary_backup;
  EXPECT_EQ(0, cpp_free_mock::RuntimePatcherImpl::SetJump(function, nullptr, binary_backup));
}

TEST_F(TestRuntimePatcherImpl, SetJump) {
  char data[16];
  char data2[16];
  void* function = reinterpret_cast<void*>(data);
  void* destination = reinterpret_cast<void*>(data2);
  auto patchFunction64bitAddressMocker = MOCKER(cpp_free_mock::RuntimePatcherImpl::PatchFunction64bitAddress);
  auto patchFunction32bitDistanceMocker = MOCKER(cpp_free_mock::RuntimePatcherImpl::PatchFunction32bitDistance);
  EXPECT_CALL(*patchFunction64bitAddressMocker, MOCK_FUNCTION(_, _)).Times(::testing::Exactly(0));
  EXPECT_CALL(*patchFunction32bitDistanceMocker, MOCK_FUNCTION(reinterpret_cast<char*>(function), _))
      .Times(::testing::Exactly(1));
  std::vector<char> binary_backup;
  EXPECT_EQ(0, cpp_free_mock::RuntimePatcherImpl::SetJump(function, destination, binary_backup));
}
