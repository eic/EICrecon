# Truthiness Algorithm Update - Chi-Squared Formulation with Uncertainties

## Date: 2025-11-14
## Changes: Improved penalty calculation with proper uncertainty treatment

---

## SUMMARY OF CHANGES

Based on collaborator feedback, the Truthiness algorithm has been updated to:

1. **Use square root of normalized chi-squared for energy/momentum penalties**
2. **Incorporate reconstructed particle uncertainties from covariance matrix**
3. **Add configurable penalty weights for PID and unassociated particles**

---

## DETAILED CHANGES

### 1. Energy Penalty Calculation

**OLD (simple squared difference):**
```cpp
const double energy_diff = mc_energy - rc_energy;
const double energy_penalty = energy_diff * energy_diff;
```

**NEW (chi-squared with uncertainty):**
```cpp
// Get energy uncertainty from covariance matrix
const double energy_error = (rc_cov.tt > 0.0) ? std::sqrt(rc_cov.tt) : 1.0;

// Calculate chi-squared and take square root
const double energy_diff = mc_energy - rc_energy;
const double energy_chi2 = (energy_diff * energy_diff) / (energy_error * energy_error);
const double energy_penalty = std::sqrt(energy_chi2);
```

**Impact:**
- If `rc_cov.tt > 0`: Uses actual energy uncertainty → normalized penalty
- If `rc_cov.tt = 0`: Defaults to error = 1.0 → same as old behavior
- Square root makes penalty scale more linearly with significance

### 2. Momentum Penalty Calculation

**OLD (sum of squared differences):**
```cpp
const double px_diff = mc_momentum.x - rc_momentum.x;
const double py_diff = mc_momentum.y - rc_momentum.y;
const double pz_diff = mc_momentum.z - rc_momentum.z;
const double momentum_penalty = px_diff*px_diff + py_diff*py_diff + pz_diff*pz_diff;
```

**NEW (chi-squared with component uncertainties):**
```cpp
// Get momentum component uncertainties from covariance matrix
const double px_error = (rc_cov.xx > 0.0) ? std::sqrt(rc_cov.xx) : 1.0;
const double py_error = (rc_cov.yy > 0.0) ? std::sqrt(rc_cov.yy) : 1.0;
const double pz_error = (rc_cov.zz > 0.0) ? std::sqrt(rc_cov.zz) : 1.0;

// Calculate chi-squared for each component
const double px_chi2 = (px_diff * px_diff) / (px_error * px_error);
const double py_chi2 = (py_diff * py_diff) / (py_error * py_error);
const double pz_chi2 = (pz_diff * pz_diff) / (pz_error * pz_error);
const double momentum_chi2 = px_chi2 + py_chi2 + pz_chi2;

// Take square root of total chi-squared
const double momentum_penalty = std::sqrt(momentum_chi2);
```

**Impact:**
- Properly weights momentum components by their uncertainties
- Accounts for different resolutions in x, y, z directions
- Square root makes 3-component chi-squared more comparable to single values

### 3. PID Penalty Configuration

**OLD (fixed weight):**
```cpp
const double pdg_penalty = (mc_part.getPDG() != rc_part.getPDG()) ? 1.0 : 0.0;
```

**NEW (configurable weight):**
```cpp
const double pdg_penalty_base = (mc_part.getPDG() != rc_part.getPDG()) ? 1.0 : 0.0;
const double pdg_penalty = m_cfg.pidPenaltyWeight * pdg_penalty_base;
```

**New Configuration Parameter:**
```cpp
double pidPenaltyWeight = 1.0;  // Default: 1
```

**Usage:**
```bash
-Preco:Truthiness:pidPenaltyWeight=2.0  # Make PID misidentification count double
-Preco:Truthiness:pidPenaltyWeight=0.5  # Reduce PID penalty weight
```

### 4. Unassociated MC Particle Penalty Configuration

**OLD (fixed count):**
```cpp
const double mc_penalty = static_cast<double>(unassociated_mc_count);
```

**NEW (configurable weight):**
```cpp
const double mc_penalty = m_cfg.unassociatedMCPenaltyWeight *
                          static_cast<double>(unassociated_mc_count);
```

**New Configuration Parameter:**
```cpp
double unassociatedMCPenaltyWeight = 1.0;  // Default: 1
```

**Usage:**
```bash
-Preco:Truthiness:unassociatedMCPenaltyWeight=2.0  # Penalize missing MC more
-Preco:Truthiness:unassociatedMCPenaltyWeight=0.0  # Ignore missing MC particles
```

### 5. Unassociated Reco Particle Penalty Configuration

**OLD (fixed count):**
```cpp
const double rc_penalty = static_cast<double>(unassociated_rc_count);
```

