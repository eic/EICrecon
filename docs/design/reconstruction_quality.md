# Reconstruction Quality Analysis

This document describes the automated reconstruction quality analysis that compares reconstructed particle quantities against Monte Carlo truth.

## Overview

The EICrecon GitHub Actions workflow includes automated quality assessment that:

1. **Analyzes reconstruction outputs** from both particle gun and deep inelastic scattering (DIS) events
2. **Compares reconstructed quantities to truth** using MC-reconstruction associations
3. **Calculates performance metrics** including accuracy, precision, and efficiency
4. **Provides quality assessments** to help identify reconstruction issues

## Implementation

### Analysis Script

The analysis is performed by `scripts/compare_reconstruction_truth.py`, which:

- Reads EDM4hep ROOT files produced by EICrecon
- Extracts MC truth and reconstructed particle information
- Uses association collections to match truth and reconstructed particles
- Calculates ratios (reconstructed/truth) for key quantities
- Provides statistical analysis and quality assessments

### GitHub Actions Integration

The `reconstruction-quality-analysis` job in the CI workflow:

- Depends on `eicrecon-gun` and `eicrecon-dis` jobs
- Downloads reconstruction artifacts from those jobs
- Runs analysis for multiple detector configurations
- Uploads results as artifacts for review
- Displays summary in the workflow output

### Analyzed Collections

The script analyzes the following data when associations are available:

1. **Reconstructed Particles**: 
   - Uses `MCParticles`, `ReconstructedParticles`, and `ReconstructedParticleAssociations`
   - Compares momentum components, total momentum, and energy

2. **Central Tracking**:
   - Uses `CentralCKFTracks` and `CentralCKFTrackAssociations`  
   - Compares track-level momentum measurements

3. **Truth-Seeded Tracking**:
   - Uses `CentralCKFTruthSeededTracks` and associations
   - Provides baseline performance with perfect seeding

## Metrics and Quality Assessment

### Key Metrics

For each quantity, the analysis reports:

- **Mean ratio**: Average of reconstructed/truth ratios
- **Resolution**: Standard deviation relative to mean (σ/μ)
- **Efficiency**: Fraction of MC particles successfully reconstructed
- **Percentile ranges**: Distribution characteristics

### Quality Grades

The analysis provides qualitative assessments:

- **EXCELLENT**: Mean ratio 0.9-1.1, resolution < 20%
- **GOOD**: Mean ratio 0.8-1.2, resolution < 30%  
- **FAIR**: Mean ratio 0.7-1.3, resolution < 50%
- **POOR**: Outside fair range

### Example Output

```
=== Reconstructed Particles Analysis ===
Total MC particles: 1500
Total reconstructed particles: 1342
Successfully matched: 1298
Reconstruction efficiency: 86.5%

Momentum:
  Mean ratio: 0.995 ± 0.087
  Median ratio: 0.998
  RMS: 1.002
  Resolution (σ/μ): 0.087
  N matched: 1298 / 1298 total
  25th-75th percentile: [0.942, 1.047]
  Quality assessment: EXCELLENT
```

## Usage

### Manual Analysis

To run the analysis manually:

```bash
# Analyze a single file
python scripts/compare_reconstruction_truth.py reconstruction_output.edm4eic.root

# Analyze multiple files with output to file
python scripts/compare_reconstruction_truth.py *.edm4eic.root --output results.txt
```

### Interpreting Results

- **Ratios near 1.0** indicate accurate reconstruction
- **Small resolution values** indicate precise/consistent reconstruction
- **High efficiency** indicates good particle-finding capability
- **Compare different detector configurations** to assess design choices
- **Compare gun vs DIS events** to understand performance in different environments

## Integration with CI/CD

The analysis runs automatically on:

- Pull requests (for changed code validation)
- Main branch pushes (for continuous monitoring)
- Scheduled runs (for regression detection)

Results are:

- Displayed in the GitHub Actions output
- Uploaded as downloadable artifacts
- Can be integrated into automated quality gates

This provides continuous monitoring of reconstruction performance and helps identify regressions or improvements in the codebase.