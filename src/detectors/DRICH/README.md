# dRICH

## Algorithms and Data Flowchart

The following is a flowchart of algorithms and collections corresponding to **P**article **Id**entification (PID)
from the **D**ual **R**ing **I**maging **Ch**erenkov (dRICH) detector.

### Legend
```mermaid
flowchart TB
  classDef alg fill:#ff8888,color:black
  classDef col fill:#00aaaa,color:black
  Algorithm[<strong>Algorithm Description</strong><br/>Algorithm Name<br/><i>Factory Name</i>]:::alg
  InputCollection(<strong>Input Collection Name</strong><br/>Input Collection Datatype):::col
  OutputCollection(<strong>Output Collection Name</strong><br/>Output Collection Datatype):::col
  InputCollection ==> Algorithm ==> OutputCollection
```

### Flowchart
```mermaid
flowchart TB
  classDef alg fill:#ff8888,color:black
  classDef col fill:#00aaaa,color:black
  classDef misc fill:#ff88ff,color:black

  %%-----------------
  %% Nodes
  %%-----------------

  subgraph Inputs
    direction LR
    SimHits(<strong>DRICHHits</strong><br/>MC dRICH photon hits<br/>edm4hep::SimTrackerHit):::col
    Trajectories(<strong>CentralCKFTrajectories</strong><br/>ActsExamples::Trajectories):::col
    MCParts(<strong>MCParticles</strong><br/>MC True Particles<br/>edm4hep::MCParticles):::col
  end

  subgraph Digitization
    DigiAlg[<strong>Digitization</strong><br/>PhotoMultiplierHitDigi<br/><i>PhotoMultiplierHitDigi_factory</i>]:::alg
    RawHits(<strong>DRICHRawHits</strong><br/>includes noise<br/>edm4eic::RawTrackerHit):::col
    HitAssocs(<strong>DRICHRawHitsAssociations</strong><br/>no noise<br/>edm4eic::MCRecoTrackerHitAssociation):::col
  end

  subgraph Charged Particles and Tracking
    PropagatorAlg[<strong>Track Projection</strong><br/>TrackPropagation<br/><i>RichTrack_factory</i>]:::alg
    subgraph rad1 [radiators]
      AerogelTracks(<strong>DRICHAerogelTracks</strong><br/>edm4eic::TrackSegment):::col
      GasTracks(<strong>DRICHGasTracks</strong><br/>edm4eic::TrackSegment):::col
    end
    MergeTracksAlg[<strong>Merge Tracks</strong><br/>MergeTrackSegments<br/><i>MergeTrack_factory</i>]:::alg
    MergedTracks(<strong>DRICHMergedTracks</strong><br/>edm4eic::TrackSegment):::col

    %%MirrorTracks(<strong>DRICHMirrorTracks - TODO</strong><br/>edm4eic::TrackSegment):::col
    %%ReflectionsAlg[<strong>Track Reflections - TODO</strong><br/>RichTrackReflection<br/><i>RichTrackReflection_factory</i>]:::alg
    %%Reflections(<strong>DRICHTrackReflections - TODO</strong><br/>edm4eic::TrackSegment):::col

    TrackingAlgos[(<strong>Tracking Algorithms</strong>)]:::alg
    TrackParameters(<strong>outputTrackParameters</strong><br/>edm4eic::TrackParameters):::col
  end

  PIDInputs{{<strong>PID Algorithm Inputs</strong>}}:::misc

  subgraph Particle Identification Algorithms
    IRT[<strong>IRT: Indirect Ray Tracing</strong><br/>IrtCherenkovParticleID<br/><i>IrtCherenkovParticleID_factory</i>]:::alg
    subgraph rad2 [radiators]
      IRTPIDAerogel(<strong>DRICHAerogelIrtCherenkovParticleID</strong><br/>edm4eic::CherenkovParticleID):::col
      IRTPIDGas(<strong>DRICHGasIrtCherenkovParticleID</strong><br/>edm4eic::CherenkovParticleID):::col
    end

    AlternatePIDAlgo[<strong>Alternate PID Algorithm</strong><br/>TODO]:::alg
    AlternatePID(<strong>Alternate PID Output Collection<br/>TODO</strong><br/>edm4eic::CherenkovParticleID):::col

    MergePID[<strong>Combine PID from radiators</strong><br/>MergeParticleID<br/><i>MergeCherenkovParticleID_factory</i>]:::alg
    IRTPIDMerged(<strong>DRICHIrtMergedCherenkovParticleID</strong><br/>edm4eic::CherenkovParticleID):::col
  end

  PIDOutputs{{<strong>PID Algorithm Outputs</strong>}}:::misc

  subgraph Particle Identification Linking
    ProxMatch[<strong>Proximity Matching</strong><br/>ParticlesWithPID<br/><i>ParticlesWithPID_factory</i>]:::alg

    ReconParts(<strong>ReconstructedChargedParticles</strong><br/>edm4eic::ReconstructedParticle):::col
    ReconAssocs(<strong>ReconstructedChargedParticleAssociations</strong><br/>edm4eic::MCRecoParticleAssociation):::col
    ReconPIDs(<strong>ReconstructedChargedParticleIDs</strong><br/>edm4hep::ParticleID):::col
  end


  %%-----------------
  %% Edges
  %%-----------------

  %% digitization
  SimHits ==> DigiAlg
  DigiAlg ==> RawHits
  DigiAlg ==> HitAssocs
  %%SimHits -.association.- HitAssocs
  %%SimHits -.association.- MCParts

  %% tracking
  Trajectories   ==> PropagatorAlg
  PropagatorAlg  ==> AerogelTracks ==> MergeTracksAlg
  PropagatorAlg  ==> GasTracks     ==> MergeTracksAlg
  MergeTracksAlg ==> MergedTracks

  %% track reflections
  %% PropagatorAlg  ==> MirrorTracks
  %% MirrorTracks   ==> ReflectionsAlg
  %% ReflectionsAlg ==> Reflections
  %% Reflections    ==> PIDInputs

  %% PID
  RawHits       ==> PIDInputs
  HitAssocs     ==> PIDInputs
  AerogelTracks ==> PIDInputs
  GasTracks     ==> PIDInputs
  MergedTracks  ==> PIDInputs
  PIDInputs     ==> IRT
  IRT           ==> IRTPIDAerogel    ==> MergePID
  IRT           ==> IRTPIDGas        ==> MergePID
  MergePID      ==> IRTPIDMerged     ==> PIDOutputs
  PIDInputs     ==> AlternatePIDAlgo ==> AlternatePID ==> PIDOutputs

  %% linking
  Trajectories ==> TrackingAlgos ==> TrackParameters
  PIDOutputs      ==> ProxMatch
  TrackParameters ==> ProxMatch
  MCParts         ==> ProxMatch
  ProxMatch ===> ReconParts
  ProxMatch ==> ReconPIDs
  ProxMatch ==> ReconAssocs
  ReconParts -.1 to N.-> ReconPIDs
  %%ReconAssocs -.association.- MCParts
```

