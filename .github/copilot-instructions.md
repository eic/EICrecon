# EICrecon Development Instructions

EICrecon is a JANA-based reconstruction software for the ePIC detector. This is a complex high-energy physics reconstruction software requiring specialized dependencies and extensive build times.

**ALWAYS follow these instructions first and fallback to additional search and context gathering only if the information in these instructions is incomplete or found to be in error.**

## Working Effectively with EICrecon

### Essential Setup: Use eic-shell Environment

**CRITICAL: This is the ONLY recommended approach for development. Manual dependency installation is complex and strongly discouraged.**

**Bootstrap the eic-shell environment:**
```bash
mkdir ~/eic
cd ~/eic
curl --location https://get.epic-eic.org | bash
./eic-shell
```

**Alternative if /cvmfs is available:**
```bash
singularity exec /cvmfs/singularity.opensciencegrid.org/eicweb/eic_xl:nightly eic-shell
```

This approach is achieved by the following GitHub Actions snippet (inside a workflow):
```yaml
    steps:
    - name: Ensure CernVM-FS is available
      uses: cvmfs-contrib/github-action-cvmfs@v5
    - name: Run inside the eic_xl:nightly environment
      uses: eic/run-cvmfs-osg-eic-shell@main
      with:
        platform-release: "eic_xl:nightly"
        run: |
          echo
```

**Setup geometry and clone EICrecon:**
```bash
source /opt/detector/epic-main/bin/thisepic.sh
git clone https://github.com/eic/EICrecon
cd EICrecon
```

### Build Process

**Configure and build (NEVER CANCEL - Build takes 30-60 minutes):**
```bash
cmake -B build -S . -DCMAKE_INSTALL_PREFIX=install
cmake --build build --target install -- -j8
```

**CRITICAL TIMING WARNING: Set timeout to 90+ minutes. Build can take 30-60 minutes even with ccache. NEVER CANCEL long-running builds.**

**Alternative debug build:**
```bash
cmake -B build -S . -DCMAKE_INSTALL_PREFIX=install -DCMAKE_BUILD_TYPE=Debug
cmake --build build --target install -- -j8
```

**Setup environment to use the build:**
```bash
source install/bin/eicrecon-this.sh
```

### Testing

**Run unit tests (NEVER CANCEL - Tests take 15-30 minutes):**
```bash
export LD_LIBRARY_PATH=$PWD/install/lib:$LD_LIBRARY_PATH
export JANA_PLUGIN_PATH=$PWD/install/lib/EICrecon/plugins${JANA_PLUGIN_PATH:+:${JANA_PLUGIN_PATH}}
ctest --test-dir build -V
```

**Test basic functionality:**
```bash
eicrecon --help
```

Expected output shows usage information with options like `-h`, `-v`, `-c`, `-Pkey=value`, etc.

**Test plugin loading (basic validation):**
```bash
eicrecon -Pplugins=podio,dd4hep
```

### Manual Validation Scenarios

**ALWAYS perform these validation steps after making changes:**

1. **Basic executable test:**
   ```bash
   eicrecon --version
   eicrecon --configs
   ```

2. **Plugin validation:**
   ```bash
   eicrecon -L  # List factories
   ```

3. **Environment validation:**
   ```bash
   echo $JANA_PLUGIN_PATH
   echo $LD_LIBRARY_PATH
   ldd install/lib/EICrecon/plugins/*.so | grep -v "not found" || echo "Missing dependencies detected"
   ```

### Build Configurations and Sanitizers

**Available build options (use in cmake configure step):**
- `-DCMAKE_BUILD_TYPE=Release` (default, fastest)
- `-DCMAKE_BUILD_TYPE=Debug` (for debugging)
- `-DUSE_ASAN=ON` (Address Sanitizer)
- `-DUSE_TSAN=ON` (Thread Sanitizer - cannot combine with ASAN)
- `-DUSE_UBSAN=ON` (Undefined Behavior Sanitizer)

**Example with sanitizers:**
```bash
cmake -B build -S . -DCMAKE_INSTALL_PREFIX=install -DCMAKE_BUILD_TYPE=Debug -DUSE_ASAN=ON -DUSE_UBSAN=ON
```

### Development Workflow

**After making code changes:**
```bash
# Rebuild (incremental build is faster)
cmake --build build --target install -- -j8

# Run relevant tests
ctest --test-dir build -V -R "specific_test_name"

# Test functionality
source install/bin/eicrecon-this.sh
eicrecon --help
```

**Before committing changes - ALWAYS run:**
```bash
# These must pass or CI will fail
cmake --build build --target install -- -j8
ctest --test-dir build -V
```

## Repository Structure and Navigation

### Key Directories
- `src/algorithms/` - Core physics algorithms
- `src/factories/` - JANA factory implementations
- `src/services/` - Service components
- `src/tests/` - Unit and integration tests
- `src/benchmarks/` - Performance benchmarks
- `docs/` - Documentation and tutorials
- `cmake/` - CMake configuration files

