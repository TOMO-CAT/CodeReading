#include <iostream>

#include "cpp_free_mock/cpp_free_mock.h"

std::string func() {
  return "Non mocked.";
}

TEST(HelloWorld, First) {
  EXPECT_CALL(*MOCKER(func), MOCK_FUNCTION()).Times(::testing::Exactly(1)).WillOnce(::testing::Return("Hello world."));
  EXPECT_EQ("Hello world.", func());
}
