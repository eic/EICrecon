### [Generated particles](https://eicweb.phy.anl.gov/EIC/benchmarks/physics_benchmarks/-/blob/master/options/reconstruction.py#L312)

### [Truth level kinematics](https://eicweb.phy.anl.gov/EIC/benchmarks/physics_benchmarks/-/blob/master/options/reconstruction.py#L320)

```mermaid
flowchart LR
  classDef alg fill:#f96;
  subgraph Sim output
    in(MCParticles)
  end
  in--> B[`InclusiveKinematicsTruth`]:::alg -->C(InclusiveKinematicsTruth)
```

### [Roman pods](https://eicweb.phy.anl.gov/EIC/benchmarks/physics_benchmarks/-/blob/master/options/reconstruction.py#L328)

```mermaid
flowchart TB
  classDef alg fill:#f96;
  subgraph Sim output
    direction TB
    i1(ForwardRomanPotHits1<br/>ForwardRomanPotHits2)
    i2(ForwardOffMTrackerHits1<br/>ForwardOffMTrackerHits2<br/>ForwardOffMTrackerHits3<br/>ForwardOffMTrackerHits4)
  end

  i1 --> a1[SimTrackerHitsCollector]:::alg
  i2 --> a1

  a1 --> mi1(ForwardRomanPotAllHits)
  mi1 --> a2[TrackerDigi]:::alg
  a2 --> mi2(ForwardRomanPotRawHits)
  mi2 --> a3[TrackerHitReconstruction]:::alg
  a3 --> mi3(ForwardRomanPotRecHits)
  mi3-->a4[FarForwardParticles]:::alg
  a4 --> mi4(ForwardRomanPotParticles)
```

### [Off momentum tracker](https://eicweb.phy.anl.gov/EIC/benchmarks/physics_benchmarks/-/blob/master/options/reconstruction.py#L357)

### [B0 tracker](https://eicweb.phy.anl.gov/EIC/benchmarks/physics_benchmarks/-/blob/master/options/reconstruction.py#L386)

### [ZDC ECAL WSciFi](https://eicweb.phy.anl.gov/EIC/benchmarks/physics_benchmarks/-/blob/master/options/reconstruction.py#L402)

### [ZDC HCAL PbSciFi](https://eicweb.phy.anl.gov/EIC/benchmarks/physics_benchmarks/-/blob/master/options/reconstruction.py#L438)

### [Crystal Endcap Ecal](https://eicweb.phy.anl.gov/EIC/benchmarks/physics_benchmarks/-/blob/master/options/reconstruction.py#L474)

```mermaid
flowchart TB
classDef alg fill:#f96;
  subgraph Simulation output
    direction TB
    EcalEndcapNHits(EcalEndcapNHits)
  end
  EcalEndcapNHits --> CalHitDigi[CalHitDigi]:::alg
  CalHitDigi --> EcalEndcapNRawHits(EcalEndcapNRawHits)

  EcalEndcapNRawHits --> CalHitReco[CalHitReco]:::alg
  EcalEndcapNHits --> CalHitReco
  CalHitReco --> EcalEndcapNRecHits(EcalEndcapNRecHits)

  EcalEndcapNRecHits --> IslandCluster[IslandCluster]:::alg
  IslandCluster --> EcalEndcapNIslandProtoClusters(EcalEndcapNIslandProtoClusters)

  EcalEndcapNRecHits --> TruthClustering[TruthClustering]:::alg
  TruthClustering --> EcalEndcapNTruthProtoClusters(EcalEndcapNTruthProtoClusters)
  EcalEndcapNHits --> TruthClustering

  EcalEndcapNTruthProtoClusters --> RecoCoG[RecoCoG]:::alg
  EcalEndcapNHits --> RecoCoG
  RecoCoG --> EcalEndcapNClusters(EcalEndcapNClusters)
  RecoCoG --> EcalEndcapNClustersAssoc(EcalEndcapNClustersAssoc)

  EcalEndcapNClusters --> ClusterMerger[ClusterMerger]:::alg
  ClusterMerger --> EcalEndcapNMergedClusters(EcalEndcapNMergedClusters)
  ClusterMerger --> EcalEndcapNMergedClustersAssoc(EcalEndcapNMergedClustersAssoc)
```

