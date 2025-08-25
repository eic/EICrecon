# Audio Anomaly Detection Service

## Overview

The Audio Anomaly Detection Service provides auditory feedback for anomaly detection in subatomic physics data analysis within EICrecon. It creates an audio stream where each detector subsystem is mapped to a different frequency band, with loudness serving as a proxy for the amount of anomaly detected in that detector.

## Features

- **Multi-detector Support**: Monitors 17 different detector subsystems (BEMC, BHCAL, EEMC, EHCAL, FEMC, FHCAL, BTRK, ECTRK, BVTX, DRICH, PFRICH, DIRC, BTOF, ECTOF, ZDC, B0TRK, B0ECAL)
- **Frequency Mapping**: Each detector is assigned a unique frequency band between 200 Hz and 2000 Hz for optimal audibility
- **Real-time Processing**: Continuously processes events and updates audio output
- **Configurable Audio Device**: Supports specification of Linux audio device via parameters
- **Silent Mode**: Gracefully handles environments without audio hardware

## Architecture

The system consists of three main components:

1. **AudioAnomalyDetection_service**: Core service that manages audio output and frequency mapping
2. **AnomalyDetection algorithm**: Computes anomaly levels by comparing truth and reconstructed data
3. **AnomalyDetection_factory**: JANA factory that integrates the algorithm into the event processing pipeline

## Frequency Mapping

| Detector | Frequency (Hz) | Description |
|----------|----------------|-------------|
| BEMC     | 300           | Barrel Electromagnetic Calorimeter |
| BHCAL    | 400           | Barrel Hadronic Calorimeter |
| EEMC     | 500           | Endcap Electromagnetic Calorimeter |
| EHCAL    | 600           | Endcap Hadronic Calorimeter |
| FEMC     | 700           | Far Forward Electromagnetic Calorimeter |
| FHCAL    | 800           | Far Forward Hadronic Calorimeter |
| BTRK     | 900           | Barrel Tracker |
| ECTRK    | 1000          | Endcap Tracker |
| BVTX     | 1100          | Barrel Vertex Detector |
| DRICH    | 1200          | Dual Ring Imaging Cherenkov |
| PFRICH   | 1300          | Proximity Focusing RICH |
| DIRC     | 1400          | Detection of Internally Reflected Cherenkov |
| BTOF     | 1500          | Barrel Time of Flight |
| ECTOF    | 1600          | Endcap Time of Flight |
| ZDC      | 1700          | Zero Degree Calorimeter |
| B0TRK    | 1800          | B0 Tracker |
| B0ECAL   | 1900          | B0 Electromagnetic Calorimeter |

## Usage

### Loading the Plugin

To enable audio anomaly detection, add the plugin to your EICrecon execution:

```bash
eicrecon -Pplugins=audio_anomaly [other options] input_files
```

### Configuration Parameters

The service accepts the following parameters:

- `audio_anomaly:device` - Audio device name (default: "default")
- `audio_anomaly:sample_rate` - Sample rate for audio output in Hz (default: 44100)
- `audio_anomaly:buffer_size` - Audio buffer size in samples (default: 1024)

Example:
```bash
eicrecon -Pplugins=audio_anomaly -Paudio_anomaly:device=hw:0,0 input.edm4hep.root
```

### Algorithm Configuration

The anomaly detection algorithm can be configured via:

- `energy_threshold` - Energy threshold for considering particles in GeV (default: 0.1)
- `momentum_threshold` - Momentum threshold for considering particles in GeV/c (default: 0.1)
- `max_anomaly_value` - Maximum anomaly value for normalization (default: 10.0)
- `update_frequency` - Update frequency for audio output in events (default: 10)

## Implementation Details

### Anomaly Calculation

The service computes anomalies by comparing Monte Carlo truth information with reconstructed particle data:

1. **Energy-based Anomaly**: Compares total energy between truth and reconstructed particles
2. **Momentum-based Anomaly**: Compares total momentum magnitude between truth and reconstructed particles
3. **Combined Anomaly**: Weighted average of energy and momentum anomalies
4. **Normalization**: Maps raw anomaly values to [0,1] range for audio output

### Audio Generation

- Uses ALSA (Advanced Linux Sound Architecture) for low-latency audio output
- Generates sine waves at detector-specific frequencies
- Amplitude (loudness) is proportional to anomaly level
- Supports graceful degradation when audio hardware is unavailable

### Thread Safety

- Uses mutex protection for shared anomaly data
- Atomic flags for thread-safe control of audio generation loop
- Separate audio generation thread for real-time performance

## Dependencies

- **ALSA**: Advanced Linux Sound Architecture for audio output
- **JANA2**: Event processing framework
- **spdlog**: Logging framework
- **C++17**: Modern C++ features

## Limitations

- Currently requires Linux environment with ALSA support
- Audio hardware must be available for audible output (falls back to silent mode otherwise)
- Simplified anomaly calculation (can be enhanced with more sophisticated algorithms)
- Limited to mono audio output

## Future Enhancements

- Support for additional audio backends (PulseAudio, JACK)
- More sophisticated anomaly detection algorithms
- Stereo/multichannel audio for spatial representation
- Real-time parameter adjustment via web interface
- Integration with alert systems for critical anomalies