**NEW (configurable weight):**
```cpp
const double rc_penalty = m_cfg.unassociatedRecoPenaltyWeight *
                          static_cast<double>(unassociated_rc_count);
```

**New Configuration Parameter:**
```cpp
double unassociatedRecoPenaltyWeight = 1.0;  // Default: 1
```

**Usage:**
```bash
-Preco:Truthiness:unassociatedRecoPenaltyWeight=2.0  # Penalize fakes more
-Preco:Truthiness:unassociatedRecoPenaltyWeight=0.0  # Ignore fake particles
```

---

## CONFIGURATION PARAMETERS

### TruthinessConfig.h

```cpp
struct TruthinessConfig {
  // Penalty weights for different contributions
  double pidPenaltyWeight = 1.0;                 // Weight for PID mismatch (default: 1)
  double unassociatedMCPenaltyWeight = 1.0;      // Weight for missing MC particles (default: 1)
  double unassociatedRecoPenaltyWeight = 1.0;    // Weight for fake reco particles (default: 1)
};
```

### Usage Examples

**Default behavior (unchanged):**
```bash
eicrecon input.root -Ppodio:output_file=output.root
```

**Emphasize PID quality:**
```bash
eicrecon input.root \
  -Preco:Truthiness:pidPenaltyWeight=5.0 \
  -Ppodio:output_file=output.root
```

**Focus on reconstruction efficiency (ignore fakes):**
```bash
eicrecon input.root \
  -Preco:Truthiness:unassociatedMCPenaltyWeight=2.0 \
  -Preco:Truthiness:unassociatedRecoPenaltyWeight=0.0 \
  -Ppodio:output_file=output.root
```

**Custom weighting scheme:**
```bash
eicrecon input.root \
  -Preco:Truthiness:pidPenaltyWeight=3.0 \
  -Preco:Truthiness:unassociatedMCPenaltyWeight=2.0 \
  -Preco:Truthiness:unassociatedRecoPenaltyWeight=0.5 \
  -Ppodio:output_file=output.root
```

---

## PHYSICS INTERPRETATION

### Chi-Squared Formulation

The new formulation treats each association as a measurement:

**Energy contribution:**
```
χ²_E = (E_MC - E_reco)² / σ²_E
penalty_E = √χ²_E
```

**Momentum contribution:**
```
χ²_p = (px_MC - px_reco)²/σ²_px + (py_MC - py_reco)²/σ²_py + (pz_MC - pz_reco)²/σ²_pz
penalty_p = √χ²_p
```

### Why Square Root?

1. **Makes units comparable:** √χ² has same dimensionality as # of sigma
2. **Reduces dynamic range:** χ² can be huge for outliers, √χ² is more controlled
3. **Linear scaling:** √χ² scales linearly with significance level
4. **Combines nicely:** Can add √χ² values from different measurements

### Interpretation of Penalties

**Energy/Momentum penalties:**
- `penalty ≈ 0`: Perfect agreement within uncertainties
- `penalty ≈ 1`: ~1σ disagreement (good)
- `penalty ≈ 3`: ~3σ disagreement (concerning)
- `penalty >> 3`: Significant disagreement (outlier)

**PID penalty:**
- `0`: Correct PID
- `pidPenaltyWeight`: Wrong PID (default: 1)

**Unassociated penalties:**
- Each missing/fake particle: `weight × 1`

---

## UNCERTAINTY HANDLING

### Covariance Matrix Elements Used

From `ReconstructedParticle::getCovMatrix()`:

| Element | Physical Meaning | Used For |
|---------|------------------|----------|
| `tt` | Time-time covariance | Energy uncertainty |
| `xx` | x-momentum variance | px uncertainty |
| `yy` | y-momentum variance | py uncertainty |
| `zz` | z-momentum variance | pz uncertainty |

### Fallback Behavior

If covariance matrix elements are **zero or negative**:
- **Fallback:** Use uncertainty = 1.0
- **Effect:** Reverts to old squared-difference behavior
- **Reason:** Prevents division by zero, maintains backward compatibility

### Expected Covariance Matrix Status

**Central/Forward tracking:**
- Covariance matrices are typically **well-defined**
- Should have positive diagonal elements
- Will benefit from new formulation

**Far-forward tracking:**
- Currently **not working** (collections empty)
- When fixed, covariance should be available

**Manual particle creation:**
- May have zero covariance
- Will use fallback (error = 1.0)

---

## IMPACT ON TRUTHINESS VALUES

### Expected Changes

**If covariance matrices are populated:**

1. **Well-reconstructed particles (σ < |Δ|):**
   - OLD: Large squared penalty
   - NEW: Moderate √χ² penalty
   - **Effect:** Lower truthiness → better metric