### [Endcap ScFi Ecal](https://eicweb.phy.anl.gov/EIC/benchmarks/physics_benchmarks/-/blob/master/options/reconstruction.py#L536)

### [Central Barrel Ecal (SciGlass calorimeter)](https://eicweb.phy.anl.gov/EIC/benchmarks/physics_benchmarks/-/blob/master/options/reconstruction.py#L731)

### [Central Barrel Hcal](https://eicweb.phy.anl.gov/EIC/benchmarks/physics_benchmarks/-/blob/master/options/reconstruction.py#L796)

### [Hcal Hadron Endcap](https://eicweb.phy.anl.gov/EIC/benchmarks/physics_benchmarks/-/blob/master/options/reconstruction.py#L850)

### [Hcal Electron Endcap](https://eicweb.phy.anl.gov/EIC/benchmarks/physics_benchmarks/-/blob/master/options/reconstruction.py#L901)

### [Tracking](https://eicweb.phy.anl.gov/EIC/benchmarks/physics_benchmarks/-/blob/master/options/reconstruction.py#L952)

```mermaid
flowchart TB
  classDef alg fill:#f96;
  classDef col fill:#66ccff;
  subgraph Simulation output
    direction LR

    tracker_endcap_collections(Endcap trk:<br />InnerTrackerEndcapPHits<br/>InnerTrackerEndcapNHits<br/>MiddleTrackerEndcapPHits<br/>MiddleTrackerEndcapNHits<br/>OuterTrackerEndcapPHits<br/>OuterTrackerEndcapNHits<br/>)
    gem_endcap_collections(Endcap GEM:<br/>GEMTrackerEndcapHits1<br/>GEMTrackerEndcapHits2<br/>GEMTrackerEndcapHits3)
    tracker_barrel_collections(Barrel trk:<br/>SagittaSiBarrelHits<br/>OuterSiBarrelHits)
    mpgd_barrel_collections(InnerMPGDBarrelHits<br/>OuterMPGDBarrelHits)
    vertex_barrel_collections(Barrel vtx:<br/>VertexBarrelHits)
  end

  tracker_endcap_collections --> SimTrackerHitsCollector2[SimTrackerHitsCollector]:::col
  SimTrackerHitsCollector2 --> TrackerEndcapAllHits(TrackerEndcapAllHits)
  TrackerEndcapAllHits --> TrackerDigi2[TrackerDigi]:::alg
  TrackerDigi2 --> TrackerEndcapRawHits(TrackerEndcapRawHits)
  TrackerEndcapRawHits --> TrackerHitReconstruction2[TrackerHitReconstruction]:::alg
  TrackerHitReconstruction2 --> TrackerEndcapRecHits(TrackerEndcapRecHits)

  gem_endcap_collections --> SimTrackerHitsCollector5[SimTrackerHitsCollector]:::col
  SimTrackerHitsCollector5 --> GEMTrackerEndcapAllHits(GEMTrackerEndcapAllHits)
  GEMTrackerEndcapAllHits --> TrackerDigi5[TrackerDigi]:::alg
  TrackerDigi5 --> GEMTrackerEndcapRawHits(GEMTrackerEndcapRawHits)
  GEMTrackerEndcapRawHits --> TrackerHitReconstruction5[TrackerHitReconstruction]:::alg
  TrackerHitReconstruction5 --> GEMTrackerEndcapRecHits(GEMTrackerEndcapRecHits)

  tracker_barrel_collections --> SimTrackerHitsCollector[SimTrackerHitsCollector]:::col
  SimTrackerHitsCollector --> TrackerBarrelAllHits(TrackerBarrelAllHits)
  TrackerBarrelAllHits --> TrackerDigi[TrackerDigi]:::alg
  TrackerDigi --> TrackerBarrelRawHits(TrackerBarrelRawHits)
  TrackerBarrelRawHits --> TrackerHitReconstruction[TrackerHitReconstruction]:::alg
  TrackerHitReconstruction --> TrackerBarrelRecHits(TrackerBarrelRecHits)

  mpgd_barrel_collections --> SimTrackerHitsCollector4[SimTrackerHitsCollector]:::col
  SimTrackerHitsCollector4 -->  MPGDTrackerBarrelAllHits(MPGDTrackerBarrelAllHits)
  MPGDTrackerBarrelAllHits --> TrackerDigi4[TrackerDigi]:::alg
  TrackerDigi4 --> MPGDTrackerBarrelRawHits(MPGDTrackerBarrelRawHits)
  MPGDTrackerBarrelRawHits --> TrackerHitReconstruction4[TrackerHitReconstruction]:::alg
  TrackerHitReconstruction4 --> MPGDTrackerBarrelRecHits(MPGDTrackerBarrelRecHits)

  vertex_barrel_collections --> SimTrackerHitsCollector3[SimTrackerHitsCollector]:::col
  SimTrackerHitsCollector3 --> VertexBarrelAllHits(VertexBarrelAllHits)
  VertexBarrelAllHits --> TrackerDigi3[TrackerDigi]:::alg
  TrackerDigi3 --> VertexBarrelRawHits(VertexBarrelRawHits)
  VertexBarrelRawHits --> TrackerHitReconstruction3[TrackerHitReconstruction]:::alg
  TrackerHitReconstruction3 --> VertexBarrelRecHits(VertexBarrelRecHits)

  TrackerHitsCollector[TrackerHitsCollector]:::col

  TrackerBarrelRecHits --> TrackerHitsCollector
  TrackerEndcapRecHits --> TrackerHitsCollector
  VertexBarrelRecHits --> TrackerHitsCollector
  MPGDTrackerBarrelRecHits --> TrackerHitsCollector
  GEMTrackerEndcapRecHits --> TrackerHitsCollector

  TrackerHitsCollector --> TrackerSourceLinker[TrackerSourceLinker]:::alg

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
  CKFTracking --> trajectories

  trajectories --> ParticlesFromTrackFit[ParticlesFromTrackFit]:::alg
  ParticlesFromTrackFit --> outputParticles
  ParticlesFromTrackFit --> outputTrackParameters
```


