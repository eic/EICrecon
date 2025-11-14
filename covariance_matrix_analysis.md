# Covariance Matrix Analysis - EICrecon Reconstruction

## Date: 2025-11-14
## Critical Finding: NO covariance matrices are populated in reconstruction

---

## EXECUTIVE SUMMARY

**100% of reconstructed particles lack covariance matrices** in the current EICrecon
reconstruction. This means the Truthiness algorithm's chi-squared formulation is
using **default resolutions (1.0 GeV) for all particles** instead of actual
reconstruction uncertainties.

---

## ANALYSIS RESULTS

### Covariance Matrix Population Status

**From rec_dis_10x100_minQ2=100_craterlake.edm4hep.root (100 events, 1309 particles):**

| Covariance Element | Non-Zero Count | Percentage | Status |
|-------------------|----------------|------------|---------|
| `covMatrix.tt` (energy) | **0** / 1309 | **0.0%** | ❌ EMPTY |
| `covMatrix.xx` (px) | **0** / 1309 | **0.0%** | ❌ EMPTY |
| `covMatrix.yy` (py) | **0** / 1309 | **0.0%** | ❌ EMPTY |
| `covMatrix.zz` (pz) | **0** / 1309 | **0.0%** | ❌ EMPTY |

**Result:** All 1309 particles (100%) have ZERO covariance → using defaults!

---

## PARTICLE TYPES WITHOUT COVARIANCE

### All Particle Types Affected

| PDG Code | Name | Count | % of Total |
|----------|------|-------|------------|
| 0 | Unknown/Neutral | 709 | 54.2% |
| 211 | π+ | 221 | 16.9% |
| -211 | π- | 203 | 15.5% |
| 11 | electron | 40 | 3.1% |
| -321 | K- | 37 | 2.8% |
| 321 | K+ | 36 | 2.7% |
| 2212 | proton | 27 | 2.1% |
| -11 | positron | 25 | 1.9% |
| -2212 | antiproton | 11 | 0.8% |

**Observation:** ALL particle types lack covariance, from charged tracks to neutrals.

---

## IMPLICATIONS FOR TRUTHINESS

### What This Means

1. **Chi-Squared Calculation:**
   ```
   energy_penalty = √[(E_MC - E_reco)² / σ²_E]

   With covMatrix.tt = 0:
   σ_E = defaultEnergyResolution = 1.0 GeV  (fallback)

   energy_penalty = √[(E_MC - E_reco)² / 1.0²] = |E_MC - E_reco|
   ```

2. **Momentum Calculation:**
   ```
   momentum_penalty = √[Σ(Δp_i)² / σ²_i]

   With covMatrix.xx/yy/zz = 0:
   σ_px = σ_py = σ_pz = defaultMomentumResolution = 1.0 GeV

   momentum_penalty = √[(Δpx)² + (Δpy)² + (Δpz)²] / 1.0 = |Δp|
   ```

3. **Effective Penalty:**
   - Energy penalty = absolute energy difference (GeV)
   - Momentum penalty = absolute 3-momentum difference magnitude (GeV)
   - NOT properly normalized by uncertainties
   - All particles treated as having 1 GeV uncertainty

### Truthiness Values Observed

From analysis of 100 events:
- **Mean truthiness: 44.73**
- **Median truthiness: 35.18**
- **Range: 2.69 - 146.11**

These values are **artificially high** because:
- Using fixed 1.0 GeV uncertainties
- Well-measured particles (σ << 1 GeV) get same penalty as poorly-measured
- No statistical weighting by actual measurement quality

---

## WHY COVARIANCE MATRICES ARE EMPTY

### Possible Reasons

#### 1. Reconstruction Doesn't Fill Covariance (Most Likely)

**Tracking:**
- CKF tracking may not populate covariance matrices in output
- Track parameter covariances exist internally but not propagated to EDM4hep
- Need to check tracking algorithms

**Particle Flow:**
- Particle flow reconstruction combines tracks + clusters
- May not propagate uncertainties to final particles
- Covariance matrix filling not implemented

#### 2. EDM4hep Schema Issue

- Covariance matrix fields exist but not mapped correctly
- Conversion from internal format to EDM4hep may lose info
- Need to check podio output module

#### 3. Configuration Issue

- Covariance filling may need to be explicitly enabled
- Missing parameter to turn on uncertainty calculation
- Not enabled by default for performance?

---

## COMPARISON WITH EXPECTED BEHAVIOR

### What SHOULD Happen

**For a well-tracked particle (e.g., central pion):**
```
MC:   E = 2.0 GeV, p = (0.5, 0.5, 1.5) GeV
Reco: E = 1.95 ± 0.05 GeV, p = (0.48 ± 0.02, 0.51 ± 0.02, 1.49 ± 0.03) GeV

Expected penalties with actual uncertainties:
  energy_chi2 = (0.05)² / (0.05)² = 1.0
  energy_penalty = √1.0 = 1.0  (~1σ, good!)

  momentum_chi2 = (0.02/0.02)² + (0.01/0.02)² + (0.01/0.03)² ≈ 1.4
  momentum_penalty = √1.4 ≈ 1.2  (~1σ, good!)
```

**What ACTUALLY Happens (with defaults):**
```
MC:   E = 2.0 GeV, p = (0.5, 0.5, 1.5) GeV
Reco: E = 1.95 ± 1.0 GeV, p = (0.48 ± 1.0, 0.51 ± 1.0, 1.49 ± 1.0) GeV
                ↑ DEFAULT         ↑ DEFAULTS

Actual penalties with default uncertainties:
  energy_chi2 = (0.05)² / (1.0)² = 0.0025
  energy_penalty = √0.0025 = 0.05  (way too small!)

  momentum_chi2 = (0.02/1.0)² + (0.01/1.0)² + (0.01/1.0)² ≈ 0.0006
  momentum_penalty = √0.0006 ≈ 0.024  (way too small!)
```

