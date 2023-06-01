#pragma once

#include "cpp_free_mock/impl.h"

#define MOCKER_INTERNAL(function, identity) \
  ::cpp_free_mock::MockerCreator::GetMocker<::cpp_free_mock::TypeForUniqMocker<identity>>(function, #function)