### Tracking no collectors

Collectors are very simple algorithms that basically concatenate incoming collections. The removing them makes
the diagram easier to conceive and more PowerPoint friendly.

```mermaid
flowchart TB
  classDef alg fill:#f96;
  classDef col fill:#66ccff;
  subgraph Simulation output
    direction LR

    tracker_endcap_collections(Endcap trk:<br />InnerTrackerEndcapPHits<br/>InnerTrackerEndcapNHits<br/>MiddleTrackerEndcapPHits<br/>MiddleTrackerEndcapNHits<br/>OuterTrackerEndcapPHits<br/>OuterTrackerEndcapNHits<br/>)
    gem_endcap_collections(Endcap GEM:<br/>GEMTrackerEndcapHits1<br/>GEMTrackerEndcapHits2<br/>GEMTrackerEndcapHits3)
    tracker_barrel_collections(Barrel trk:<br/>SagittaSiBarrelHits<br/>OuterSiBarrelHits)
    mpgd_barrel_collections(InnerMPGDBarrelHits<br/>OuterMPGDBarrelHits)
    vertex_barrel_collections(Barrel vtx:<br/>VertexBarrelHits)
  end

  tracker_endcap_collections --> TrackerDigi2[TrackerDigi]:::alg
  TrackerDigi2 --> TrackerEndcapRawHits(TrackerEndcapRawHits)
  TrackerEndcapRawHits --> TrackerHitReconstruction2[TrackerHitReconstruction]:::alg
  TrackerHitReconstruction2 --> TrackerEndcapRecHits(TrackerEndcapRecHits)

  gem_endcap_collections --> TrackerDigi5[TrackerDigi]:::alg
  TrackerDigi5 --> GEMTrackerEndcapRawHits(GEMTrackerEndcapRawHits)
  GEMTrackerEndcapRawHits --> TrackerHitReconstruction5[TrackerHitReconstruction]:::alg
  TrackerHitReconstruction5 --> GEMTrackerEndcapRecHits(GEMTrackerEndcapRecHits)

  tracker_barrel_collections --> TrackerDigi[TrackerDigi]:::alg
  TrackerDigi --> TrackerBarrelRawHits(TrackerBarrelRawHits)
  TrackerBarrelRawHits --> TrackerHitReconstruction[TrackerHitReconstruction]:::alg
  TrackerHitReconstruction --> TrackerBarrelRecHits(TrackerBarrelRecHits)

  mpgd_barrel_collections --> TrackerDigi4[TrackerDigi]:::alg
  TrackerDigi4 --> MPGDTrackerBarrelRawHits(MPGDTrackerBarrelRawHits)
  MPGDTrackerBarrelRawHits --> TrackerHitReconstruction4[TrackerHitReconstruction]:::alg
  TrackerHitReconstruction4 --> MPGDTrackerBarrelRecHits(MPGDTrackerBarrelRecHits)

  vertex_barrel_collections --> TrackerDigi3[TrackerDigi]:::alg
  TrackerDigi3 --> VertexBarrelRawHits(VertexBarrelRawHits)
  VertexBarrelRawHits --> TrackerHitReconstruction3[TrackerHitReconstruction]:::alg
  TrackerHitReconstruction3 --> VertexBarrelRecHits(VertexBarrelRecHits)

  TrackerSourceLinker[TrackerSourceLinker]:::alg

  TrackerBarrelRecHits --> TrackerSourceLinker
  TrackerEndcapRecHits --> TrackerSourceLinker
  VertexBarrelRecHits --> TrackerSourceLinker
  MPGDTrackerBarrelRecHits --> TrackerSourceLinker
  GEMTrackerEndcapRecHits --> TrackerSourceLinker

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
  CKFTracking --> trajectories

  trajectories --> ParticlesFromTrackFit[ParticlesFromTrackFit]:::alg
  ParticlesFromTrackFit --> outputParticles
  ParticlesFromTrackFit --> outputTrackParameters
```

