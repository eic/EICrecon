# Implementation Plan: Type Traits for Pulse Shapes

## Overview
This plan implements type traits for pulse shapes to enable algorithmic optimizations while keeping EvaluatorPulse for research purposes. The approach uses compile-time and runtime trait detection to select optimal vs conservative code paths.

## Background

### Current State (rm-evaluator-pulse branch)
- EvaluatorPulse has been removed from SignalPulse
- Only LandauPulse remains as a hardcoded pulse shape
- PR #2363 aims to simplify code and enforce single-peak assumptions from #2362

### **CRITICAL**: Existing Code Already Makes Trait Assumptions!
**PR #2362 (commit bb9296c15)** added an optimization that **already assumes**:
- ✓ **Unimodal** - single peak (no multiple peaks)
- ✓ **Monotonic derivative after peak** - once falling below threshold, won't rise again

**The optimization** (lines 144-152 in PulseGeneration.cc on main):
```cpp
if (std::abs(signal) < m_cfg.ignore_thres) {
  if (!passed_threshold) {
    auto diff = std::abs(signal) - std::abs(previous);
    previous  = signal;
    if (diff >= 0) {
      skip_bins = i;
      continue;  // Still rising before threshold
    } else {
      break;  // ASSUMES: falling + no threshold = passed single peak, done
    }
  }
}
```

**This breaks** for multi-modal or oscillating pulses. **This is the core problem**: the code makes assumptions without enforcing them, which is exactly why the reviewer wants traits!

### Reviewer Feedback (from PR #2363)
> "We do need some assumptions on regularity of the pulse shape in order to optimize the algorithms. This can be uni-modality, monotonicity of the derivative, continuity, etc. It is hard to ensure those with an arbitrary pulse shape. We could add type traits to the hardcoded pulses and assume the EvaluatorPulse does not have the trait, which would force its evaluation to be less optimal."

### Goals
1. **Keep EvaluatorPulse** for R&D and validation studies
2. **Add type traits** to describe mathematical properties of pulse shapes
3. **Make existing optimizations conditional** on traits (fix current bug!)
4. **Enable additional optimizations** for pulses with known good properties
5. **Degrade gracefully** for arbitrary pulse shapes (EvaluatorPulse)

## Design: Pulse Shape Type Traits

### Trait Categories

#### 1. Continuity Traits
```cpp
struct ContinuousTrait {};           // Continuous function
struct DifferentiableTrait {};       // First derivative exists everywhere
struct TwiceDifferentiableTrait {};  // Second derivative exists everywhere
```

#### 2. Monotonicity Traits
```cpp
struct MonotonicIncreasingTrait {};  // Always non-decreasing
struct MonotonicDecreasingTrait {};  // Always non-increasing
struct PiecewiseMonotonicTrait {};   // Monotonic in regions
```

#### 3. Modality Traits
```cpp
struct UnimodalTrait {               // Single peak/mode
  // Can include peak location method if known analytically
  virtual double getPeakTime(double charge) const = 0;
};

struct MultimodalTrait {};           // Multiple peaks
```

#### 4. Boundedness Traits
```cpp
struct BoundedTrait {                // Function has finite support
  virtual double getMinTime() const = 0;
  virtual double getMaxTime() const = 0;
};

struct AsymptoticTrait {};           // Goes to zero asymptotically
```

#### 5. Symmetry Traits
```cpp
struct SymmetricTrait {              // Symmetric around peak
  virtual double getSymmetryCenter() const = 0;
};

struct AsymmetricTrait {};           // Not symmetric
```

### Implementation Strategy

#### Option A: CRTP-based Traits (Compile-time)
```cpp
template <typename... Traits>
class SignalPulseWithTraits : public SignalPulse, public Traits... {
public:
  template <typename T>
  static constexpr bool has_trait() {
    return (std::is_base_of_v<T, Traits> || ...);
  }
};

class LandauPulse : public SignalPulseWithTraits<
    ContinuousTrait,
    DifferentiableTrait,
    UnimodalTrait,
    AsymmetricTrait> {
  // Implementation
};

class EvaluatorPulse : public SignalPulse {
  // No traits - uses conservative algorithms
};
```

**Pros:**
- Zero runtime overhead for trait checking
- Type-safe at compile time
- Clear trait declaration

**Cons:**
- Factory pattern becomes more complex
- May need runtime polymorphism wrapper