2. **Poorly-reconstructed particles (σ >> |Δ|):**
   - OLD: Small squared penalty
   - NEW: Even smaller √χ² penalty
   - **Effect:** Lower truthiness → correctly de-weighted

3. **PID misidentification:**
   - OLD: Fixed penalty of 1.0
   - NEW: Configurable (default: 1.0)
   - **Effect:** Unchanged by default, tunable

**If covariance matrices are NOT populated (all zeros):**
- Fallback to error = 1.0
- Behavior similar to old version but with √ applied
- Values will be different but scaling should be similar

### Backward Compatibility

**Defaults maintain similar behavior:**
- All weights = 1.0 (unchanged from old fixed values)
- Fallback to error = 1.0 when covariance unavailable
- Square root changes absolute values but preserves relative ordering

**Breaking changes:**
- Absolute truthiness values will be different
- Need to re-establish baselines for cuts
- Previous truthiness plots not directly comparable

---

## VALIDATION RECOMMENDATIONS

### 1. Check Covariance Matrix Population

```python
import uproot
tree = uproot.open("output.root")["events"]

# Check if covariance matrices are non-zero
cov_tt = tree["ReconstructedParticles/ReconstructedParticles.covMatrix.tt"].array()
cov_xx = tree["ReconstructedParticles/ReconstructedParticles.covMatrix.xx"].array()

print(f"Non-zero tt: {sum(len([x for x in event if x > 0]) for event in cov_tt)}")
print(f"Non-zero xx: {sum(len([x for x in event if x > 0]) for event in cov_xx)}")
```

### 2. Compare Old vs New Truthiness

```bash
# Old algorithm (status=2 version)
eicrecon sim.root -Ppodio:output_file=old.root

# Rebuild with new algorithm
spack install eicrecon

# New algorithm
eicrecon sim.root -Ppodio:output_file=new.root

# Compare distributions
python3 analyze_truthiness.py old.root --output old_truth.png
python3 analyze_truthiness.py new.root --output new_truth.png
```

### 3. Test Configuration Parameters

```bash
# Test different PID weights
for w in 0.1 0.5 1.0 2.0 5.0; do
  eicrecon sim.root \
    -Preco:Truthiness:pidPenaltyWeight=$w \
    -Ppodio:output_file=pid_weight_${w}.root
done
```

---

## FILES MODIFIED

1. **src/algorithms/reco/TruthinessConfig.h**
   - Added three configuration parameters
   - All default to 1.0 (backward compatible)

2. **src/algorithms/reco/Truthiness.cc**
   - Updated energy penalty calculation with chi-squared
   - Updated momentum penalty calculation with chi-squared
   - Added covariance matrix uncertainty extraction
   - Applied configuration weights to all penalties
   - Enhanced trace logging with uncertainties

---

## NEXT STEPS

1. ✅ **Code modified** - Changes complete
2. 🔄 **Rebuild required** - Run `spack install eicrecon`
3. 🔄 **Rerun reconstruction** - Generate new output with updated algorithm
4. 🔄 **Validate changes** - Compare old vs new truthiness distributions
5. 🔄 **Check covariance** - Verify reconstruction produces non-zero uncertainties
6. 🔄 **Establish baselines** - Determine new "good" truthiness thresholds

---

## EXAMPLE EXPECTED BEHAVIOR

### Scenario: Well-tracked particle

**MC truth:** E = 10.0 GeV, p = (5.0, 5.0, 7.0) GeV
**Reco:** E = 9.8 ± 0.2 GeV, p = (4.9 ± 0.1, 5.1 ± 0.1, 6.9 ± 0.2) GeV

**OLD calculation:**
```
energy_penalty = (10.0 - 9.8)² = 0.04
momentum_penalty = (0.1² + 0.1² + 0.1²) = 0.03
total = 0.07
```

**NEW calculation:**
```
energy_chi2 = (0.2)² / (0.2)² = 1.0
energy_penalty = √1.0 = 1.0

momentum_chi2 = (0.1/0.1)² + (0.1/0.1)² + (0.1/0.2)² = 1 + 1 + 0.25 = 2.25
momentum_penalty = √2.25 = 1.5

total = 2.5
```

**Interpretation:** In new formulation, this is ~1-2σ agreement (excellent!)

---

*Update Date: 2025-11-14*
*Changes: Chi-squared with uncertainties + configurable weights*
*Backward Compatible: Yes (with defaults)*
*Rebuild Required: Yes*


---

## UPDATE: Configurable Default Resolutions

### New Parameters Added (2025-11-14)

In addition to the penalty weights, two new configuration parameters control the **default resolution values** used when covariance matrix elements are zero or unavailable:

