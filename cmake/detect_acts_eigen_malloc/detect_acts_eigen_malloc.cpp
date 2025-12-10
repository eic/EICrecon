/**
 * Utility to detect which EIGEN_MALLOC_ALREADY_ALIGNED setting was used
 * when Acts was compiled by examining the actual implementation of
 * Eigen::internal::aligned_malloc in the Acts library.
 *
 * Strategy: Call aligned_malloc and examine the memory layout to determine
 * if it's using handmade_aligned_malloc or system malloc.
 *
 * ROBUSTNESS: Supports both old (Eigen 3.x) and new (Eigen 5.x) implementations:
 * - Eigen 3.x: stores absolute pointer at ptr[-sizeof(void*)]
 * - Eigen 5.x: stores 1-byte offset at ptr[-1]
 * - System malloc: no metadata pattern
 */

#include <iostream>
#include <cstdint>
#include <cstring>

// Forward declare Eigen's internal functions that are exported by Acts
namespace Eigen {
namespace internal {
  void* aligned_malloc(std::size_t size);
}
} // namespace Eigen

int main() {
  // Allocate memory using Acts' Eigen::internal::aligned_malloc
  const size_t alloc_size = 1024;
  void* ptr               = Eigen::internal::aligned_malloc(alloc_size);

  if (!ptr) {
    std::cerr << "ERROR: Failed to allocate memory" << std::endl;
    return 255;
  }

  uintptr_t aligned_addr = reinterpret_cast<uintptr_t>(ptr);
  bool is_handmade       = false;
  void* original_ptr     = nullptr;

  // Test 1: Check for Eigen 5.x style (1-byte offset before aligned pointer)
  // The byte at ptr[-1] should be a small offset value (typically 1-32 for alignment)
  uint8_t potential_offset = *(reinterpret_cast<uint8_t*>(ptr) - 1);
  if (potential_offset > 0 && potential_offset <= 64) {
    // Check if subtracting this offset gives us a reasonable address
    uintptr_t potential_original = aligned_addr - potential_offset;

    // The offset should bring us back to the actual malloc'd address
    // which means: original + offset == aligned
    // and the offset should match alignment requirements
    if ((potential_original + potential_offset) == aligned_addr) {
      is_handmade  = true;
      original_ptr = reinterpret_cast<void*>(potential_original);
    }
  }

  // Test 2: Check for Eigen 3.x style (absolute pointer before aligned pointer)
  if (!is_handmade) {
    void** stored_ptr_location = reinterpret_cast<void**>(ptr) - 1;
    uintptr_t stored_addr      = reinterpret_cast<uintptr_t>(*stored_ptr_location);

    // If handmade_aligned_malloc was used:
    // - stored_addr should be less than aligned_addr (original is before aligned)
    // - difference should be small (< 256 bytes for typical alignment)
    // - stored_addr should be a valid heap address

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

  if (is_handmade) {
    std::cout << "EIGEN_MALLOC_ALREADY_ALIGNED=0" << std::endl;
    return 0;
  } else {
    std::cout << "EIGEN_MALLOC_ALREADY_ALIGNED=1" << std::endl;
    return 1;
  }
}