### Important Test Suites
- `src/tests/algorithms_test/` - Algorithm unit tests (uses Catch2)
- `src/tests/omnifactory_test/` - Factory framework tests
- `src/tests/tracking_test/` - Tracking algorithm tests
- `src/tests/geometry_navigation_test/` - Geometry tests

### Commonly Modified Files
When making physics algorithm changes:
1. Check `src/algorithms/` for the relevant algorithm
2. Look for corresponding factory in `src/factories/`
3. Check for tests in `src/tests/algorithms_test/`
4. Update documentation in `docs/` if needed

## Manual Build (NOT RECOMMENDED)

**WARNING: Manual dependency installation is extremely complex and time-consuming. Only attempt if eic-shell is absolutely unavailable.**

Manual build requires installing these dependencies in order:
- C++20 compiler (GCC 10+ or Clang 10+)
- CMake 3.24+
- Python 3.8+ with pyyaml, jinja2
- boost 1.70+
- ROOT 6.26+ (with C++17)
- JANA 2.2.0+
- fmt 9.0.0+
- spdlog 1.11.0+
- PODIO 0.17+
- EDM4hep 0.7.1+
- DD4hep 1.21+
- ACTS 33.0.0+
- Eigen 3.3+

**Each dependency build can take 15-60 minutes. Total time: 4-8 hours.**

See `docs/get-started/manual-build.md` for complete manual build instructions, but expect significant complexity and troubleshooting.

## Timing Expectations and Critical Warnings

**NEVER CANCEL these operations - they are expected to take significant time:**

| Operation | Expected Time | Minimum Timeout |
|-----------|---------------|-----------------|
| Initial build | 30-60 minutes | 90 minutes |
| Incremental build | 5-15 minutes | 30 minutes |
| Full test suite | 15-30 minutes | 45 minutes |
| Individual test | 1-5 minutes | 10 minutes |
| Manual dependency build | 4-8 hours | Not recommended |

**Use appropriate timeouts for all long-running commands. The CI system uses ccache and parallel builds which can still take significant time.**

## Common Issues and Solutions

**Build fails with missing dependencies:**
- Ensure you're in eic-shell environment
- Run `source /opt/detector/epic-main/bin/thisepic.sh`
- Check CMake configuration output for specific missing packages

**Tests fail:**
- Verify environment setup: `source install/bin/eicrecon-this.sh`
- Check library paths are correct
- Run individual failing tests for detailed output

**Performance issues:**
- Use `-j8` for parallel builds (adjust number based on available cores)
- Consider using ccache for repeated builds
- Debug builds are significantly slower than Release builds

## CI/CD Integration

The project uses GitHub Actions with the following key checks:
- Builds on multiple compilers (GCC, Clang)
- Multiple build types (Release, Debug)
- Address, Thread, and UB sanitizers
- clang-tidy and include-what-you-use checks
- Extensive physics simulation and reconstruction tests

**All commits must pass these checks. Run local builds and tests before pushing.**

## Data Files and Physics Validation

**Physics reconstruction requires specific input data formats:**
- Input: `.edm4hep.root` files (simulated physics events)
- Output: `.edm4eic.root` files (reconstructed physics data)

**Sample reconstruction command:**
```bash
export DETECTOR_CONFIG=${DETECTOR}_craterlake
eicrecon -Ppodio:output_file=output.edm4eic.root input_simulation.edm4hep.root
```

**For physics validation, you need appropriate simulation files. The CI system generates these automatically.**

## Code Style and Header Organization

### Header Include Order and Alphabetization

**CRITICAL: Headers must be sorted alphabetically within each blank-line delimited header block to pass include-what-you-use (IWYU) checks.**

**Header organization rules:**
1. **Always sort headers alphabetically within each group** separated by blank lines
2. **Group headers in this order:**
   - System/standard library headers (e.g., `<algorithm>`, `<cmath>`, `<memory>`, `<vector>`)
   - Third-party library headers (e.g., `<DD4hep/...>`, `<JANA/...>`, `<edm4eic/...>`, `<fmt/...>`)
   - Project-specific headers (e.g., `"algorithms/..."`, `"services/..."`, `"utilities/..."`)

**Example correct header ordering:**
```cpp
#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>

#include <DD4hep/Detector.h>
#include <JANA/JApplication.h>
#include <edm4eic/ClusterCollection.h>
#include <fmt/core.h>

#include "algorithms/calorimetry/CalorimeterHitDigi.h"
#include "services/geometry/dd4hep/DD4hep_service.h"
#include "services/log/Log_service.h"
```

**IWYU enforcement:** The CI system runs include-what-you-use with `--reorder` and `fix_includes.py --reorder` to automatically detect and fix header ordering violations. If headers are not properly sorted:
- CI will fail with IWYU violations
- An automatic PR may be created with header fixes
- Manual fixes must sort headers alphabetically within each group

**When editing existing code:**
- Always maintain alphabetical ordering when adding new headers
- Check that new headers are placed in the correct group (system, third-party, or project)
- Respect existing blank line separations between header groups
- Use the IWYU mapping file (`.github/iwyu.imp`) rules for header substitutions