## Data Model

### Digitized Hits
- All digitized hits, including noise hits, are stored as `edm4eic::RawTrackerHit` collections
- Association `edm4eic::MCRecoTrackerHitAssociation` stores the 1-N link from a digitized hit to the MC truth hits
  - each MC truth hit has a 1-1 relation to the original MC `opticalphoton` (or whatever particle caused the hit)
  - digitized noise hits will not have associated MC truth hits, and therefore do not appear in `edm4eic::MCRecoTrackerHitAssociation` collections
```mermaid
flowchart TB
  classDef col fill:#00aaaa,color:black
  classDef fn fill:#c3b091,color:black

  %% nodes
  Association(<strong>DRICHRawHitsAssociation</strong><br/>edm4eic::MCRecoTrackerHitAssociation):::col
  RawHitFn[rawHit]:::fn
  subgraph Digitized
    RawHit(edm4eic::RawTrackerHit):::col
  end
  SimHitsFn[simHits]:::fn
  subgraph Simulated MC Truth
    direction TB
    SimHit1(edm4hep::SimTrackerHit):::col
    SimHit2(edm4hep::SimTrackerHit):::col
    SimHit3(... additional MC hits and corresponding photons ...):::col
    MCParticleFn1[MCParticle]:::fn
    MCParticleFn2[MCParticle]:::fn
    Photon1(edm4hep::MCParticle):::col
    Photon2(edm4hep::MCParticle):::col
  end

  %% edges
  Association ==> RawHitFn ==> RawHit
  Association ==> SimHitsFn
  SimHitsFn ==> SimHit1 ==> MCParticleFn1 ==> Photon1
  SimHitsFn ==> SimHit2 ==> MCParticleFn2 ==> Photon2
  SimHitsFn ==> SimHit3
```

### Expert-level PID Output
- RICH-specific particle ID datatype `edm4eic::CherenkovParticleID`
  - From IRT: aerogel and gas results separated
  - Vector member of `edm4eic::CherenkovParticleIDHypothesis` components, one for each PID hypothesis, with members:
    - PDG
    - NPE
    - weight
  - Additional members:
    - Reconstructed Cherenkov (theta, phi) for each photon
    - NPE
    - MC: average photon energy and refractive index at emission point
    - Link to charged particle and hit-associations
  - 1-1 relation to corresponding charged particle `edm4eic::TrackSegment`
    - points to the _same_ `DRICHMergedTrack`, to facilitate merging aerogel and gas PID results

