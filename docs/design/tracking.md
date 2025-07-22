# Tracking

## Legend for workflow diagrams
- Blue boxes - data collection
- Orange boxes - factory / algorithm

## Conceptual diagram for track reconstruction
```mermaid
flowchart TB
  classDef alg fill:#f96;
  classDef col fill:#66ccff;

  SimHits(<strong>Simulation hits<br/> for tracking detectors</strong>):::col
  SimHits --> TrackDigi(<strong>Digitization</strong>):::alg
  TrackDigi --> RecHits(<strong>Digitized hits<br/> for tracking detectors</strong>):::col

  RecHits --> TrackSeeding(<strong> Track seeding):::alg
  TrackSeeding --> InitParams(<strong> Initial track parameters<br/> from seed triplet):::col

  RecHits --> CKFTracking(<strong> Combinatorial Kalman Filter<br/> for track finding and fitting):::alg
  InitParams --> CKFTracking
  CKFTracking --> ActsTracksUnsolved(<strong> Reconstructed tracks):::col

  ActsTracksUnsolved --> AmbiguitySolver(<strong> Track ambiguity resolution):::alg

  %% AmbiguitySolver --> ActsTracks(<strong> Final solved tracks):::col
  %% ActsTracks --> TypeConverter(<strong> Conversion of tracks<br/> to PODIO data type):::alg
  %% TypeConverter --> ReconstructedTracks(<strong> Reconstructed tracks<br/> in PODIO format):::col
  AmbiguitySolver --> ReconstructedTracks(<strong> Final Solved Tracks):::col

  ReconstructedTracks --> TrackProjector(<strong> Save predicted track states<br/> at each tracking layer):::alg
  TrackProjector --> TrackStates(<strong> Track parameters<br/> at each tracking layer):::col
  ReconstructedTracks --> TrackPropagator(<strong> Track propagation<br/> to calorimeters and PID detectors):::alg
  TrackPropagator --> PropagatedTracks(<strong> Projected track position and angle<br/> at calorimeters and PID detectors):::col
  ReconstructedTracks -->IterVertFind(<strong> Iterative Vertex finder<br/> and primary vertex fitter):::alg
  IterVertFind --> PrimVertices(<strong> Reconstructed primary vertex):::col
  ReconstructedTracks --> ParticleFactory(<strong> Copy reconstructed tracks<br/> to collection used for physics analyses):::alg
  ParticleFactory --> ReconstructedChargedParticles(<strong> Reconstructed charged particles<br/> used for physics analyses):::col

```

## Full diagram for track reconstruction

