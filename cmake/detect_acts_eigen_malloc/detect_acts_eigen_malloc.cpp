/**
 * Utility to detect which EIGEN_MALLOC_ALREADY_ALIGNED setting was used
 * when Acts was compiled by examining the actual implementation of
 * Eigen::internal::aligned_malloc in the Acts library.
 *
 * Strategy: Use dlopen to load Acts library and dlsym to get aligned_malloc,
 * then examine the memory layout to determine allocation strategy.
 *
 * This avoids compiling with Eigen headers, which would use our own settings.
 *
 * ROBUSTNESS: Supports both old (Eigen 3.x) and new (Eigen 5.x) implementations:
 * - Eigen 3.x: stores absolute pointer at ptr[-sizeof(void*)]
 * - Eigen 5.x: stores 1-byte offset at ptr[-1]
 * - System malloc: no metadata pattern
 */

#include <iostream>
#include <cstdint>
#include <cstring>
#include <dlfcn.h>
#include <cstdlib>

// Function pointer type for aligned_malloc
typedef void* (*aligned_malloc_func)(std::size_t);

int main(int argc, char** argv) {
  // Require the Acts library path to be passed as an argument
  const char* acts_lib = nullptr;

  if (argc > 1) {
    acts_lib = argv[1];
  } else {
    std::cerr << "Usage: " << argv[0] << " /full/path/to/libActsCore.so" << std::endl;
    return 254;
  }

  // Load Acts library
  void* handle = dlopen(acts_lib, RTLD_LAZY | RTLD_GLOBAL);
  if (!handle) {
    std::cerr << "ERROR: Could not load Acts library: " << acts_lib << std::endl;
    std::cerr << "dlerror: " << dlerror() << std::endl;
    return 255;
  }

  // Find the aligned_malloc symbol
  void* sym = dlsym(handle, "_ZN5Eigen8internal14aligned_mallocEm");

  if (!sym) {
    std::cerr << "ERROR: Could not find Eigen::internal::aligned_malloc symbol" << std::endl;
    std::cerr << "dlerror: " << dlerror() << std::endl;
    dlclose(handle);
    return 255;
  }

  aligned_malloc_func aligned_malloc = reinterpret_cast<aligned_malloc_func>(sym);

  // Allocate memory using Acts' Eigen::internal::aligned_malloc
  const size_t alloc_size = 1024;
  void* ptr               = aligned_malloc(alloc_size);

  if (!ptr) {
    std::cerr << "ERROR: Failed to allocate memory" << std::endl;
    dlclose(handle);
    return 255;
  }

  uintptr_t aligned_addr = reinterpret_cast<uintptr_t>(ptr);
  bool is_handmade       = false;
  void* original_ptr     = nullptr;

  // Test 1: Check for Eigen 5.x style (1-byte offset before aligned pointer)
  uint8_t potential_offset = *(reinterpret_cast<uint8_t*>(ptr) - 1);
  if (potential_offset > 0 && potential_offset <= 64) {
    uintptr_t potential_original = aligned_addr - potential_offset;

    if ((potential_original + potential_offset) == aligned_addr) {
      is_handmade  = true;
      original_ptr = reinterpret_cast<void*>(potential_original);
    }
  }

  // Test 2: Check for Eigen 3.x style (absolute pointer before aligned pointer)
  if (!is_handmade) {
    void** stored_ptr_location = reinterpret_cast<void**>(ptr) - 1;
    uintptr_t stored_addr      = reinterpret_cast<uintptr_t>(*stored_ptr_location);

    if ((stored_addr < aligned_addr) && (aligned_addr - stored_addr < 256) && (stored_addr != 0) &&
        (stored_addr > 0x1000)) {
      is_handmade  = true;
      original_ptr = *stored_ptr_location;
    }
  }

  // Free the memory appropriately
  if (is_handmade && original_ptr) {
    std::free(original_ptr);
  } else {
    std::free(ptr);
  }

  dlclose(handle);

  if (is_handmade) {
    std::cout << "EIGEN_MALLOC_ALREADY_ALIGNED=0" << std::endl;
    return 0;
  } else {
    std::cout << "EIGEN_MALLOC_ALREADY_ALIGNED=1" << std::endl;
    return 1;
  }
}
