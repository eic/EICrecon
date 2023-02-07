# Tracking

```mermaid
flowchart TB
  classDef alg fill:#44cc;
  classDef col fill:#cc66ff;
  subgraph Simulation output
    direction LR
    SimHits(<strong>Simulation hits for detectors</strong>:<br/>edm4hep::SimTrackerHit)
    MCParticles(<strong>MC particles</strong>:<br/>edm4hep::MCParticle)
  end

  SimHits --> HitsReco[<strong>Per detector hits processing</strong>:<br/><i>SiliconTrackerDigi</i><br><i>TrackerHitReconstruction</i>]:::alg
  HitsReco --> Hits(Hits prepared for tracking)

  Hits --> CKFTracking[<strong>ACTS CFK Tracking</strong>:<br/><i>TrackSourceLinker</i><br><i>TrackParamTruthInit</i><br><i>CFKTracking</i>]:::alg
  MCParticles --> CKFTracking

  CKFTracking --> ACTSOutput(ACTS output)

  ACTSOutput --> ACTSToModel[<strong>Convert ACTS to data model</strong>:<br/><i>ParticlesFromTrackFit</i><br><i>TrackProjector</i><br><i>TrackPropagator</i>]:::alg

  ACTSToModel --> TrackingModel(Tracking PODIO data)

  TrackingModel --> ParticlesWithTruthPID:::alg
  MCParticles --> ParticlesWithTruthPID[<strong>Track to MC matching</strong>:<br/><i>ParticlesWithTruthPID</i>]
  ACTSToModel --> CentralTrackSegments

  ParticlesWithTruthPID --> ReconstructedChargedParticles
  ParticlesWithTruthPID --> ReconstructedChargedParticlesAssociations

  subgraph Tracking output
    direction LR
    CentralTrackSegments(<strong>CentralTrackSegments</strong><br/><i>edm4eic::TrackSegments</i>)
    ReconstructedChargedParticles(<strong>ReconstructedChargedParticles</strong><br/><i>edm4eic::ReconstructedParticle</i>)
    ReconstructedChargedParticlesAssociations(<strong>ReconstructedChargedParticlesAssociations</strong><br/><i>edm4eic::ReconstructedParticleAssociation</i>)
  end
```

Simplified tracking data flows and algorithms diagram. Full diagram is described below.

### Tracking related flags:

[ACTS flags](flags/acts.md ':include')

### Reconstructed particles chart


## Full diagram