```mermaid
flowchart TB
  classDef alg fill:#f96;
  classDef col fill:#66ccff;
  subgraph Simulation output
    direction LR

    BVTXSimHits(<strong>Barrel Si vertex</strong>:<br/>VertexBarrelHits):::col
    BTRKSimHits(<strong>Barrel Si trk</strong>:<br/>SiBarrelHits):::col
    ECTRKSimHits(<strong>EndCap Si trk</strong>:<br/>TrackerEndcapHits):::col
    BMPGDSimHits(<strong>Barrel MPGD trk</strong>:<br/>MPGDBarrelHits):::col
    OBMPGDSimHits(<strong>Barrel MPGD Outer trk</strong>:<br/>OuterMPGDBarrelHits):::col
    ECNMPGDSimHits(<strong>Neg EndCap MPGD trk</strong>:<br/>BackwardMPGDEndcapHits):::col
    ECPMPGDSimHits(<strong>Pos EndCap MPGD trk</strong>:<br/>ForwardMPGDEndcapHits):::col
    BTOFSimHits(<strong>Barrel TOF</strong>:<br/>TOFBarrelHits):::col
    ECTOFSimHits(<strong>EndCap TOF</strong>:<br/>TOFEndcapHits):::col
  end

  subgraph Hit digitization
    BVTXSimHits --> TrackerDigi1[SiliconTrackerDigi]:::alg
    TrackerDigi1 --> VertexBarrelRawHits(SiBarrelVertexRawHits):::col
    VertexBarrelRawHits --> TrackerHitReconstruction1[TrackerHitReconstruction]:::alg
    TrackerHitReconstruction1 --> VertexBarrelRecHits(SiBarrelVertexRecHits):::col

    BTRKSimHits -->  TrackerDigi2[SiliconTrackerDigi]:::alg
    TrackerDigi2 --> TrackerBarrelRawHits(SiBarrelRawHits):::col
    TrackerBarrelRawHits --> TrackerHitReconstruction2[TrackerHitReconstruction]:::alg
    TrackerHitReconstruction2 --> TrackerBarrelRecHits(SiBarrelTrackerRecHits):::col

    ECTRKSimHits -->  TrackerDigi3[SiliconTrackerDigi]:::alg
    TrackerDigi3 --> TrackerEndcapRawHits(SiEndcapTrackerRawHits):::col
    TrackerEndcapRawHits --> TrackerHitReconstruction3[TrackerHitReconstruction]:::alg
    TrackerHitReconstruction3 --> TrackerEndcapRecHits(SiEndcapTrackerRecHits):::col

    BMPGDSimHits -->   MPGDTrackerBarrelDigi[SiliconTrackerDigi]:::alg
    MPGDTrackerBarrelDigi --> MPGDTrackerBarrelRawHits(MPGDBarrelRawHits):::col
    MPGDTrackerBarrelRawHits --> MPGDTrackerBarrelReconstruction[TrackerHitReconstruction]:::alg
    MPGDTrackerBarrelReconstruction --> MPGDTrackerBarrelRecHits(MPGDBarrelRecHits):::col

    OBMPGDSimHits -->   MPGDTrackerOutBarrelDigi[SiliconTrackerDigi]:::alg
    MPGDTrackerOutBarrelDigi --> MPGDTrackerOutBarrelRawHits(OuterMPGDBarrelRawHits):::col
    MPGDTrackerOutBarrelRawHits --> MPGDTrackerOutBarrelReconstruction[TrackerHitReconstruction]:::alg
    MPGDTrackerOutBarrelReconstruction --> MPGDTrackerOutBarrelRecHits(OuterMPGDBarrelRecHits):::col

    ECNMPGDSimHits -->   MPGDTrackerECNDigi[SiliconTrackerDigi]:::alg
    MPGDTrackerECNDigi --> MPGDTrackerECNRawHits(BackwardMPGDEndcapRawHits):::col
    MPGDTrackerECNRawHits --> MPGDTrackerECNReconstruction[TrackerHitReconstruction]:::alg
    MPGDTrackerECNReconstruction --> MPGDTrackerECNRecHits(BackwardMPGDEndcapRecHits):::col

    ECPMPGDSimHits -->   MPGDTrackerECPDigi[SiliconTrackerDigi]:::alg
    MPGDTrackerECPDigi --> MPGDTrackerECPRawHits(ForwardMPGDEndcapRawHits):::col
    MPGDTrackerECPRawHits --> MPGDTrackerECPReconstruction[TrackerHitReconstruction]:::alg
    MPGDTrackerECPReconstruction --> MPGDTrackerECPRecHits(ForwardMPGDEndcapRecHits):::col

    BTOFSimHits --> BTOFTrackerDigi[SiliconTrackerDigi]:::alg
    BTOFTrackerDigi --> BTOFRawHits(TOFBarrelRawHits):::col
    BTOFRawHits --> BTOFHitReconstruction[TrackerHitReconstruction]:::alg
    BTOFHitReconstruction --> BTOFRecHits(TOFBarrelRawHits):::col

    ECTOFSimHits --> ECTOFTrackerDigi[SiliconTrackerDigi]:::alg
    ECTOFTrackerDigi --> ECTOFRawHits(TOFEndcapRawHits):::col
    ECTOFRawHits --> ECTOFHitReconstruction[TrackerHitReconstruction]:::alg
    ECTOFHitReconstruction --> ECTOFRecHits(TOFEndcapRecHits):::col

    TrackerHitsCollector[CollectionCollector]:::alg

    VertexBarrelRecHits --> TrackerHitsCollector
    TrackerBarrelRecHits --> TrackerHitsCollector
    TrackerEndcapRecHits --> TrackerHitsCollector
    MPGDTrackerBarrelRecHits --> TrackerHitsCollector
    MPGDTrackerOutBarrelRecHits --> TrackerHitsCollector
    MPGDTrackerECNRecHits --> TrackerHitsCollector
    MPGDTrackerECPRecHits --> TrackerHitsCollector
    BTOFRecHits --> TrackerHitsCollector
    ECTOFRecHits --> TrackerHitsCollector

    TrackerHitsCollector --> CentralTrackingRecHits:::col
  end

  subgraph Track finding & fitting
    CentralTrackingRecHits --> TrackerHitsConverter[TrackerMeasurementFromHits]:::alg
    TrackerHitsConverter --> TrackerHitsOnSurface[CentralTrackerMeasurements]:::col

    TrackSeeding --> SeedParameters[CentralTrackSeedingResults]:::col
    CentralTrackingRecHits --> TrackSeeding[TrackSeeding]:::alg

    CKFTracking:::alg
    SeedParameters --> CKFTracking
    TrackerHitsOnSurface --> CKFTracking
    CKFTracking --> UnfilteredActsTracks[CentralCKFActsTracksUnfiltered]:::col

    AmbiguitySolver:::alg
    UnfilteredActsTracks --> AmbiguitySolver
    TrackerHitsOnSurface --> AmbiguitySolver
    AmbiguitySolver --> CentralCKFActsTracks:::col
    AmbiguitySolver --> CentralCKFActsTrajectories:::col

    ActsToTracks:::alg
    CentralCKFActsTrajectories --> ActsToTracks
    TrackerHitsOnSurface --> ActsToTracks
    ActsToTracks --> CentralCKFTracks:::col
    ActsToTracks --> CentralCKFTrajectories:::col
    ActsToTracks --> CentralCKFTrackParameters:::col
    ActsToTracks --> CentralCKFTrackAssociations:::col
  end

```