```mermaid
flowchart TB
  classDef col fill:#00aaaa,color:black
  classDef fn fill:#c3b091,color:black
  classDef comp fill:#8888ff,color:black

  %% nodes
  CPIDAgl(<strong>DRICHAerogelIrtCherenkovParticleID</strong><br/>edm4eic::CherenkovParticleID):::col
  CPIDGas(<strong>DRICHGasIrtCherenkovParticleID</strong><br/>edm4eic::CherenkovParticleID):::col
  HypAglFn[hypotheses]:::fn
  HypGasFn[hypotheses]:::fn

  subgraph <strong>hypotheses from aerogel</strong>
    direction TB
    HypAgl0([Electron]):::comp
    HypAgl1([Pion]):::comp
    HypAgl2([Kaon]):::comp
    HypAgl3([Proton]):::comp
  end
  subgraph <strong>hypotheses from gas</strong>
    direction TB
    HypGas0([Electron]):::comp
    HypGas1([Pion]):::comp
    HypGas2([Kaon]):::comp
    HypGas3([Proton]):::comp
  end

  TrackAglFn[chargedParticle]:::fn
  TrackGasFn[chargedParticle]:::fn
  Track(<strong>DRICHMergedTracks</strong><br/>edm4eic::TrackSegment):::col

  %% edges
  CPIDAgl ==> HypAglFn
  HypAglFn   ==> HypAgl0
  HypAglFn   ==> HypAgl1
  HypAglFn   ==> HypAgl2
  HypAglFn   ==> HypAgl3
  CPIDAgl ==> TrackAglFn ==> Track
  CPIDGas ==> HypGasFn
  HypGasFn   ==> HypGas0
  HypGasFn   ==> HypGas1
  HypGasFn   ==> HypGas2
  HypGasFn   ==> HypGas3
  CPIDGas ==> TrackGasFn ==> Track
```

### User-level PID Output
- Add `edm4hep::ParticleID` objects to `ReconstructedParticle`
  - `edm4hep::ParticleID` objects include a likelihood, PDG (and some index variables and `float` parameters for full generality)
  - Use 1-1 relation `ReconstructedParticle::particleIDUsed` to specify the most-likely `edm4hep::ParticleID` object;
    the diagram below exemplifies this for a pion
  - Use 1-N relation `ReconstructedParticle::particleIDs` to link all the `edm4hep::ParticleID` objects
- User can then access PDG via:
```cpp
ReconstructedParticleAssociation.getRec().getParticleIDUsed().getPDG(); // most likely PDG from PID
ReconstructedParticleAssociation.getSim().getPDG();                     // true PDG
```
```mermaid
flowchart TB
  classDef col fill:#00aaaa,color:black
  classDef fn fill:#c3b091,color:black
  classDef comp fill:#8888ff,color:black

  %% nodes
  ReconPart(<strong>ReconstructedChargedParticles</strong><br/>edm4eic::ReconstructedParticle):::col
  ReconAssoc(<strong>ReconstructedChargedParticleAssociations</strong><br/>edm4eic::MCRecoParticleAssociation):::col
  MCPart(<strong>MCParticles</strong><br/>MC True Particles<br/>edm4hep::MCParticles):::col
  RecFn[rec]:::fn
  SimFn[sim]:::fn

  PDGReconFn[PDG]:::fn
  PDGMCFn[PDG]:::fn

  PDGTrue([True PDG]):::comp

  ParticleIDsFn[particleIDs]:::fn
  ParticleIDUsedFn[particleIDUsed]:::fn

  subgraph <strong>Particle ID Objects</strong>
    direction LR
    Hyp0(edm4hep::ParticleID):::col ==> Pdg0Fn[PDG]:::fn ==> Pdg0([Electron]):::comp
    Hyp1(edm4hep::ParticleID):::col ==> Pdg1Fn[PDG]:::fn ==> Pdg1([Pion]):::comp
    Hyp2(edm4hep::ParticleID):::col ==> Pdg2Fn[PDG]:::fn ==> Pdg2([Kaon]):::comp
    Hyp3(edm4hep::ParticleID):::col ==> Pdg3Fn[PDG]:::fn ==> Pdg3([Proton]):::comp
  end

  %% edges
  ReconAssoc ==> RecFn   ==> ReconPart
  ReconAssoc ==> SimFn   ==> MCPart
  ReconPart  ==>|TO BE REMOVED| PDGReconFn ==> PDGTrue
  MCPart     ==> PDGMCFn ==> PDGTrue

  ReconPart ==> ParticleIDUsedFn ==> Hyp1
  ReconPart ==> ParticleIDsFn

  ParticleIDsFn ==> Hyp0
  ParticleIDsFn ==> Hyp1
  ParticleIDsFn ==> Hyp2
  ParticleIDsFn ==> Hyp3
  ```
