#pragma once

#include <cstdlib>
#include <vector>

namespace cpp_free_mock {

namespace RuntimePatcherImpl {
// Need impl in architecture relevant file.
// OS
static int UnprotectMemoryForOnePage(void* const address);
// CPU
static int SetJump(void* const address, const void* const destination, std::vector<char>& binary_backup);
static void RevertJump(void* address, const std::vector<char>& binary_backup);
}  // namespace RuntimePatcherImpl

}  // namespace cpp_free_mock

#if defined(__x86_64__) || defined(__i386__)
#include "cpp_free_mock/x86/runtime_patch_impl.h"
#endif

#ifdef __APPLE__
#include "cpp_free_mock/posix/runtime_patch_impl.h"
#elif __linux__
#include "cpp_free_mock/posix/runtime_patch_impl.h"
#endif

namespace cpp_free_mock {

// Provide 2 interface, GraftFunction and RevertGraft.
struct RuntimePatcher {
  template <typename F1, typename F2>
  static int GraftFunction(F1 address, F2 destination, std::vector<char>& binary_backup) {
    void* function = reinterpret_cast<void*>((std::size_t&)address);
    if (0 != RuntimePatcherImpl::UnprotectMemoryForOnePage(function)) {
      std::abort();
    } else {
      // For mock std::abort, this need not after the 'if'.
      return RuntimePatcherImpl::SetJump(function, reinterpret_cast<void*>((std::size_t&)destination), binary_backup);
    }
  }

  template <typename F>
  static void RevertGraft(F address, const std::vector<char>& binary_backup) {
    RuntimePatcherImpl::RevertJump(reinterpret_cast<void*>((std::size_t&)address), binary_backup);
  }
};

}  // namespace cpp_free_mock