## Full diagram for track states, track projections, and primary vertexing
```mermaid
flowchart TB
  classDef alg fill:#f96;
  classDef col fill:#66ccff;

subgraph Reconstructed Tracking Info
  direction LR

  CentralCKFActsTrajectories(<strong>CentralCKFActsTrajectories):::col
  CentralCKFTracks(<strong>CentralCKFTracks):::col
  ReconstructedChargedParticles(<strong>ReconstructedChargedParticles):::col
  CentralCKFActsTracks(<strong>CentralCKFActsTracks):::col
end

subgraph Projections and Vertexing
    TrackProjector:::alg
    CentralCKFActsTrajectories --> TrackProjector
    CentralCKFTracks --> TrackProjector
    TrackProjector --> CentralTrackSegments:::col

    IterativeVertexFinder:::alg
    CentralCKFActsTrajectories --> IterativeVertexFinder
    ReconstructedChargedParticles --> IterativeVertexFinder
    IterativeVertexFinder --> CentralTrackVertices:::col

    TrackPropagation:::alg
    CentralCKFActsTracks --> TrackPropagation
    CentralCKFActsTrajectories --> TrackPropagation
    TrackPropagation --> CalorimeterTrackProjections:::col
  end

```

## Full diagram for idealized track reconstruction using truth seeding
```mermaid
flowchart TB
  classDef alg fill:#f96;
  classDef col fill:#66ccff;
  subgraph Simulation output
    direction LR

    BVTXSimHits(<strong>Barrel Si vertex</strong>:<br/>VertexBarrelHits):::col
    BTRKSimHits(<strong>Barrel Si trk</strong>:<br/>SiBarrelHits):::col
    ECTRKSimHits(<strong>EndCap Si trk</strong>:<br/>TrackerEndcapHits):::col
    BMPGDSimHits(<strong>Barrel MPGD trk</strong>:<br/>MPGDBarrelHits):::col
    OBMPGDSimHits(<strong>Barrel MPGD Outer trk</strong>:<br/>OuterMPGDBarrelHits):::col
    ECNMPGDSimHits(<strong>Neg EndCap MPGD trk</strong>:<br/>BackwardMPGDEndcapHits):::col
    ECPMPGDSimHits(<strong>Pos EndCap MPGD trk</strong>:<br/>ForwardMPGDEndcapHits):::col
    BTOFSimHits(<strong>Barrel TOF</strong>:<br/>TOFBarrelHits):::col
    ECTOFSimHits(<strong>EndCap TOF</strong>:<br/>TOFEndcapHits):::col
  end

  subgraph Hit digitization
    BVTXSimHits --> TrackerDigi1[SiliconTrackerDigi]:::alg
    TrackerDigi1 --> VertexBarrelRawHits(SiBarrelVertexRawHits):::col
    VertexBarrelRawHits --> TrackerHitReconstruction1[TrackerHitReconstruction]:::alg
    TrackerHitReconstruction1 --> VertexBarrelRecHits(SiBarrelVertexRecHits):::col

    BTRKSimHits -->  TrackerDigi2[SiliconTrackerDigi]:::alg
    TrackerDigi2 --> TrackerBarrelRawHits(SiBarrelRawHits):::col
    TrackerBarrelRawHits --> TrackerHitReconstruction2[TrackerHitReconstruction]:::alg
    TrackerHitReconstruction2 --> TrackerBarrelRecHits(SiBarrelTrackerRecHits):::col

    ECTRKSimHits -->  TrackerDigi3[SiliconTrackerDigi]:::alg
    TrackerDigi3 --> TrackerEndcapRawHits(SiEndcapTrackerRawHits):::col
    TrackerEndcapRawHits --> TrackerHitReconstruction3[TrackerHitReconstruction]:::alg
    TrackerHitReconstruction3 --> TrackerEndcapRecHits(SiEndcapTrackerRecHits):::col

    BMPGDSimHits -->   MPGDTrackerBarrelDigi[SiliconTrackerDigi]:::alg
    MPGDTrackerBarrelDigi --> MPGDTrackerBarrelRawHits(MPGDBarrelRawHits):::col
    MPGDTrackerBarrelRawHits --> MPGDTrackerBarrelReconstruction[TrackerHitReconstruction]:::alg
    MPGDTrackerBarrelReconstruction --> MPGDTrackerBarrelRecHits(MPGDBarrelRecHits):::col

    OBMPGDSimHits -->   MPGDTrackerOutBarrelDigi[SiliconTrackerDigi]:::alg
    MPGDTrackerOutBarrelDigi --> MPGDTrackerOutBarrelRawHits(OuterMPGDBarrelRawHits):::col
    MPGDTrackerOutBarrelRawHits --> MPGDTrackerOutBarrelReconstruction[TrackerHitReconstruction]:::alg
    MPGDTrackerOutBarrelReconstruction --> MPGDTrackerOutBarrelRecHits(OuterMPGDBarrelRecHits):::col

    ECNMPGDSimHits -->   MPGDTrackerECNDigi[SiliconTrackerDigi]:::alg
    MPGDTrackerECNDigi --> MPGDTrackerECNRawHits(BackwardMPGDEndcapRawHits):::col
    MPGDTrackerECNRawHits --> MPGDTrackerECNReconstruction[TrackerHitReconstruction]:::alg
    MPGDTrackerECNReconstruction --> MPGDTrackerECNRecHits(BackwardMPGDEndcapRecHits):::col

    ECPMPGDSimHits -->   MPGDTrackerECPDigi[SiliconTrackerDigi]:::alg
    MPGDTrackerECPDigi --> MPGDTrackerECPRawHits(ForwardMPGDEndcapRawHits):::col
    MPGDTrackerECPRawHits --> MPGDTrackerECPReconstruction[TrackerHitReconstruction]:::alg
    MPGDTrackerECPReconstruction --> MPGDTrackerECPRecHits(ForwardMPGDEndcapRecHits):::col

    BTOFSimHits --> BTOFTrackerDigi[SiliconTrackerDigi]:::alg
    BTOFTrackerDigi --> BTOFRawHits(TOFBarrelRawHits):::col
    BTOFRawHits --> BTOFHitReconstruction[TrackerHitReconstruction]:::alg
    BTOFHitReconstruction --> BTOFRecHits(TOFBarrelRawHits):::col

    ECTOFSimHits --> ECTOFTrackerDigi[SiliconTrackerDigi]:::alg
    ECTOFTrackerDigi --> ECTOFRawHits(TOFEndcapRawHits):::col
    ECTOFRawHits --> ECTOFHitReconstruction[TrackerHitReconstruction]:::alg
    ECTOFHitReconstruction --> ECTOFRecHits(TOFEndcapRecHits):::col

    TrackerHitsCollector[CollectionCollector]:::alg

    VertexBarrelRecHits --> TrackerHitsCollector
    TrackerBarrelRecHits --> TrackerHitsCollector
    TrackerEndcapRecHits --> TrackerHitsCollector
    MPGDTrackerBarrelRecHits --> TrackerHitsCollector
    MPGDTrackerOutBarrelRecHits --> TrackerHitsCollector
    MPGDTrackerECNRecHits --> TrackerHitsCollector
    MPGDTrackerECPRecHits --> TrackerHitsCollector
    BTOFRecHits --> TrackerHitsCollector
    ECTOFRecHits --> TrackerHitsCollector

    TrackerHitsCollector --> CentralTrackingRecHits:::col
  end

  subgraph Simulation output
    MCParticles:::col
  end

  subgraph Track finding & fitting
    CentralTrackingRecHits ---> TrackerHitsConverter[TrackerMeasurementFromHits]:::alg
    TrackerHitsConverter ---> TrackerHitsOnSurface[CentralTrackerMeasurements]:::col

    MCParticles --> TrackParamTruthInit:::alg
    TrackParamTruthInit --> TrackTruthSeeds:::col

    TrackTruthSeeds --> SubDivideCollection:::alg
    SubDivideCollection --> CentralTrackerTruthSeeds:::col

    CKFTracking:::alg
    TrackerHitsOnSurface --> CKFTracking
    CentralTrackerTruthSeeds --> CKFTracking
    CKFTracking --> UnfilteredActsTracks[CentralCKFTruthSeededActsTracksUnfiltered]:::col

    AmbiguitySolver:::alg
    UnfilteredActsTracks --> AmbiguitySolver
    TrackerHitsOnSurface --> AmbiguitySolver
    AmbiguitySolver --> CentralCKFActsTrajectories[CentralTruthSeededCKFActsTrajectories]:::col
    AmbiguitySolver --> CentralCKFActsTracks[CentralTruthSeededCKFActsTracks]:::col

    ActsToTracks:::alg
    CentralCKFActsTrajectories --> ActsToTracks
    TrackerHitsOnSurface --> ActsToTracks
    ActsToTracks --> CentralCKFTruthSeededTracks:::col
    ActsToTracks --> CentralCKFTruthSeededTrajectories:::col
    ActsToTracks --> CentralCKFTruthSeededTrackParameters:::col
    ActsToTracks --> CentralCKFTruthSeededTrackAssociations:::col

  end

```

