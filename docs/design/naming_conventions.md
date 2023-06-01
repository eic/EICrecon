
# Naming conventions

## Collection names

- Should generally end with the type they contain.
  Example: `EcalEndcapPClusters` contains type `edm4eic::Cluster`

- If the type name is too long and unwieldy to be used as a suffix, it can be shortened, but should be done so consistently.
  Example: `ReconstructedParticleAssociations` contains type `edm4eic::MCRecoParticleAssociation`

- The prefix describes what the collection is used for. As these names become more specific, they should append to the _front_.

- Collection names should be pluralized, but only at the very end.
  Example: `EcalEndcapPClusterAssociations`, NOT `EcalEndcapPClustersAssociations`.
  Note: `MatchClusters_factory` relies on the convention that all of its input collections names end in `Clusters`
    and have a corresponding `ClusterAssociations` collection.


## Factory names

- JFactoryT's and JFactoryPodioT's should follow the pattern `OutputTypeName_factory_CollectionName`.
  Example: `Cluster_factory_EcalEndcapPClusters`

- Multifactories, ChainFactories, and ChainMultifactories (where there are multiple output types, parameterized
  collection names, or both, respectively) should follow the pattern `AlgorithmName_factory`.
  Example: `MatchClusters_factory`

- If a Multifactory produces predominantly one collection (e.g. if the other output collections are just associations.
  or debugging data), it is okay for them to follow the pattern `OutputTypeName_factory_CollectionName`.
