# dRICH

## Algorithms and Data Flowchart
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

## Data Model

### Expert-level PID Output
- RICH-specific particle ID datatype `edm4eic::CherenkovParticleID`
  - Vector components `edm4eic::CherenkovParticleIDHypothesis`, one for each PID hypothesis
  - 1-1 relation to corresponding charged particle `edm4eic::TrackSegment`
- Aerogel and Gas results are combined to one collection: `DRICHIrtCherenkovParticleID`
```mermaid
flowchart LR
  classDef col fill:#00aaaa,color:black
  classDef comp fill:#8888ff,color:black

  %% nodes
  TrackAgl(<strong>DRICHAerogelTracks</strong><br/>edm4eic::TrackSegment):::col
  TrackGas(<strong>DRICHGasTracks</strong><br/>edm4eic::TrackSegment):::col
  subgraph DRICHIrtCherenkovParticleID
    direction TB
    CPIDAgl(<strong>DRICHIrtCherenkovParticleID</strong><br/>edm4eic::CherenkovParticleID):::col
    CPIDGas(<strong>DRICHIrtCherenkovParticleID</strong><br/>edm4eic::CherenkovParticleID):::col
  end
  subgraph <strong>hypotheses, from aerogel</strong><br/>edm4eic::CherenkovParticleIDHypothesis
    direction TB
    HypAgl0([Electron Hypothesis]):::comp
    HypAgl1([Pion Hypothesis]):::comp
    HypAgl2([Kaon Hypothesis]):::comp
    HypAgl3([Proton Hypothesis]):::comp
  end
  subgraph <strong>hypotheses, from gas</strong><br/>edm4eic::CherenkovParticleIDHypothesis
    direction TB
    HypGas0([Electron Hypothesis]):::comp
    HypGas1([Pion Hypothesis]):::comp
    HypGas2([Kaon Hypothesis]):::comp
    HypGas3([Proton Hypothesis]):::comp
  end
  

  %% edges
  CPIDAgl --> HypAgl0
  CPIDAgl --> HypAgl1
  CPIDAgl --> HypAgl2
  CPIDAgl --> HypAgl3
  CPIDAgl --> TrackAgl
  CPIDGas --> HypGas0
  CPIDGas --> HypGas1
  CPIDGas --> HypGas2
  CPIDGas --> HypGas3
  CPIDGas --> TrackGas
```

### User-level PID Output
- `DRICHAerogelTracks` and `DRICHGasTracks` are combined to `DRICHTracks`
- `DRICHIrtCherenkovParticleID::hypotheses` are combined from each radiator, and transformed to `edm4hep::ParticleID`
  objects named `DRICHParticleID`
- Use 1-N relation `edm4eic::TrackSegment::particleIDs` to link `DRICHTrack` to `DRICHParticleIDs`
- Then use (eta,phi) proximity matching to find the `ReconstructedParticle` that corresponds to the `DRICHTrack`, and
  link particle ID objects
  - Use 1-1 relation `ReconstructedParticle::particleIDUsed` to specifiy the most-likely `edm4hep::ParticleID` object, and
    set `ReconstructedParticle::PDG` accordingly
  - Use 1-N relation `ReconstructedParticle::particleIDs` to link all the `edm4hep::ParticleID` objects
```mermaid
flowchart TB
  classDef col fill:#00aaaa,color:black
  classDef alg fill:#ff8888,color:black
  classDef comp fill:#8888ff,color:black

  %% nodes
  Track(<strong>DRICHTracks</strong><br/>edm4eic::TrackSegment):::col
  Prox[proximity matching]:::alg
  Recon(<strong>ReconstructedParticles</strong><br/>edm4eic::ReconstructedParticle):::col
  subgraph <strong>DRICHParticleID</strong>
    direction LR
    Hyp0(<strong>DRICHParticleID</strong><br/>edm4hep::ParticleID):::col --> Pdg0([PDG=electron]):::comp
    Hyp1(<strong>DRICHParticleID</strong><br/>edm4hep::ParticleID):::col --> Pdg1([PDG=pion]):::comp
    Hyp2(<strong>DRICHParticleID</strong><br/>edm4hep::ParticleID):::col --> Pdg2([PDG=kaon]):::comp
    Hyp3(<strong>DRICHParticleID</strong><br/>edm4hep::ParticleID):::col --> Pdg3([PDG=proton]):::comp
  end

  %% edges
  Track --> Prox
  Prox --> Recon
  Track --> Hyp0
  Track --> Hyp1
  Track --> Hyp2
  Track --> Hyp3
  Recon --> Hyp0
  Recon --> Hyp1
  Recon -- particleIDUsed --> Hyp1
  Recon --> Hyp2
  Recon --> Hyp3
  ```
