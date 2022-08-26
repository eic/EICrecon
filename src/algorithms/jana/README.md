
We have a chain that looks like this: 

```mermaid
flowchart TB
  classDef alg fill:#f96;
  subgraph Simulation output
    direction LR
    tracker_endcap_collections(Endcap trk:<br />TrackerEndcapHits1<br/>TrackerEndcapHits2<br/>TrackerEndcapHits3<br/>[edm4hep::SimHit])
    tracker_barrel_collections(Endcap trk:<br />TrackerBarrelHits1<br/>TrackerBarrelHits2<br/>TrackerBarrelHits3<br/>[edm4hep::SimHit])
  end
  
  tracker_barrel_collections --> TrackerDigi[TrackerDigi]:::alg
  TrackerDigi --> TrackerBarrelRawHits(TrackerBarrelRawHits)
    
  tracker_endcap_collections --> TrackerDigi2[TrackerDigi]:::alg
  TrackerDigi2 --> TrackerEndcapRawHits(TrackerEndcapRawHits)
  
  TrackerSourceLinker[TrackerSourceLinker]:::alg
  
  TrackerBarrelRecHits --> TrackerSourceLinker
  TrackerEndcapRecHits --> TrackerSourceLinker  
```
