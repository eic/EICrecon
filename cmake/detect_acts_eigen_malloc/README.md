# Eigen Memory Allocation Detection Test

This utility detects which `EIGEN_MALLOC_ALREADY_ALIGNED` setting was actually used when Acts was compiled.

## The Problem

Eigen's memory allocation behavior changes based on:
1. Whether AddressSanitizer (ASan) is enabled (`__SANITIZE_ADDRESS__`)
2. Platform characteristics (64-bit, glibc version, etc.)
3. Explicit configuration via `-DEIGEN_MALLOC_ALREADY_ALIGNED=X`

When `EIGEN_MALLOC_ALREADY_ALIGNED=0`, Eigen uses `handmade_aligned_malloc/free`.
When `EIGEN_MALLOC_ALREADY_ALIGNED=1`, Eigen uses system `malloc/free`.

**The issue**: If EICrecon is compiled with different settings than Acts, you can get allocation/deallocation mismatches leading to crashes or ASan errors.

## How This Detection Works

This detection test actually calls `Eigen::internal::aligned_malloc` from the Acts library and examines the runtime behavior:

1. Allocates memory using Acts' compiled `aligned_malloc` function
2. Examines the memory layout to detect the allocation strategy
3. `handmade_aligned_malloc` stores metadata before the aligned address:
   - **Eigen 3.x**: Stores absolute pointer at `ptr[-sizeof(void*)]`
   - **Eigen 5.x+**: Stores 1-byte offset at `ptr[-1]`
4. System malloc does not have this metadata pattern

The detection tests for both old and new metadata formats, making it robust across Eigen versions.

This gives us the **actual** configuration used when Acts was compiled, not a guess.

## Usage

This utility is automatically built as part of the EICrecon CMake configuration.

### Run the detection test:
```bash
./build/detect_acts_eigen_malloc
```

Or using the custom target:
```bash
cmake --build build --target check_eigen_malloc
```

Or as a CMake test:
```bash
ctest --test-dir build -R detect_acts_eigen_malloc -V
```

## Output

The utility prints one line showing the `EIGEN_MALLOC_ALREADY_ALIGNED` value:
- `EIGEN_MALLOC_ALREADY_ALIGNED=0` - Uses handmade_aligned_malloc/free
- `EIGEN_MALLOC_ALREADY_ALIGNED=1` - Uses system malloc/free
- `EIGEN_MALLOC_ALREADY_ALIGNED=<not set>` - Auto-detection mode

The exit code equals the value (0 or 1), or 255 if not explicitly set.

## Automatic Configuration

**This detection test runs automatically during CMake configuration!**

When you run `cmake -B build -S .`, the detection process:
1. Compiles and runs at configure time using `try_run()`
2. Detects Acts' actual `EIGEN_MALLOC_ALREADY_ALIGNED` value
3. Automatically sets the same value for EICrecon via `add_compile_definitions()`

This ensures EICrecon always matches Acts' configuration, preventing allocation mismatches.

## Manual Override

If you need to override the automatic detection:

```bash
cmake -B build -S . -DCMAKE_CXX_FLAGS="-DEIGEN_MALLOC_ALREADY_ALIGNED=0"
```

Command-line flags take precedence over `add_compile_definitions()`.

## Robustness

### Eigen Version Compatibility

The detection test handles known variations in `handmade_aligned_malloc` implementation:

- **Eigen 3.0-3.4** (current): Stores absolute pointer at `ptr[-sizeof(void*)]`
- **Eigen 5.0+** (future): Stores 1-byte offset at `ptr[-1]`

Both are tested, ensuring forward compatibility.

### Limitations

The detection test assumes:
1. `handmade_aligned_malloc` stores recoverable metadata before the aligned pointer
2. System malloc does not have this specific metadata pattern
3. Eigen's `aligned_malloc` is exported as a weak symbol from Acts

If Eigen fundamentally changes its allocation strategy (e.g., using `aligned_alloc`, custom allocators), the detection test may need updates. However, such changes would likely break Eigen's ABI anyway, requiring recompilation of Acts.

### Fallback Behavior

If detection fails (compile error, unexpected output), the CMake script defaults to `EIGEN_MALLOC_ALREADY_ALIGNED=0` (safest option for mixed builds).

## Related Files

- `cmake/detect_acts_eigen_malloc/detect_acts_eigen_malloc.cmake`: CMake configuration for this detection test
- `cmake/detect_acts_eigen_malloc/detect_acts_eigen_malloc.cpp`: Source code
- `Eigen/src/Core/util/Memory.h` (lines 23-60, 95-120): Eigen's implementation
- `EICrecon/CMakeLists.txt`: ASan ignorelist comment
