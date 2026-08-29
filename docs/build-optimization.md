# Build Optimization in EICrecon

This document describes build-time optimizations implemented in EICrecon to reduce compilation time.

## Precompiled JOmniFactory Templates

### Overview

EICrecon uses the extern template pattern to precompile common factory instantiations, eliminating redundant template instantiation across multiple plugin files. This optimization was implemented based on ClangBuildAnalyzer analysis (Recommendation #6).

**Expected build time savings: 150-200+ seconds per full build**

### Technical Background

The `JOmniFactory` template is instantiated once for each factory class (e.g., `JOmniFactory<TrackProjector_factory, TrackProjectorConfig>`). Without precompilation, each plugin file that uses a factory independently instantiates the template, leading to:
- Redundant parsing and code generation
- Increased memory usage during compilation
- Slower link times

The extern template pattern allows us to:
1. Declare that a template instantiation exists elsewhere (`extern template`)
2. Explicitly instantiate it in exactly one compilation unit
3. All other translation units link to the precompiled version

### Implementation

For each factory subdirectory (`src/factories/*/`), the build system automatically:

1. **Parses factory headers** at CMake configure time to discover all factories
2. **Generates precompiled sources**:
   - `build/src/factories/*/generated/factories.h` - extern template declarations
   - `build/src/factories/*/generated/factories.cc` - explicit instantiations
3. **Builds static libraries**: `libfactories_*_precompiled.a`
4. **Plugins include generated headers** instead of individual factory headers

### CMake Implementation

The code generation logic lives in `cmake/factory_precompile.cmake` and provides:

```cmake
generate_factory_precompile_sources(<target_name> <source_dir>)
```

This function:
- Globs for `*_factory.h` headers in the current factory subdirectory
- Parses each header to extract:
  - Factory class name (e.g., `TrackProjector_factory`)
  - Config type (e.g., `TrackProjectorConfig` or `NoConfig` or `EmptyConfig`)
  - Namespace (typically `eicrecon::`)
- Generates `factories.h` with extern template declarations
- Generates `factories.cc` with explicit instantiations and full includes
- Creates a static precompiled library target

**Regex patterns used:**
- With config: `class\s+(\w+)\s*:\s*public\s+JOmniFactory<\1,\s*(\w+)>`
- Without config: `class\s+(\w+)\s*:\s*public\s+JOmniFactory<\1>`

### Factory Subdirectories

Currently precompiled:
- `src/factories/tracking/` - Track reconstruction factories
- `src/factories/reco/` - General reconstruction factories
- `src/factories/calorimetry/` - Calorimeter reconstruction
- `src/factories/digi/` - Digitization factories
- `src/factories/fardetectors/` - Far-forward detector factories
- `src/factories/meta/` - Meta-data and utility factories
- `src/factories/particle_flow/` - Particle flow reconstruction
- `src/factories/pid/` - Particle identification
- `src/factories/pid_lut/` - PID lookup tables

### Plugin Integration

**Global plugins** (`src/global/*/`):
```cpp
// Before: individual includes
#include "factories/tracking/TrackProjector_factory.h"
#include "factories/tracking/TrackPropagation_factory.h"
// ... many more

// After: single precompiled header
#include "factories/tracking/generated/factories.h"
```

**Detector plugins** (`src/detectors/*/`):
Same pattern - replace individual factory includes with generated precompiled headers.

Each plugin's `CMakeLists.txt` must include:
```cmake
target_include_directories(${PLUGIN_NAME}_plugin PRIVATE ${CMAKE_BINARY_DIR}/src)
```

This allows the compiler to find generated headers at `factories/*/generated/factories.h`.

### Automatic Synchronization

**No manual maintenance required!**

When a developer adds or removes a factory:
1. CMake reconfigures (automatically on next build, or run `cmake -B build`)
2. The generation script re-parses all factory headers
3. Generated files are updated with the new factory list
4. Precompiled libraries are rebuilt with the new instantiations

**The system stays in sync automatically** because:
- Generated files are marked as `GENERATED` in CMake
- They're excluded from git (in build directory)
- CMake tracks header file timestamps as dependencies
- Any change to a factory header triggers regeneration

### Limitations and Caveats

1. **Multi-line factory declarations** may not parse correctly if the inheritance spans multiple lines. Keep factory class declarations on one line:
   ```cpp
   // Good
   class MyFactory_factory : public JOmniFactory<MyFactory_factory, MyFactoryConfig> {

   // May not parse
   class MyFactory_factory :
       public JOmniFactory<MyFactory_factory, MyFactoryConfig> {
   ```

2. **Namespace requirements**: Factories and their config structs should be in the `eicrecon::` namespace for consistency.

3. **Template specializations** would need additional handling in the generation script.

4. **Incremental builds**: Changes to factory headers trigger regeneration and rebuild of precompiled libraries, which can add a few seconds to incremental builds.

### Verification

To verify precompiled factories are working:

```bash
# Check generated files exist
ls -l build/src/factories/*/generated/factories.{h,cc}

# Check precompiled libraries were built
ls -lh build/src/factories/*/libfactories_*_precompiled.a

# Check library sizes (should be non-trivial for complex factories)
# tracking and reco libraries should be 1-2 MB each
```

### Troubleshooting

**Problem**: CMake warnings about "Could not parse factory pattern"
- **Cause**: Factory class declaration spans multiple lines or uses unusual syntax
- **Fix**: Ensure the class declaration fits the expected pattern on one line

**Problem**: Plugins fail to compile with "undefined reference" to factory methods
- **Cause**: Plugin doesn't include the generated header
- **Fix**: Replace individual factory includes with `#include "factories/SUBSYSTEM/generated/factories.h"`

**Problem**: "No such file or directory" error for generated headers
- **Cause**: Plugin CMakeLists.txt missing include directory
- **Fix**: Add `target_include_directories(${PLUGIN_NAME}_plugin PRIVATE ${CMAKE_BINARY_DIR}/src)`

### Historical Context

This optimization was implemented based on ClangBuildAnalyzer analysis that showed:
- JOmniFactoryGeneratorT: 62.3s (203 instantiations, avg 306ms each)
- JOmniFactory PreInit: 32.9s (201 instantiations, avg 163ms each)
- Total template instantiation cost: ~104s

The precompiled factory approach eliminates this redundancy across 27 plugin files (5 global + 22 detector plugins) that previously each instantiated their own copies of these templates.

### Related Optimizations

- **PR #2903**: Monolithic PODIO includes (100-150s savings)
- Combined with other ClangBuildAnalyzer recommendations, total potential savings: 500-800 seconds

### Future Work

Potential additional optimizations:
- Precompile other frequently-instantiated templates beyond JOmniFactory
- Use PCH (precompiled headers) for common system headers
- Further modularize factory libraries to reduce compile-time dependencies
