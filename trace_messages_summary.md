# Truthiness Algorithm - Trace Messages for Default Resolution Usage

## Date: 2025-11-14
## Addition: Diagnostic trace messages for covariance matrix fallback

---

## WHAT WAS ADDED

New trace-level logging messages that indicate when default resolution values are used
instead of actual covariance matrix uncertainties.

---

## IMPLEMENTATION

### Energy Uncertainty Trace

When `covMatrix.tt` is zero or unavailable, a trace message is printed:

```cpp
const bool energy_has_cov = (rc_cov.tt > 0.0);
const double energy_error = energy_has_cov ? std::sqrt(rc_cov.tt) : m_cfg.defaultEnergyResolution;
if (!energy_has_cov) {
  trace("  Energy uncertainty using default: {:.3f} GeV (covMatrix.tt = {})",
        m_cfg.defaultEnergyResolution, rc_cov.tt);
}
```

**Example output:**
```
  Energy uncertainty using default: 1.000 GeV (covMatrix.tt = 0)
```

### Momentum Uncertainty Trace

When any momentum component covariance (`xx`, `yy`, `zz`) is zero or unavailable:

```cpp
const bool px_has_cov = (rc_cov.xx > 0.0);
const bool py_has_cov = (rc_cov.yy > 0.0);
const bool pz_has_cov = (rc_cov.zz > 0.0);
const double px_error = px_has_cov ? std::sqrt(rc_cov.xx) : m_cfg.defaultMomentumResolution;
const double py_error = py_has_cov ? std::sqrt(rc_cov.yy) : m_cfg.defaultMomentumResolution;
const double pz_error = pz_has_cov ? std::sqrt(rc_cov.zz) : m_cfg.defaultMomentumResolution;
if (!px_has_cov || !py_has_cov || !pz_has_cov) {
  trace("  Momentum uncertainty using default for [{},{},{}]: {:.3f} GeV (covMatrix: xx={}, yy={}, zz={})",
        px_has_cov ? "cov" : "def", py_has_cov ? "cov" : "def", pz_has_cov ? "cov" : "def",
        m_cfg.defaultMomentumResolution, rc_cov.xx, rc_cov.yy, rc_cov.zz);
}
```

**Example outputs:**
```
  Momentum uncertainty using default for [def,def,def]: 1.000 GeV (covMatrix: xx=0, yy=0, zz=0)
  Momentum uncertainty using default for [cov,def,cov]: 1.000 GeV (covMatrix: xx=0.01, yy=0, zz=0.02)
```

---

## WHY THIS IS USEFUL

### 1. Diagnostics

**Quickly identify if covariance matrices are populated:**
```bash
eicrecon sim.root -Ppodio:output_file=output.root 2>&1 | grep "using default"
```

**Count how many particles lack covariance:**
```bash
eicrecon sim.root -Ppodio:output_file=output.root 2>&1 | grep -c "Energy uncertainty using default"
```

### 2. Validation

**Check reconstruction quality:**
- If many defaults → covariance matrices not being filled properly
- If few defaults → reconstruction is providing uncertainties (good!)

**Example check:**
```bash
# Enable trace logging
export JANA_LOG_LEVEL=trace

# Run reconstruction
eicrecon sim.root -Ppodio:output_file=output.root 2>&1 | tee recon_trace.log

# Analyze trace output
echo "Energy uncertainties using default:"
grep "Energy uncertainty using default" recon_trace.log | wc -l

echo "Momentum uncertainties using default:"
grep "Momentum uncertainty using default" recon_trace.log | wc -l

echo "Total associations:"
grep "Association: MC PDG" recon_trace.log | wc -l
```

### 3. Algorithm Tuning

**Identify which components need defaults:**
- `[def,def,def]` → All momentum components using defaults
- `[cov,def,cov]` → Only py missing covariance
- `[cov,cov,cov]` → All components have covariance (no message printed)

**Helps decide if defaults need tuning:**
```bash
# If many particles use defaults, might want different default values
-Preco:Truthiness:defaultEnergyResolution=0.5
-Preco:Truthiness:defaultMomentumResolution=0.2
```

---

## EXAMPLE TRACE OUTPUT

### Particle WITH Covariance (Good)

```
[trace] Association: MC PDG=211 (E=5.234, p=[2.100,1.500,4.200]) <-> RC PDG=211 (E=5.180±0.150, p=[2.090±0.080,1.510±0.075,4.150±0.120])
[trace]   Energy penalty: 0.360 (χ²=0.130), Momentum penalty: 0.447 (χ²=0.200), PDG penalty: 0.000
```

No trace messages about defaults → covariance matrix was used!

### Particle WITHOUT Energy Covariance

```
[trace]   Energy uncertainty using default: 1.000 GeV (covMatrix.tt = 0)
[trace] Association: MC PDG=211 (E=5.234, p=[2.100,1.500,4.200]) <-> RC PDG=211 (E=5.180±1.000, p=[2.090±0.080,1.510±0.075,4.150±0.120])
[trace]   Energy penalty: 0.054 (χ²=0.003), Momentum penalty: 0.447 (χ²=0.200), PDG penalty: 0.000
```

Energy using default (1.0 GeV) → might want to tune this!

### Particle WITHOUT Momentum Covariance

```
[trace]   Momentum uncertainty using default for [def,def,def]: 1.000 GeV (covMatrix: xx=0, yy=0, zz=0)
[trace] Association: MC PDG=211 (E=5.234, p=[2.100,1.500,4.200]) <-> RC PDG=211 (E=5.180±0.150, p=[2.090±1.000,1.510±1.000,4.150±1.000])
[trace]   Energy penalty: 0.360 (χ²=0.130), Momentum penalty: 0.070 (χ²=0.005), PDG penalty: 0.000
```