## Full diagram for B0 track reconstruction
In progress...

## Description of collections
- MCParticles ([edm4hep::MCParticle](https://github.com/key4hep/EDM4hep/blob/v00-99-02/edm4hep.yaml#L230-L258)) -- Monte Carlo particle
- SimTrackerHits ([edmhep::SimTrackerHit](https://github.com/key4hep/EDM4hep/blob/v00-99-02/edm4hep.yaml#L296-L333)) -- Simulated tracker hit (e.g. VertexBarrelHits)
- RawTrackerHits ([edm4eic::RawTrackerHit](https://github.com/eic/EDM4eic/blob/v8.2.0/edm4eic.yaml#L372-L379)) -- Raw (digitized) tracker hit (e.g. SiBarrelVertexRawHits)
- TrackerHits ([edm4eic::TrackerHit](https://github.com/eic/EDM4eic/blob/v8.2.0/edm4eic.yaml#L381-L393)) -- Tracker hit reconstructed from RawTrackerHit (e.g. SiBarrelVertexRecHits, CentralTrackingRecHits)
- CentralTrackerMeasurements ([edm4eic::Measurement2D](https://github.com/eic/EDM4eic/blob/v8.2.0/edm4eic.yaml#L395-L406)) -- 2D measurement (on an arbitrary surface)
- CentralTrackSeedingResults ([edm4eic::TrackSeed](https://github.com/eic/EDM4eic/blob/v8.2.0/edm4eic.yaml#L408-L416)) -- Track seed information
- CentralCKFTracks ([edm4eic::Track](https://github.com/eic/EDM4eic/blob/v8.2.0/edm4eic.yaml#L453-L471)) -- Reconstructed track information and link to trajectory
- CentralCKFTrajectories ([edm4eic::Trajectory](https://github.com/eic/EDM4eic/blob/v8.2.0/edm4eic.yaml#L453-L471)) -- Track quality information and link to track parameters
- CentralCKFTrackParameters ([edm4eic::TrackParameters](https://github.com/eic/EDM4eic/blob/v8.2.0/edm4eic.yaml#L438-L450)] -- Reconstructed track parameters
- CentralCKFTrackAssociations ([edm4eic::MCRecoTrackParticleAssociation](https://github.com/eic/EDM4eic/blob/v8.2.0/edm4eic.yaml#L555-L564)) -- Hit-based association between reconstructed track and MCParticle
- ReconstructedChargedParticles ([edm4eic::ReconstructedParticle](https://github.com/eic/EDM4eic/blob/v8.2.0/edm4eic.yaml#L231-L263)) -- Reconstructed particle based on reconstructed track
- ReconstructedChargedParticleAssociations ([edm4eic::MCRecoParticleAssociation](https://github.com/eic/EDM4eic/blob/v8.2.0/edm4eic.yaml#L533-L542)) -- Copy of CentralCKFTrackAssociations to associate ReconstructedChargedParticle and MCParticle
- CentralTrackSegments ([edm4eic::TrackSegment](https://github.com/eic/EDM4eic/blob/v8.2.0/edm4eic.yaml#L473-L482)) -- Track segment with link to track information at various tracking detector layers ([edm4eic::TrackPoint](https://github.com/eic/EDM4eic/blob/v8.2.0/edm4eic.yaml#L159-L174))
- CentralTrackVertices ([edm4eic::Vertex](https://github.com/eic/EDM4eic/blob/v8.2.0/edm4eic.yaml#L159-L174)) -- Reconstructed primary vertex (vertices)
- CalorimeterTrackProjections ([edm4eic::TrackSegment](https://github.com/eic/EDM4eic/blob/v8.2.0/edm4eic.yaml#L473-L482)) -- Track projection at calorimeter detectors

## Acts information
- [Instructions for creating Acts Material Map](https://github.com/eic/epic/tree/main/scripts/material_map#material-map-for-acts)
- [Acts flags](flags/acts.md ':include')