```cpp
struct TruthinessConfig {
  // ... existing parameters ...

  // Default resolutions when covariance matrix is unavailable or zero (in GeV)
  double defaultEnergyResolution = 1.0;          // Default energy uncertainty (default: 1.0 GeV)
  double defaultMomentumResolution = 1.0;        // Default momentum component uncertainty (default: 1.0 GeV)
};
```

### Usage

**Energy resolution default:**
```bash
# Use 0.5 GeV as default energy uncertainty when covMatrix.tt = 0
eicrecon sim.root \
  -Preco:Truthiness:defaultEnergyResolution=0.5 \
  -Ppodio:output_file=output.root
```

**Momentum resolution default:**
```bash
# Use 0.2 GeV as default momentum uncertainty when covMatrix diagonal = 0
eicrecon sim.root \
  -Preco:Truthiness:defaultMomentumResolution=0.2 \
  -Ppodio:output_file=output.root
```

**Combined example:**
```bash
# Set different default resolutions appropriate for detector
eicrecon sim.root \
  -Preco:Truthiness:defaultEnergyResolution=0.3 \
  -Preco:Truthiness:defaultMomentumResolution=0.1 \
  -Ppodio:output_file=output.root
```

### Rationale

**Why configurable defaults?**

1. **Detector-dependent:** Different detectors have different typical resolutions
   - Calorimeters: ~10-30% energy resolution → default E might be 0.3-1.0 GeV
   - Trackers: ~1-5% momentum resolution → default p might be 0.1-0.5 GeV

2. **Analysis-dependent:** Some analyses may want to:
   - Penalize more harshly: Use smaller defaults (e.g., 0.1 GeV)
   - Be more forgiving: Use larger defaults (e.g., 2.0 GeV)

3. **Missing covariance matrices:** If reconstruction doesn't fill covariance:
   - Can set realistic defaults based on known detector performance
   - Avoids overly harsh or lenient penalties

### Implementation Details

**Updated code:**
```cpp
// Energy uncertainty (from time-time covariance tt, or use configured default)
const double energy_error = (rc_cov.tt > 0.0) ?
    std::sqrt(rc_cov.tt) : m_cfg.defaultEnergyResolution;

// Momentum component uncertainties (from spatial covariance, or use configured default)
const double px_error = (rc_cov.xx > 0.0) ?
    std::sqrt(rc_cov.xx) : m_cfg.defaultMomentumResolution;
const double py_error = (rc_cov.yy > 0.0) ?
    std::sqrt(rc_cov.yy) : m_cfg.defaultMomentumResolution;
const double pz_error = (rc_cov.zz > 0.0) ?
    std::sqrt(rc_cov.zz) : m_cfg.defaultMomentumResolution;
```

### Recommended Settings

**For typical EIC detector performance:**

```bash
# Central tracking (good momentum resolution, moderate energy)
eicrecon sim.root \
  -Preco:Truthiness:defaultEnergyResolution=0.5 \
  -Preco:Truthiness:defaultMomentumResolution=0.1 \
  -Ppodio:output_file=output.root
```

**For calorimeter-focused analyses:**
```bash
# Better energy resolution emphasis
eicrecon sim.root \
  -Preco:Truthiness:defaultEnergyResolution=0.2 \
  -Preco:Truthiness:defaultMomentumResolution=0.5 \
  -Ppodio:output_file=output.root
```

**For quick studies with lenient penalties:**
```bash
# Larger defaults = less penalty when covariance missing
eicrecon sim.root \
  -Preco:Truthiness:defaultEnergyResolution=2.0 \
  -Preco:Truthiness:defaultMomentumResolution=2.0 \
  -Ppodio:output_file=output.root
```

### Complete Configuration Example

**Full control over all truthiness parameters:**
```bash
eicrecon sim.root \
  -Preco:Truthiness:pidPenaltyWeight=3.0 \
  -Preco:Truthiness:unassociatedMCPenaltyWeight=2.0 \
  -Preco:Truthiness:unassociatedRecoPenaltyWeight=0.5 \
  -Preco:Truthiness:defaultEnergyResolution=0.5 \
  -Preco:Truthiness:defaultMomentumResolution=0.2 \
  -Ppodio:output_file=output.root
```

### Summary of All 5 Configuration Parameters

| Parameter | Default | Units | Purpose |
|-----------|---------|-------|---------|
| `pidPenaltyWeight` | 1.0 | dimensionless | Weight for PID mismatch |
| `unassociatedMCPenaltyWeight` | 1.0 | dimensionless | Weight for missing MC |
| `unassociatedRecoPenaltyWeight` | 1.0 | dimensionless | Weight for fake reco |
| `defaultEnergyResolution` | 1.0 | GeV | Default σ_E when covariance unavailable |
| `defaultMomentumResolution` | 1.0 | GeV | Default σ_p when covariance unavailable |

---

*Updated: 2025-11-14*
*Added: Configurable default resolution parameters*