#### Option B: Runtime Trait Queries (Virtual methods)
```cpp
class SignalPulse {
public:
  virtual ~SignalPulse() = default;
  virtual double operator()(double time, double charge) = 0;
  virtual double getMaximumTime() const = 0;

  // Trait queries
  virtual bool isContinuous() const { return false; }
  virtual bool isUnimodal() const { return false; }
  virtual bool hasBoundedSupport() const { return false; }
  virtual bool isMonotonic() const { return false; }

  // Optional trait-specific methods
  virtual std::optional<double> getPeakTime(double charge) const { return {}; }
  virtual std::optional<std::pair<double, double>> getSupportBounds() const { return {}; }
};
```

**Pros:**
- Simple factory pattern unchanged
- Easy to query traits at runtime
- Gradual adoption possible

**Cons:**
- Small runtime overhead for virtual calls (~1-2ns per call)
- Trait checks happen inside hot loop (100M calls per 1M hits)

#### **Option C: Const Member Cache (RECOMMENDED)**
```cpp
class SignalPulse {
protected:
  // Compile-time trait info cached as const runtime values
  const bool m_is_unimodal;
  const bool m_is_continuous;
  const bool m_has_bounded_support;

  SignalPulse(bool unimodal, bool continuous, bool bounded)
    : m_is_unimodal(unimodal)
    , m_is_continuous(continuous)
    , m_has_bounded_support(bounded)
  {}

public:
  virtual ~SignalPulse() = default;
  virtual double operator()(double time, double charge) = 0;
  virtual double getMaximumTime() const = 0;

  // Non-virtual trait queries (inlineable!)
  bool isUnimodal() const { return m_is_unimodal; }
  bool isContinuous() const { return m_is_continuous; }
  bool hasBoundedSupport() const { return m_has_bounded_support; }

  // Optional trait-specific methods (still virtual)
  virtual std::optional<double> getPeakTime(double charge) const { return {}; }
  virtual std::optional<std::pair<double, double>> getSupportBounds() const { return {}; }
};

class LandauPulse : public SignalPulse {
  LandauPulse(std::vector<double> params)
    : SignalPulse(true, true, false)  // unimodal, continuous, unbounded
  {
    // ... initialize parameters
  }

  std::optional<double> getPeakTime(double /*charge*/) const override {
    return m_hit_sigma_offset * m_sigma_analog;
  }
};

class EvaluatorPulse : public SignalPulse {
  EvaluatorPulse(const std::string& expr, const std::vector<double>& params)
    : SignalPulse(false, false, false)  // conservative: no traits
  {
    // ... setup evaluator
  }
};
```

**Pros:**
- **Zero runtime overhead** - trait checks inline to single memory load
- **Branch predictor friendly** - const values lead to perfect prediction
- Simple implementation (no template magic)
- Compatible with factory pattern
- Type-safe at construction

**Cons:**
- Traits immutable after construction (this is actually a feature)
- Optional methods still virtual (but called rarely)

#### Recommended: Option C (Const Member Cache)
This combines the best of both worlds: compile-time trait information (set at construction) with zero-overhead runtime queries (inlined). The pulse type is determined at runtime (from config), but trait values are const and inline perfectly.

## Implementation Steps

### Phase 1: Trait Infrastructure (Week 1)

#### Step 1.1: Update SignalPulse base class with const trait cache
**File:** `src/algorithms/digi/PulseGeneration.cc`
```cpp
class SignalPulse {
protected:
  // Trait cache - set once at construction from compile-time knowledge
  const bool m_is_unimodal;
  const bool m_is_continuous;
  const bool m_has_bounded_support;

  // Protected constructor for derived classes to set traits
  SignalPulse(bool unimodal, bool continuous, bool bounded)
    : m_is_unimodal(unimodal)
    , m_is_continuous(continuous)
    , m_has_bounded_support(bounded)
  {}

public:
  virtual ~SignalPulse() = default;

  // Pulse evaluation (virtual - this is the expensive operation)
  virtual double operator()(double time, double charge) = 0;
  virtual double getMaximumTime() const = 0;

  // Trait queries (non-virtual - inlineable, zero overhead)
  bool isUnimodal() const { return m_is_unimodal; }
  bool isContinuous() const { return m_is_continuous; }
  bool hasBoundedSupport() const { return m_has_bounded_support; }

  // Optional trait-specific data (virtual - called rarely)
  virtual std::optional<double> getPeakTime(double charge) const { return {}; }
  virtual std::optional<std::pair<double, double>> getSupportBounds() const { return {}; }
};
```

**Why this design?**
- Boolean traits are const members → inline to single memory load
- Optional data methods are virtual → flexibility for complex cases
- No separate PulseTraits.h needed → simpler
- Traits set at construction → immutable, type-safe

