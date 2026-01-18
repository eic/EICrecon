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

### Commonly Modified Files
When making physics algorithm changes:
1. Check `src/algorithms/` for the relevant algorithm
2. Look for corresponding factory in `src/factories/`
3. Check for tests in `src/tests/algorithms_test/`
4. Update documentation in `docs/` if needed

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

## Data Files and Physics Validation

**Physics reconstruction requires specific input data formats:**
- Input: `.edm4hep.root` files (simulated physics events)
- Output: `.edm4eic.root` files (reconstructed physics data)

**Sample reconstruction command:**
```bash
export DETECTOR_CONFIG=${DETECTOR}_craterlake
eicrecon -Ppodio:output_file=output.edm4eic.root input_simulation.edm4hep.root
```

For physics validation, you need appropriate simulation files. Generate one for single particles using:

    ddsim --compactFile $DETECTOR_PATH/$DETECTOR_CONFIG.xml --numberOfEvents 10 --enableGun --gun.thetaMin 'pi/2' --gun.thetaMax 'pi/2' --gun.distribution uniform --gun.phiMin '0*deg' --gun.phiMax '0*deg' --gun.energy '1*GeV' --gun.particle 'e-' --outputFile sim.edm4hep.root

or using full physics events with multiple particles:

    ddsim --compactFile $DETECTOR_PATH/$DETECTOR_CONFIG.xml --numberOfEvents 10 --inputFiles root://dtn-eic.jlab.org//volatile/eic/EPIC/EVGEN/DIS/NC/10x100/minQ2=10/pythia8NCDIS_10x100_minQ2=10_beamEffects_xAngle=-0.025_hiDiv_1.hepmc3.tree.root --outputFile sim.edm4hep.root


## Conventional Commits and Breaking Changes

### Commit Message Format

It is acceptable to use [Conventional Commits specification](https://www.conventionalcommits.org/en/v1.0.0/)

**Standard commit message format:**
```
<type>[optional scope]: <description>

[optional body]

[optional footer(s)]
```

**Common commit types:**
- `feat:` - New features
- `fix:` - Bug fixes
- `docs:` - Documentation changes
- `style:` - Code style changes (formatting, etc.)
- `refactor:` - Code refactoring without functional changes
- `test:` - Adding or modifying tests
- `chore:` - Maintenance tasks, build changes
- `perf:` - Performance improvements

### Breaking Changes in EICrecon Context

**CRITICAL: Use `BREAKING CHANGE:` footer or `!` suffix for any changes that affect user workflows.**

**Consider as breaking changes:**

1. **Command-line interface changes:**
   - Changes to argument parsing that affect existing scripts
   - Modifications to configuration parameter names or behavior
   - Changes to plugin loading syntax or requirements

2. **Output collection changes:**
   - Renaming of output collection names (e.g., `EcalBarrelClusters` → `ECALClusters`)
   - Removal of existing output collections that users depend on
   - Significant changes to collection content structure or data members
   - Changes to default output file naming conventions

**Non-breaking changes (safe to implement without `BREAKING CHANGE`):**
- Algorithm improvements that produce better but compatible results
- Addition of new optional command-line arguments
- Addition of new output collections alongside existing ones
- Performance optimizations that don't change interfaces
- Internal refactoring that doesn't affect user-facing APIs