### [Event building](https://eicweb.phy.anl.gov/EIC/benchmarks/physics_benchmarks/-/blob/master/options/reconstruction.py#L1145)

```mermaid
flowchart LR
  classDef alg fill:#f96;
  subgraph Simulation output
    MCParticles(MCParticles)
  end

  subgraph Algorithms output
    direction TB
    outputTrackParameters(outputTrackParameters)
    EcalEndcapNMergedClusters(EcalEndcapNMergedClusters<br/>EcalEndcapNMergedClustersAssoc)
    EcalBarrelMergedClusters(EcalBarrelMergedClusters<br/>EcalBarrelMergedClusters)
    EcalEndcapPMergedClusters(EcalEndcapPMergedClusters<br />EcalEndcapPMergedClustersAssoc)
    HcalEndcapNClusters(HcalEndcapNClusters<br/>HcalEndcapNClustersAssoc)
    HcalBarrelClusters(HcalBarrelClusters<br/>HcalBarrelClustersAssoc)
    HcalEndcapPClusters(HcalEndcapPClusters<br/>HcalEndcapPClustersAssoc)
  end

  MCParticles --> ParticlesWithTruthPID[ParticlesWithTruthPID]:::alg
  outputTrackParameters --> ParticlesWithTruthPID
  ParticlesWithTruthPID --> ReconstructedChargedParticles(ReconstructedChargedParticles)
  ParticlesWithTruthPID --> ReconstructedChargedParticlesAssoc(ReconstructedChargedParticlesAssoc)

  MCParticles --> MatchClusters[MatchClusters]:::alg
  ReconstructedChargedParticles --> MatchClusters
  ReconstructedChargedParticlesAssoc --> MatchClusters
  EcalEndcapNMergedClusters --> MatchClusters
  EcalBarrelMergedClusters --> MatchClusters
  EcalEndcapPMergedClusters --> MatchClusters
  HcalEndcapNClusters --> MatchClusters
  HcalBarrelClusters --> MatchClusters
  HcalEndcapPClusters --> MatchClusters

  MatchClusters --> ReconstructedParticles(ReconstructedParticles)
  MatchClusters --> ReconstructedParticlesAssoc(ReconstructedParticlesAssoc)

```

### [DRICH](https://eicweb.phy.anl.gov/EIC/benchmarks/physics_benchmarks/-/blob/master/options/reconstruction.py#L1196)

```mermaid
flowchart LR
  classDef alg fill:#f96;
  subgraph Simulation output
    DRICHHits(DRICHHits)
  end

  DRICHHits --> PhotoMultiplierDigi[PhotoMultiplierDigi]:::alg --> DRICHRawHits(DRICHRawHits)
  DRICHRawHits --> PhotoMultiplierReco[PhotoMultiplierReco]:::alg --> DRICHRecHits(DRICHRecHits)
```

### [Inclusive kinematics](https://eicweb.phy.anl.gov/EIC/benchmarks/physics_benchmarks/-/blob/master/options/reconstruction.py#L1234)