#### Step 1.2: Add traits to LandauPulse
```cpp
class LandauPulse : public SignalPulse {
public:
  LandauPulse(std::vector<double> params)
    : SignalPulse(
        true,   // is_unimodal: Landau has single peak
        true,   // is_continuous: smooth function
        false   // has_bounded_support: infinite tail
      )
  {
    if ((params.size() != 2) && (params.size() != 3)) {
      throw std::runtime_error(
          "LandauPulse requires 2 or 3 parameters, gain, sigma_analog, [hit_sigma_offset], got " +
          std::to_string(params.size()));
    }
    m_gain         = params[0];
    m_sigma_analog = params[1];
    if (params.size() == 3) {
      m_hit_sigma_offset = params[2];
    }
  }

  double operator()(double time, double charge) override {
    return charge * m_gain *
           TMath::Landau(time, m_hit_sigma_offset * m_sigma_analog, m_sigma_analog, kTRUE);
  }

  double getMaximumTime() const override {
    return m_hit_sigma_offset * m_sigma_analog;
  }

  // Override optional trait methods for better performance
  std::optional<double> getPeakTime(double /*charge*/) const override {
    return m_hit_sigma_offset * m_sigma_analog;
  }

private:
  double m_gain             = 1.0;
  double m_sigma_analog     = 1.0;
  double m_hit_sigma_offset = 3.5;
};
```

#### Step 1.3: Re-add EvaluatorPulse (conservative traits)
Restore the EvaluatorPulse implementation from commit before 081e70132:
```cpp
class EvaluatorPulse : public SignalPulse {
public:
  EvaluatorPulse(const std::string& expression, const std::vector<double>& params)
    : SignalPulse(
        false,  // is_unimodal: unknown, be conservative
        false,  // is_continuous: unknown, be conservative
        false   // has_bounded_support: unknown, be conservative
      )
  {
    std::vector<std::string> keys = {"time", "charge"};
    for (std::size_t i = 0; i < params.size(); i++) {
      std::string p = "param" + std::to_string(i);
      if (expression.find(p) == std::string::npos) {
        throw std::runtime_error("Parameter " + p + " not found in expression");
      }
      keys.push_back(p);
      param_map[p] = params[i];
    }

    if (expression.find("time") == std::string::npos) {
      throw std::runtime_error("Parameter [time] not found in expression");
    }
    if (expression.find("charge") == std::string::npos) {
      throw std::runtime_error("Parameter [charge] not found in expression");
    }

    auto& serviceSvc = algorithms::ServiceSvc::instance();
    m_evaluator      = serviceSvc.service<EvaluatorSvc>("EvaluatorSvc")->_compile(expression, keys);
  }

  double operator()(double time, double charge) override {
    param_map["time"]   = time;
    param_map["charge"] = charge;
    return m_evaluator(param_map);
  }

  double getMaximumTime() const override {
    return 0;  // Unknown for arbitrary expressions
  }

private:
  std::unordered_map<std::string, double> param_map;
  std::function<double(const std::unordered_map<std::string, double>&)> m_evaluator;
};
```

**Key point**: All boolean traits are `false` (conservative), so EvaluatorPulse will use slower but correct algorithms.

### Phase 2: Fix Existing Optimization + Add New Ones (Week 2)

#### Step 2.1: **FIX BUG**: Make existing derivative-based early exit conditional on traits

**Current implementation on main** (assumes unimodal without checking):
```cpp
float previous = 0;
for (std::uint32_t i = 0; i < m_cfg.max_time_bins; i++) {
  double t    = signal_time + i * m_cfg.timestep - time;
  auto signal = (*m_pulse)(t, charge);

  if (std::abs(signal) < m_cfg.ignore_thres) {
    if (!passed_threshold) {
      auto diff = std::abs(signal) - std::abs(previous);
      previous  = signal;
      if (diff >= 0) {
        skip_bins = i;
        continue;  // Rising
      } else {
        break;  // BUG: assumes unimodal - may miss second peak!
      }
    } else if (t > m_min_sampling_time) {
      break;
    }
  }
  passed_threshold = true;
  pulse.push_back(signal);
  integral += signal;
}
```