```mermaid
flowchart TB
  classDef alg fill:#f96;
  classDef col fill:#66ccff;
  subgraph Simulation output
    direction LR

    BTRKSimHits(<strong>Barrel trk</strong>:<br/>SiBarrelHits)
    BTOFSimHits(<strong>Barrel TOF</strong>:<br/>TOFBarrelHits)
    BVTXSimHits(<strong>Barrel vertex</strong>:<br/>VertexBarrelHits)
    ECTRKSimHits(<strong>EndCap trk</strong>:<br/>TrackerEndcapHits)
    MPGDSimHits(<strong>MPGD barrel trk</strong>:<br/>MPGDBarrelHits)
    ECTOFSimHits(<strong>EndCap TOF</strong>:<br/>MPGDBarrelHits)
  end

  BTOFSimHits --> BTOFTrackerDigi[TrackerDigi]:::alg
  BTOFTrackerDigi --> BTOFRawHits(TOFBarrelRawHit)
  BTOFRawHits --> BTOFHitReconstruction[HitReconstruction]:::alg
  BTOFHitReconstruction --> BTOFRecHits(TOFBarrelTrackerHit)

  ECTOFSimHits --> ECTOFTrackerDigi[TrackerDigi]:::alg
  ECTOFTrackerDigi --> ECTOFRawHits(TOFEndcapRawHit)
  ECTOFRawHits --> ECTOFHitReconstruction[HitReconstruction]:::alg
  ECTOFHitReconstruction --> ECTOFRecHits(TOFEndcapTrackerHit)

  ECTRKSimHits -->  TrackerDigi2[TrackerDigi]:::alg
  TrackerDigi2 --> TrackerEndcapRawHits(TrackerEndcapRawHits)
  TrackerEndcapRawHits --> TrackerHitReconstruction2[HitReconstruction]:::alg
  TrackerHitReconstruction2 --> TrackerEndcapRecHits(EndcapTrackerHit)


  BTRKSimHits -->  TrackerDigi[TrackerDigi]:::alg
  TrackerDigi --> TrackerBarrelRawHits(TrackerBarrelRawHits)
  TrackerBarrelRawHits --> TrackerHitReconstruction[HitReconstruction]:::alg
  TrackerHitReconstruction --> TrackerBarrelRecHits(BarrelTrackerHit)

  MPGDSimHits -->   TrackerDigi4[TrackerDigi]:::alg
  TrackerDigi4 --> MPGDTrackerBarrelRawHits(MPGDTrackerRawHit)
  MPGDTrackerBarrelRawHits --> TrackerHitReconstruction4[HitReconstruction]:::alg
  TrackerHitReconstruction4 --> MPGDTrackerBarrelRecHits(MPGDTrackerHit)


  BVTXSimHits --> TrackerDigi3[TrackerDigi]:::alg
  TrackerDigi3 --> VertexBarrelRawHits(VertexBarrelRawHits)
  VertexBarrelRawHits --> TrackerHitReconstruction3[HitReconstruction]:::alg
  TrackerHitReconstruction3 --> VertexBarrelRecHits(VertexBarrelRecHits)

  TrackerHitsCollector[TrackerHitsCollector]:::col

  TrackerBarrelRecHits --> TrackerHitsCollector
  TrackerEndcapRecHits --> TrackerHitsCollector
  VertexBarrelRecHits --> TrackerHitsCollector
  MPGDTrackerBarrelRecHits --> TrackerHitsCollector
  ECTOFRecHits --> TrackerHitsCollector
  BTOFRecHits --> TrackerHitsCollector


  TrackerHitsCollector --> trackerHits
  trackerHits --> TrackerSourceLinker[TrackerSourceLinker]:::alg

  TrackerSourceLinker --> TrackSourceLinks(TrackSourceLinks)
  TrackerSourceLinker --> TrackMeasurements(TrackMeasurements)

  subgraph Sim output
    MCParticles(MCParticles)
  end

  MCParticles --> TrackParamTruthInit[TrackParamTruthInit]:::alg
  TrackParamTruthInit --> InitTrackParams

  TrackSourceLinks --> CKFTracking[CKFTracking]:::alg
  TrackMeasurements --> CKFTracking
  InitTrackParams --> CKFTracking
  CKFTracking --> CentralCKFTrajectories

  CentralCKFTrajectories --> ParticlesFromTrackFit[ParticlesFromTrackFit]:::alg
  ParticlesFromTrackFit --> outputTrackParameters
  ParticlesFromTrackFit --> outputParticles


      CentralCKFTrajectories --> TrackProjector[TrackProjector]:::alg
  TrackProjector --> CentralTrackSegments

  outputTrackParameters --> ParticlesWithTruthPID[ParticlesWithTruthPID]:::alg
  MCParticles --> ParticlesWithTruthPID
  ParticlesWithTruthPID --> ReconstructedChargedParticles
  ParticlesWithTruthPID --> ReconstructedChargedParticlesAssociations



  subgraph Tracking output
    direction LR
    CentralTrackSegments
    ReconstructedChargedParticles
    ReconstructedChargedParticlesAssociations

  end

```

This diagram illustrates data transformation and algorithms corresponding to
tracking part.

What is on the graph:

- Orange boxes - is an underlying algorithm
- BlueBoxes(TrackerHitsCollector) - simple factory that gets all hits together
- Boxes with rounded corners - data collection names

The flow is:

- Each detector hits first goes to **SiliconTrackerDigi** algorithm. Digitized tracking data has only geometry cell ID and timing data.
- Then each digitized hit gets into **HitReconstruction**. Geometry is found by ID, position, time and covariance extracted.
- Reconstructed hits from all detectors get to **TrackerSourceLinker** which provides measurement and linkage data for ACTS
- **CFKTracking** does fitting and produces results in ACTS classes
- **ParticlesFromTrackFit** process ACTS data and store it to PODIO edm4hep/eic data model
- **ParticlesWithTruthPID** algorithm does track-matching with MCParticles and produce resulted `edm4eic::ReconstructedParticles` with association class
- **TrackProjection** - saves track states/points data to PODIO data model and returns CetntralTrackSegments data
