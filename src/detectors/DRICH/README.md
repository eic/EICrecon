# dRICH

## Algorithm and Data Flowchart
### Legend
```mermaid
flowchart LR
  classDef alg fill:#ff8888,color:black
  classDef col fill:#00aaaa,color:black
  Algorithm[<strong>Algorithm Description</strong><br/>Algorithm Name<br><i>Factory Name</i>]:::alg
  Collection(<strong>Collection Name</strong><br/>Collection Datatype):::col
  Algorithm --> Collection
```

### Flowchart
```mermaid
flowchart TB
  classDef alg fill:#ff8888,color:black
  classDef col fill:#00aaaa,color:black
  classDef op fill:#00aa00,color:black

  %%-----------------
  %% Nodes
  %%-----------------

  subgraph Inputs
    direction LR
    SimHits(<strong>DRICHHits</strong><br/>edm4hep::SimTrackerHit):::col
    Trajectories(<strong>CentralCKFTrajectories</strong><br/>eicrecon::TrackingResultTrajectory):::col
  end

  subgraph Reconstruction
    ParticleAlg[<strong>Particle Reconstruction</strong>]:::alg
  end

  subgraph Digitization
    DigiAlg[<strong>Digitization</strong><br/>PhotoMultiplierHitDigi<br><i>PhotoMultiplierHitDigi_factory</i>]:::alg
    RawHits(<strong>DRICHRawHitsAssociations</strong><br/>edm4eic::MCRecoTrackerHitAssociation):::col
  end

  subgraph Charged Particles
    PseudoTracksAlg[<strong>MC Cherenkov Photon Vertices</strong><br/>PseudoTracks<br><i>PseudoTrack_factory</i>]:::alg
    PseudoTracks(<strong>DRICHAerogelPseudoTracks</strong><br/><strong>DRICHGasPseudoTracks</strong><br/>edm4eic::TrackSegment):::col

    PropagatorAlg[<strong>Track Projection</strong><br/>TrackPropagation<br><i>RichTrack_factory</i>]:::alg
    Tracks(<strong>DRICHAerogelTracks</strong><br/><strong>DRICHGasTracks</strong><br/>edm4eic::TrackSegment):::col
    MirrorTracks(<strong>DRICHMirrorTracks - TODO</strong><br/>edm4eic::TrackSegment):::col

    TrackOR{OR}:::op

    ReflectionsAlg[<strong>Track Reflections - TODO</strong><br/>RichTrackReflections<br><i>RichTrackReflections_factory</i>]:::alg
    Reflections(<strong>DRICHTrackReflections - TODO</strong><br/>edm4eic::TrackSegment):::col
  end

  subgraph Particle Identification Algorithms
    IRT[<strong>IRT: Indirect Ray Tracing</strong><br/>IrtCherenkovParticleID<br><i>IrtCherenkovParticleID_factory</i>]:::alg
    IRTPID(<strong>DRICHIrtCherenkovParticleID</strong><br/>edm4eic::CherenkovParticleID):::col
    Final[<strong>Final PID</strong><br/>ParticleID<br><i>ParticleID_factory</i>]:::alg
  end

  subgraph User-level Outputs
    FinalPID(<strong>DRICHParticleID</strong><br/>edm4hep::ParticleID):::col
    ReconstructedParticles(<strong>ReconstructedParticles</strong><br/>edm4eic::ReconstructedParticle):::col
  end

  %%-----------------
  %% Edges
  %%-----------------

  %% digitization
  SimHits --> DigiAlg
  DigiAlg --> RawHits

  %% tracking
  SimHits --> PseudoTracksAlg
  PseudoTracksAlg --> PseudoTracks
  Trajectories --> PropagatorAlg
  PropagatorAlg --> Tracks
  PropagatorAlg --> MirrorTracks
  PseudoTracks --> TrackOR
  Tracks --> TrackOR
  MirrorTracks --> ReflectionsAlg
  ReflectionsAlg --> Reflections

  %% PID
  RawHits --> IRT
  TrackOR --> IRT
  Reflections --> IRT
  IRT --> IRTPID
  IRTPID --> Final
  Final --> FinalPID

  %% linking
  Trajectories --> ParticleAlg
  FinalPID --> ParticleAlg
  ParticleAlg --> ReconstructedParticles
```

## Datatypes, Relations, and Associations

### Detailed PID Datatype: `edm4eic::CherenkovParticleID`
```mermaid
flowchart LR
  classDef data fill:#aaaa00,color:black
  classDef comp fill:#ff8888,color:black

  %% nodes
  subgraph IrtCherenkovParticleID Output Collections
    direction TB
    CPIDAgl(<strong>DRICHIrtCherenkovParticleID</strong><br/>edm4eic::CherenkovParticleID):::data
    CPIDGas(<strong>DRICHIrtCherenkovParticleID</strong><br/>edm4eic::CherenkovParticleID):::data
    subgraph <strong>hypotheses</strong><br/>edm4eic::CherenkovParticleIDHypothesis
      direction TB
      HypAgl1([Pion])::comp
      HypAgl2([Kaon])::comp
      HypAgl3([...])::comp
    end
    subgraph <strong>hypotheses</strong><br/>edm4eic::CherenkovParticleIDHypothesis
      direction TB
      HypGas1([Pion])::comp
      HypGas2([Kaon])::comp
      HypGas3([...])::comp
    end
  end
  TrackAgl(<strong>DRICHAerogelTracks</strong><br/>edm4eic::TrackSegment):::data
  TrackGas(<strong>DRICHGasTracks</strong><br/>edm4eic::TrackSegment):::data

  %% edges
  CPIDAgl --> HypAgl1
  CPIDAgl --> HypAgl2
  CPIDAgl --> HypAgl3
  CPIDAgl --> TrackAgl
  CPIDGas --> HypGas1
  CPIDGas --> HypGas2
  CPIDGas --> HypGas3
  CPIDGas --> TrackGas
```

### User-level PID Output
- `DRICHAerogelTracks` and `DRICHGasTracks` are combined to `DRICHTracks`
- `DRICHIrtCherenkovParticleID::hypotheses` are combined from each radiator, and transformed to `edm4hep::ParticleID`
  objects named `DRICHParticleID`
- Then use (eta,phi) proximity matching to find the `ReconstructedParticle` that corresponds to the `DRICHTrack`, and
  link particle ID objects
```mermaid
flowchart LR
  classDef data fill:#aaaa00,color:black
  classDef comp fill:#ff8888,color:black
  classDef op fill:#00aa00,color:black

  %% nodes
  Track(<strong>DRICHTracks</strong><br/>edm4eic::TrackSegment):::data
  subgraph <strong>DRICHParticleID</strong>
    direction TB
    Hyp1(<strong>DRICHParticleID, PDG=pion</strong><br/>edm4hep::ParticleID):::data
    Hyp2(<strong>DRICHParticleID, PDG=kaon</strong><br/>edm4hep::ParticleID):::data
    Hyp3(<strong>DRICHParticleID, PDG=...</strong><br/>edm4hep::ParticleID):::data
  end
  Prox{{proximity match}}:::op
  Recon(<strong>ReconstructedParticles</strong><br/>edm4eic::ReconstructedParticle):::data

  %% edges
  Track --> Hyp1
  Track --> Hyp2
  Track --> Hyp3
  Track --> Prox
  Prox --> Recon
  Recon --> Hyp1
  Recon --> Hyp2
  Recon --> Hyp3
```