**FIXED: Make optimization conditional on unimodal trait:**
```cpp
float previous = 0;
for (std::uint32_t i = 0; i < m_cfg.max_time_bins; i++) {
  double t    = signal_time + i * m_cfg.timestep - time;
  auto signal = (*m_pulse)(t, charge);

  if (std::abs(signal) < m_cfg.ignore_thres) {
    if (!passed_threshold) {
      // ONLY use derivative-based early exit if pulse is unimodal
      if (m_pulse->isUnimodal()) {
        auto diff = std::abs(signal) - std::abs(previous);
        previous  = signal;
        if (diff >= 0) {
          skip_bins = i;
          continue;  // Rising
        } else {
          break;  // Falling without threshold: safe for unimodal
        }
      } else {
        // Conservative: keep searching (may have multiple peaks)
        skip_bins = i;
        continue;
      }
    } else if (t > m_min_sampling_time) {
      break;
    }
  }
  passed_threshold = true;
  pulse.push_back(signal);
  integral += signal;
}
```

**Impact:**
- LandauPulse: Same performance (isUnimodal() returns true)
- EvaluatorPulse: Correct behavior (won't early-exit and miss peaks)
- Multi-modal pulse shapes: Can be added safely

#### Step 2.2: Add bounded support optimization
```cpp
if (m_pulse->hasBoundedSupport()) {
  auto bounds = m_pulse->getSupportBounds();
  if (bounds) {
    auto [t_min, t_max] = *bounds;
    int start_bin = std::max(0, static_cast<int>(
        std::floor(t_min / m_cfg.timestep)));
    int end_bin = std::min(m_cfg.max_time_bins, static_cast<uint32_t>(
        std::ceil(t_max / m_cfg.timestep)));

    // Only sample within bounded support
    for (std::uint32_t i = start_bin; i < end_bin; i++) {
      // ... sampling logic ...
    }
  }
}
```

### Phase 3: Additional Pulse Shapes (Week 3)

#### Step 3.1: Implement GaussianPulse with traits
```cpp
class GaussianPulse : public SignalPulse {
public:
  GaussianPulse(std::vector<double> params);

  double operator()(double time, double charge) override;
  double getMaximumTime() const override;

  // Traits
  bool isContinuous() const override { return true; }
  bool isUnimodal() const override { return true; }
  bool hasBoundedSupport() const override { return false; }
  bool hasMonotonicRegions() const override { return true; }

  std::optional<double> getPeakTime(double /*charge*/) const override {
    return m_mean;
  }

private:
  double m_gain;
  double m_mean;
  double m_sigma;
};
```

#### Step 3.2: Update PulseShapeFactory
```cpp
static std::unique_ptr<SignalPulse> createPulseShape(
    const std::string& type,
    const std::vector<double>& params) {
  if (type == "LandauPulse") {
    return std::make_unique<LandauPulse>(params);
  }
  if (type == "GaussianPulse") {
    return std::make_unique<GaussianPulse>(params);
  }

  // Fallback to EvaluatorPulse for arbitrary expressions
  try {
    return std::make_unique<EvaluatorPulse>(type, params);
  } catch (...) {
    throw std::invalid_argument("Unable to make pulse shape type: " + type);
  }
}
```

### Phase 4: Testing and Validation (Week 4)

#### Step 4.1: Unit tests for traits
**File:** `src/tests/algorithms_test/digi_PulseTraits.cc`
```cpp
TEST_CASE("LandauPulse has expected traits") {
  LandauPulse pulse({1.0, 0.1});
  REQUIRE(pulse.isContinuous());
  REQUIRE(pulse.isUnimodal());
  REQUIRE(!pulse.hasBoundedSupport());

  auto peak = pulse.getPeakTime(1.0);
  REQUIRE(peak.has_value());
  REQUIRE(*peak == Approx(0.35));  // 3.5 * 0.1
}

TEST_CASE("EvaluatorPulse has no traits") {
  EvaluatorPulse pulse("charge * exp(-time)", {});
  REQUIRE(!pulse.isContinuous());  // Conservative default
  REQUIRE(!pulse.isUnimodal());
  REQUIRE(!pulse.hasBoundedSupport());

  REQUIRE(!pulse.getPeakTime(1.0).has_value());
}
```

#### Step 4.2: Integration tests
**File:** `src/tests/algorithms_test/digi_PulseGeneration.cc`

Add test cases:
1. LandauPulse produces identical results with and without optimizations
2. EvaluatorPulse still works for arbitrary expressions
3. Performance comparison between trait-optimized and conservative paths

#### Step 4.3: Benchmark performance
**File:** `src/benchmarks/digi_pulse_benchmark.cc`

Measure:
- Time to generate pulses with LandauPulse (optimized path)
- Time to generate pulses with EvaluatorPulse (conservative path)
- Memory allocation patterns

### Phase 5: Documentation (Week 4)

#### Step 5.1: Update algorithm documentation
**File:** `docs/algorithms/digi/pulse_generation.md`

Document:
- Available pulse shapes and their traits
- Performance characteristics
- When to use EvaluatorPulse vs hardcoded shapes
- How to implement new pulse shapes with traits

#### Step 5.2: Add developer guide
**File:** `docs/developers/pulse_shape_traits.md`

Guide for:
- Adding new pulse shapes
- Implementing trait methods correctly
- Testing trait implementations
- Performance considerations

## Workflow Setup

### Create new worktree and branch
```bash
# From main worktree
cd /home/wdconinc/git/EICrecon
git worktree add .worktree/pulse-shape-traits -b pulse-shape-traits origin/main

# Switch to new worktree
cd .worktree/pulse-shape-traits

# Ensure eic-shell environment is active
# (Already documented in AGENTS.md bootstrap process)
```

### Development workflow
```bash
# Make changes following phases above
# Build incrementally
cmake -B build -S . -DCMAKE_INSTALL_PREFIX=install
cmake --build build --target install -- -j8

# Run specific tests during development
source install/bin/eicrecon-this.sh
ctest --test-dir build -V -R "PulseGeneration"

# Before committing
cmake --build build --target install -- -j8
ctest --test-dir build -V
```

## Commit Strategy

Using Conventional Commits:

### Phase 1 commits:
```
feat: add PulseTraits interface for pulse shape properties
feat: implement trait queries in LandauPulse
feat: restore EvaluatorPulse with conservative trait defaults
```

### Phase 2 commits:
```
perf: optimize pulse sampling for unimodal pulses
perf: add bounded support optimization for pulse generation
```

### Phase 3 commits:
```
feat: add GaussianPulse with trait support
refactor: update PulseShapeFactory with trait-aware pulse selection
```

### Phase 4 commits:
```
test: add unit tests for pulse shape traits
test: add integration tests for trait-based optimizations
perf: add benchmarks for pulse generation performance
```

### Phase 5 commits:
```
docs: document pulse shape traits system
docs: add developer guide for implementing pulse shapes
```

## Breaking Changes

**This should NOT be a breaking change** if implemented correctly:
- Existing configurations using "LandauPulse" continue to work
- EvaluatorPulse is restored (unbreaking the earlier removal)
- New trait methods have default implementations (no changes required to existing code)
- Optimizations are internal to PulseGeneration algorithm

## Success Criteria

1. ✅ EvaluatorPulse works for arbitrary pulse shapes
2. ✅ LandauPulse and other hardcoded shapes have correct trait declarations
3. ✅ Optimized code path produces identical results to conservative path
4. ✅ Measurable performance improvement for trait-optimized pulses
5. ✅ All existing tests pass
6. ✅ New tests validate trait system
7. ✅ Documentation explains trait system clearly

## Future Extensions

### Advanced Traits
- `ConvexTrait` / `ConcaveTrait` for second-derivative properties
- `AnalyticalIntegralTrait` for pulses with closed-form integrals
- `SeparableTrait` for pulses where f(t, q) = g(t) * h(q)

### Performance Optimizations
- Cache pulse evaluations for repeated time samples
- Vectorized pulse evaluation (SIMD)
- Lookup table generation for EvaluatorPulse

### Additional Pulse Shapes
- Double-exponential pulse (CR-RC shaper)
- Polynomial pulse shapes
- Piecewise-defined pulses

## Risk Assessment

### Low Risk
- Adding trait methods with defaults (backward compatible)
- Restoring EvaluatorPulse (was recently removed, well-tested)

### Medium Risk
- Algorithm optimizations may introduce subtle bugs
  - **Mitigation:** Comprehensive testing, validation against conservative path

### High Risk
- Performance regression if trait queries are expensive
  - **Mitigation:** Profile early, use hybrid CRTP approach if needed

## Timeline

- **Week 1:** Phase 1 - Trait infrastructure
- **Week 2:** Phase 2 - Algorithm optimizations
- **Week 3:** Phase 3 - Additional pulse shapes
- **Week 4:** Phase 4 & 5 - Testing, documentation, benchmarking

**Total: 4 weeks to MVP**

## Open Questions

1. **What other pulse shapes are needed?** (Ask physics community)
2. **What optimization thresholds make sense?** (Profile with realistic data)
3. **Should traits be user-configurable?** (Probably not - safety issue)
4. **What about runtime pulse shape switching?** (Factory handles this)

## References

- PR #2363: https://github.com/eic/EICrecon/pull/2363
- PR #2362: (mentioned in context, need to review for single-peak assumptions)
- Conventional Commits: https://www.conventionalcommits.org/
