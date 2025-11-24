// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Sylvester Joosten

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/MCRecoClusterParticleAssociationCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <map>
#include <string>
#include <string_view>

#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using TruthEnergyPositionClusterMergerAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4hep::MCParticleCollection, edm4eic::ClusterCollection,
                      edm4eic::MCRecoClusterParticleAssociationCollection,
                      edm4eic::ClusterCollection,
                      edm4eic::MCRecoClusterParticleAssociationCollection>,
    algorithms::Output<edm4eic::ClusterCollection,
                       edm4eic::MCRecoClusterParticleAssociationCollection>>;

/** Simple algorithm to merge the energy measurement from cluster1 with the position
   * measurement of cluster2 (in case matching clusters are found). If not, it will
   * propagate the raw cluster from cluster1 or cluster2
   *
   * Matching occurs based on the mc truth information of the clusters.
   *
   * \ingroup reco
   */
class TruthEnergyPositionClusterMerger : public TruthEnergyPositionClusterMergerAlgorithm, public WithPodConfig<NoConfig> {

public:
  TruthEnergyPositionClusterMerger(std::string_view name)
      : TruthEnergyPositionClusterMergerAlgorithm{
            name,
            {"mcParticles", "energyClusterCollection", "energyClusterAssociations",
             "positionClusterCollection", "positionClusterAssociations"},
            {"outputClusterCollection", "outputClusterAssociations"},
            "Merge energy and position clusters based on truth."} {}

public:
  void init() {}

  void process(const Input& input, const Output& output) const final;

  // get a map of MCParticle index --> cluster
  // input: cluster_collections --> list of handles to all cluster collections
  std::map<int, edm4eic::Cluster>
  indexedClusters(const edm4eic::ClusterCollection& clusters,
                  const edm4eic::MCRecoClusterParticleAssociationCollection& associations) const;
};

} // namespace eicrecon
