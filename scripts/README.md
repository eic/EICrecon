# Analysis Scripts

This directory contains analysis scripts for EICrecon reconstruction quality assessment.

## compare_reconstruction_truth.py

This script compares reconstructed particle properties against Monte Carlo truth for collections where truth-reconstruction associations exist. It calculates and prints quantitative comparisons in terms of reconstructed/truth ratios to assess reconstruction performance.

### Features

- Reads EDM4hep ROOT files produced by EICrecon
- Analyzes particle-level quantities (momentum, energy) using `ReconstructedParticleAssociations`
- Analyzes track-level quantities using `CentralCKFTrackAssociations`
- Supports both regular and truth-seeded reconstruction
- Provides statistical analysis including mean, median, RMS, and resolution metrics
- Filters outliers to provide meaningful statistics

### Usage

```bash
python compare_reconstruction_truth.py <input_files...> [--output output.txt]
```

### Example

```bash
# Analyze a single file
python compare_reconstruction_truth.py rec_e_1GeV_20GeV_craterlake.edm4eic.root

# Analyze multiple files with output to file
python compare_reconstruction_truth.py *.edm4eic.root --output analysis_results.txt
```

### Output

The script prints analysis results for each collection type:

1. **Reconstructed Particles**: Momentum and energy ratios
2. **Central CKF Tracks**: Track parameter ratios
3. **Truth-Seeded Tracks**: Performance with perfect seeding

For each quantity, the following statistics are reported:
- Mean ratio ± standard deviation
- Median ratio
- RMS
- Resolution (σ/μ)
- Number of matched particles/tracks
- 25th-75th percentile range

### Dependencies

- `uproot`: For reading ROOT files
- `awkward`: For handling jagged arrays
- `numpy`: For numerical calculations

### Integration with GitHub Actions

This script is automatically run in the EICrecon GitHub Actions workflow via the `reconstruction-quality-analysis` job, which:

1. Downloads reconstruction output files from `eicrecon-gun` and `eicrecon-dis` jobs
2. Runs the analysis on various detector configurations
3. Uploads the results as artifacts
4. Displays a summary in the workflow output

The analysis covers:
- Particle gun events (pions and electrons)
- Deep inelastic scattering events
- Multiple detector configurations (craterlake, craterlake_tracking_only, craterlake_18x275)
