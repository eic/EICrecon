# EICrecon Development Instructions

EICrecon is a JANA2-based reconstruction software for the ePIC detector.

**ALWAYS follow these instructions first and fallback to additional search and context gathering only if the information in these instructions is incomplete or found to be in error.**

## Working Effectively with EICrecon

### Essential Setup: Use eic-shell Environment

**Bootstrap the eic-shell environment:**
```bash
curl --location https://get.epic-eic.org | bash
./eic-shell
```

**Alternative if /cvmfs is available:**
```bash
singularity exec /cvmfs/singularity.opensciencegrid.org/eicweb/eic_xl:nightly eic-shell
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

### Manual Validation Scenarios

**ALWAYS perform these validation steps after making changes:**
- `eicrecon --version` - Verify executable works
- `eicrecon -L` - List factories to validate plugin loading
- Verify environment: `echo $JANA_PLUGIN_PATH && echo $LD_LIBRARY_PATH`

### Build Configurations and Sanitizers

**Available build options:**
- `-DCMAKE_BUILD_TYPE=Release` (default) or `Debug`
- `-DUSE_ASAN=ON`, `-DUSE_TSAN=ON` (cannot combine with ASAN), `-DUSE_UBSAN=ON`

### Development Workflow

**After making code changes:**
```bash
cmake --build build --target install -- -j8
ctest --test-dir build -V -R "specific_test_name"
source install/bin/eicrecon-this.sh
eicrecon --help
```

**Before committing - ALWAYS run:**
```bash
cmake --build build --target install -- -j8
ctest --test-dir build -V
```

**Before refactoring: Research existing patterns**
Search for similar implementations: `grep -r "PatternName" src/` and follow established patterns.

## Repository Structure and Navigation

### Key Directories
- `src/algorithms/` - Core physics algorithms
- `src/factories/` - JANA factory implementations
- `src/services/` - Service components
- `src/tests/` - Unit and integration tests
- `docs/` - Documentation and tutorials

### Important Test Suites
- `src/tests/algorithms_test/` - Algorithm unit tests (uses Catch2)
- `src/tests/omnifactory_test/` - Factory framework tests

### Commonly Modified Files
When making physics algorithm changes:
1. Check `src/algorithms/` for the relevant algorithm
2. Look for corresponding factory in `src/factories/`
3. Check for tests in `src/tests/algorithms_test/`
4. Update documentation in `docs/` if needed

## Completeness Checklist for Code Changes

**CRITICAL: Before marking work complete, verify ALL related changes are applied.**

**Common incompleteness patterns to avoid:**
- ✗ Suppressing warnings for some members but not ALL affected members in the scope
- ✗ Moving files without updating ALL dependent locations
- ✗ Creating backward compatibility headers without deprecation warnings

**Verification steps:**
1. Search for similar patterns: `grep -r "pattern" src/`
2. Ensure build AND tests pass after changes

## Coding Standards

### Header Organization and Sorting

**CRITICAL: Headers must be sorted alphabetically within each blank-line delimited block. CI WILL FAIL if not sorted.**

**Rules:**
1. Sort alphabetically within each group (separated by blank lines)
2. Use `<>` for system/third-party, `""` for project headers
3. Group order: system → third-party → project-specific

**Example:**
```cpp
#include <algorithm>
#include <memory>

#include <DD4hep/Detector.h>
#include <JANA/JApplication.h>

#include "algorithms/calorimetry/CalorimeterHitDigi.h"
#include "services/log/Log_service.h"
```

### clang-tidy Suppression Format

**CRITICAL: Use NOLINTBEGIN/NOLINTEND blocks. Include ALL affected members.**

**Correct approach:**
```cpp
struct InsertingVisitor {
  // NOLINTBEGIN(cppcoreguidelines-avoid-const-or-ref-data-members): Lifetime guaranteed
  JEvent& m_event;
  const std::string& m_collection_name;
  // NOLINTEND(cppcoreguidelines-avoid-const-or-ref-data-members)
};
```

**Incorrect:** `// NOLINT` inline comments (code formatters may break them)

### Deprecation Warnings for Moved Headers

**Pattern:**
```cpp
#pragma once
#pragma message("Header old/path.h is deprecated. Use new/path.h instead.")
#include "new/path.h"
```

## Timing Expectations

**NEVER CANCEL these operations:**

| Operation | Expected Time | Minimum Timeout |
|-----------|---------------|-----------------|
| Initial build | 30-60 minutes | 90 minutes |
| Incremental build | 5-15 minutes | 30 minutes |
| Full test suite | 15-30 minutes | 45 minutes |
| Individual test | 1-5 minutes | 10 minutes |

## Data Files and Physics Validation

**Physics reconstruction requires:**
- Input: `.edm4hep.root` files (simulated events)
- Output: `.edm4eic.root` files (reconstructed data)

**Sample reconstruction command:**
```bash
export DETECTOR_CONFIG=${DETECTOR}_craterlake
eicrecon -Ppodio:output_file=output.edm4eic.root input_simulation.edm4hep.root
```

Generate single particle simulation:

    ddsim --compactFile $DETECTOR_PATH/$DETECTOR_CONFIG.xml --numberOfEvents 10 --enableGun --gun.thetaMin 'pi/2' --gun.thetaMax 'pi/2' --gun.distribution uniform --gun.phiMin '0*deg' --gun.phiMax '0*deg' --gun.energy '1*GeV' --gun.particle 'e-' --outputFile sim.edm4hep.root

Generate full physics events:

    ddsim --compactFile $DETECTOR_PATH/$DETECTOR_CONFIG.xml --numberOfEvents 10 --inputFiles root://dtn-eic.jlab.org//volatile/eic/EPIC/EVGEN/DIS/NC/10x100/minQ2=10/pythia8NCDIS_10x100_minQ2=10_beamEffects_xAngle=-0.025_hiDiv_1.hepmc3.tree.root --outputFile sim.edm4hep.root


## Conventional Commits and Breaking Changes

### Commit Message Format

Use [Conventional Commits specification](https://www.conventionalcommits.org/en/v1.0.0/)

**Format:**
```
<type>[optional scope]: <description>

[optional body]

[optional footer(s)]
```

**Common types:** `feat:`, `fix:`, `docs:`, `style:`, `refactor:`, `test:`, `chore:`, `perf:`

### Breaking Changes in EICrecon Context

**CRITICAL: Use `BREAKING CHANGE:` footer or `!` suffix for changes that affect user workflows.**

**Consider as breaking:**
1. **Command-line interface changes:** argument parsing, configuration parameters, plugin loading
2. **Output collection changes:** renaming, removal, structure changes, default file naming

**Non-breaking changes:**
- Algorithm improvements with compatible results
- New optional arguments/collections
- Performance optimizations
- Internal refactoring