All momentum components using default → reconstruction not filling momentum covariance!

### Particle with PARTIAL Momentum Covariance

```
[trace]   Momentum uncertainty using default for [cov,def,cov]: 1.000 GeV (covMatrix: xx=0.0064, yy=0, zz=0.0144)
[trace] Association: MC PDG=211 (E=5.234, p=[2.100,1.500,4.200]) <-> RC PDG=211 (E=5.180±0.150, p=[2.090±0.080,1.510±1.000,4.150±0.120])
[trace]   Energy penalty: 0.360 (χ²=0.130), Momentum penalty: 0.213 (χ²=0.045), PDG penalty: 0.000
```

Only `py` missing covariance → interesting pattern to investigate!

---

## ENABLING TRACE OUTPUT

### Method 1: Environment Variable

```bash
export JANA_LOG_LEVEL=trace
eicrecon sim.root -Ppodio:output_file=output.root 2>&1 | tee trace.log
```

### Method 2: Command Line Option

```bash
eicrecon sim.root -Ppodio:output_file=output.root -Pjana:log_level=trace 2>&1 | tee trace.log
```

### Method 3: Runtime Configuration

```bash
eicrecon sim.root \
  -Ppodio:output_file=output.root \
  -Plog:reco:level=trace \
  2>&1 | tee trace.log
```

---

## ANALYZING TRACE OUTPUT

### Quick Statistics Script

```python
#!/usr/bin/env python3
import sys
import re

with open(sys.argv[1], 'r') as f:
    lines = f.readlines()

# Count associations
total_assoc = len([l for l in lines if 'Association: MC PDG' in l])

# Count defaults
energy_defaults = len([l for l in lines if 'Energy uncertainty using default' in l])
momentum_defaults = len([l for l in lines if 'Momentum uncertainty using default' in l])

# Parse momentum default patterns
full_default = len([l for l in lines if '[def,def,def]' in l])
partial_default = momentum_defaults - full_default

print(f"Total associations: {total_assoc}")
print(f"Energy using defaults: {energy_defaults} ({100*energy_defaults/total_assoc:.1f}%)")
print(f"Momentum using defaults: {momentum_defaults} ({100*momentum_defaults/total_assoc:.1f}%)")
print(f"  - All components: {full_default}")
print(f"  - Partial: {partial_default}")
print(f"Particles with full covariance: {total_assoc - max(energy_defaults, momentum_defaults)}")
```

**Usage:**
```bash
python3 analyze_trace.py trace.log
```

**Example output:**
```
Total associations: 839
Energy using defaults: 0 (0.0%)
Momentum using defaults: 0 (0.0%)
  - All components: 0
  - Partial: 0
Particles with full covariance: 839
```

---

## INTERPRETATION

### All Covariance Present (Best Case)

**No trace messages about defaults**
- Reconstruction is filling covariance matrices properly
- Algorithm using actual uncertainties
- Chi-squared formulation working as intended

### Some Defaults Used (Investigation Needed)

**Messages appear for some particles**
- Check which detector regions lack covariance
- Check which particle types lack covariance
- May need to fix reconstruction to fill covariance
- Or tune default values appropriately

### All Defaults Used (Action Required)

**Every particle shows default messages**
- Reconstruction not filling covariance matrices at all
- Algorithm effectively using fixed uncertainties
- Should either:
  1. Fix reconstruction to provide covariance, OR
  2. Tune default resolutions to reasonable values

---

## RECOMMENDATIONS

### For Users

1. **Run with trace logging initially** to check covariance status
2. **If defaults are common**, tune them:
   ```bash
   -Preco:Truthiness:defaultEnergyResolution=0.5
   -Preco:Truthiness:defaultMomentumResolution=0.2
   ```
3. **If no defaults needed**, covariance is good → proceed normally

### For Developers

1. **Check trace output** when modifying reconstruction
2. **Ensure covariance matrices are filled** properly
3. **Verify uncertainties are reasonable** (not too large/small)
4. **Document expected covariance status** for each detector component

### For Analyzers

1. **Document covariance status** in analysis notes
2. **Include default resolution values** used
3. **Report what fraction used defaults** vs actual covariance
4. **Consider systematic uncertainties** from default choices

---

## BENEFITS

✅ **Transparency**: Clear indication when defaults are used
✅ **Diagnostics**: Easy to identify covariance filling issues
✅ **Validation**: Quick check of reconstruction quality
✅ **Debugging**: Helps track down why penalties might be unexpected
✅ **Optimization**: Guides tuning of default resolution values

---

## EXAMPLE USAGE WORKFLOW

```bash
# Step 1: Run with trace to check covariance status
export JANA_LOG_LEVEL=trace
eicrecon sim.root -Ppodio:output_file=test.root 2>&1 | tee trace.log

# Step 2: Check if defaults are being used
grep -c "using default" trace.log

# Step 3a: If no defaults → good! Use production settings
eicrecon sim.root -Ppodio:output_file=output.root

# Step 3b: If many defaults → tune default resolutions
eicrecon sim.root \
  -Preco:Truthiness:defaultEnergyResolution=0.5 \
  -Preco:Truthiness:defaultMomentumResolution=0.2 \
  -Ppodio:output_file=output.root
```

---

*Update: 2025-11-14*
*Addition: Diagnostic trace messages for default resolution usage*
*Impact: Better transparency and debugging capability*