**Result:** Good tracks get nearly zero penalty because defaults are too large!

---

## IMPACT ON PHYSICS

### Current Situation

**Without covariance matrices:**
- ✅ Algorithm runs without errors (graceful fallback)
- ✅ Provides *some* quality metric
- ❌ NOT properly weighted by measurement quality
- ❌ All particles treated as having same ~1 GeV uncertainty
- ❌ Cannot distinguish well-measured vs poorly-measured tracks

**Particle-type bias:**
- **High-momentum particles (> 10 GeV):** Under-penalized (1 GeV default too large)
- **Low-momentum particles (< 1 GeV):** Over-penalized (1 GeV default too large)
- **Central tracks (good σ):** Under-penalized
- **Forward tracks (worse σ):** May be correctly penalized by chance

### Comparison with Previous Version

**Old algorithm (squared differences):**
```
penalty = (ΔE)² + Σ(Δp_i)²
```

**New algorithm (with defaults):**
```
penalty = |ΔE| + |Δp|
```

**Effect:** New algorithm with defaults actually gives LINEAR scaling instead of
QUADRATIC, which may be better than old algorithm even without covariance!

---

## RECOMMENDATIONS

### Immediate Actions

1. **✅ Document that covariance matrices are not filled**
   - Current Truthiness values are using default resolutions
   - Not yet using proper chi-squared formulation
   - Algorithm is backward-compatible fallback mode

2. **📊 Tune default resolutions for typical detector performance**
   ```bash
   # For central tracking (good momentum resolution)
   -Preco:Truthiness:defaultEnergyResolution=0.5
   -Preco:Truthiness:defaultMomentumResolution=0.1

   # For calorimeters (better energy resolution)
   -Preco:Truthiness:defaultEnergyResolution=0.2
   -Preco:Truthiness:defaultMomentumResolution=0.3
   ```

3. **✅ Current algorithm still useful**
   - Linear penalty (|ΔE| + |Δp|) is reasonable
   - Better than quadratic for outliers
   - Provides event quality metric

### High Priority: Fix Covariance Filling

4. **🔧 Investigate tracking covariance output**
   - Check if CKF tracking fills covariance internally
   - Verify track parameter covariances exist
   - Map to EDM4hep ReconstructedParticle covMatrix

5. **🔧 Investigate particle flow covariance**
   - Check how tracks + clusters combine uncertainties
   - Implement covariance propagation in particle flow
   - May need error propagation formulas

6. **🔧 Check EDM4hep output module**
   - Verify podio writer correctly maps covariances
   - Check if explicit configuration needed
   - Review examples from other experiments

### Medium Priority: Validation

7. **📊 Test with simple events**
   - Single tracks at different momenta
   - Check if ANY particles get covariance
   - Identify where propagation breaks

8. **📊 Compare with other detectors**
   - Check if ePIC has covariance filling
   - Review ATHENA reconstruction
   - Learn from working examples

9. **📝 Add covariance diagnostic**
   - Tool to check covariance population rate
   - Warnings if covariance always zero
   - Validation plots

---

## WORKAROUND: Tuned Default Resolutions

Until covariance matrices are filled, tune defaults based on detector performance:

### Central Region (|η| < 1, barrel)

**Tracking dominant:**
```bash
-Preco:Truthiness:defaultMomentumResolution=0.05  # 5% at 1 GeV ≈ 0.05 GeV
-Preco:Truthiness:defaultEnergyResolution=0.3     # From calorimeter
```

### Forward Region (1 < η < 3)

**Tracking + calorimeter:**
```bash
-Preco:Truthiness:defaultMomentumResolution=0.1   # Moderate
-Preco:Truthiness:defaultEnergyResolution=0.4     # Moderate
```

### Far-Forward Region (η ≥ 3)

**Calorimeter dominant:**
```bash
-Preco:Truthiness:defaultMomentumResolution=0.5   # Poor tracking
-Preco:Truthiness:defaultEnergyResolution=0.5     # Calorimeter
```

### Momentum-Dependent (Better)

Ideally, default should scale with momentum:
```
σ_p ≈ α × p + β
```

But current algorithm uses fixed defaults. Could add momentum-dependent defaults in future.

---

## CONCLUSIONS

### Key Findings

1. **100% of particles lack covariance matrices** in current reconstruction
2. **All particle types affected** - charged, neutral, all PDG codes
3. **Truthiness using default 1.0 GeV uncertainties** for everything
4. **Algorithm working but not optimal** - fallback mode functional

### Current Status

**Truthiness Algorithm:**
- ✅ Implemented with chi-squared formulation
- ✅ Graceful fallback to defaults
- ✅ Provides useful quality metric
- ❌ NOT using actual reconstruction uncertainties
- ❌ Cannot distinguish measurement quality

**Reconstruction:**
- ✅ Produces particles successfully
- ✅ Good momentum/energy measurements
- ❌ NOT filling covariance matrices
- ❌ Missing uncertainty information

### Path Forward

**Short-term:** Use tuned default resolutions
**Long-term:** Fix reconstruction to fill covariance matrices

The chi-squared formulation is ready and waiting for proper uncertainties!

---

*Analysis Date: 2025-11-14*
*Dataset: DIS 10x100 GeV, 100 events, 1309 particles*
*Finding: Zero covariance matrix population*
*Status: Algorithm in fallback mode, reconstruction needs covariance filling*
