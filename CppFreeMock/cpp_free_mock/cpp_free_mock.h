#pragma once

#include "cpp_free_mock/internal_macro.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

// mocker is the variable name in user code.
// Used to create:
//  1, global function;
//  2, static member function;
//  3, member function mocker that can check this pointer.
#define MOCKER(function) MOCKER_INTERNAL(function, __COUNTER__)

#define CLEAR_MOCKER ::cpp_free_mock::MockerCreator::RestoreAllMockerFunctionToReal

// Used in EXPECT_CALL(*mocker, MOCK_FUNCTION(_))
#define MOCK_FUNCTION CppFreeMockStubFunction
