// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Subhadip Pal

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <string>
#include <string_view>

#include "CaloRemnantCombinerConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using CaloRemnantCombinerAlgorithm =
    algorithms::Algorithm<algorithms::Input<std::vector<edm4eic::ClusterCollection>>,
                          algorithms::Output<edm4eic::ReconstructedParticleCollection>>;

class CaloRemnantCombiner : public CaloRemnantCombinerAlgorithm,
                            public WithPodConfig<CaloRemnantCombinerConfig> {

public:
  CaloRemnantCombiner(std::string_view name)
      : CaloRemnantCombinerAlgorithm{name,
                                     {"CaloClusters"},
                                     {"NeutralParticleCandidate"},
                                     "make neutral candidates from remnant clusters"} {}

  void init() final {};
  void process(const Input&, const Output&) const final;
  static std::size_t findSeedCluster_index(const edm4eic::ClusterCollection& clusters,
                                           std::vector<bool>& visits);
  static std::set<std::size_t>
  getcluster_indices_for_merging(const edm4eic::ClusterCollection& clusters,
                                 std::vector<bool>& visits, std::size_t seed_cluster_index,
                                 double delta_r, const edm4eic::ClusterCollection& seed);
};

} // namespace eicrecon
