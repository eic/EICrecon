// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Subhadip Pal

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

#include "CaloRemnantCombinerConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using CaloRemnantCombinerAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4eic::ClusterCollection, edm4eic::ClusterCollection>,
                          algorithms::Output<edm4eic::ReconstructedParticleCollection>>;

// ==========================================================================
// Calorimeter Remnant Cluster Combiner
// ==========================================================================
/*! An algorithm which takes multiple calorimeter cluster collections and combines them into
 *  neutral-particle candidates based on distance matching.
 */
class CaloRemnantCombiner : public CaloRemnantCombinerAlgorithm,
                            public WithPodConfig<CaloRemnantCombinerConfig> {

public:
  CaloRemnantCombiner(std::string_view name)
      : CaloRemnantCombinerAlgorithm{name,
                                     {"ECalClusters", "HCalClusters"},
                                     {"NeutralParticleCandidate"},
                                     "make neutral candidates from remnant clusters"} {}

  void process(const Input&, const Output&) const final;

private:
  std::vector<std::size_t>
  move_cluster_indices_for_merging(const edm4eic::ClusterCollection& clusters, auto& remaining,
                                   std::size_t seed_cluster_index, double delta_r_add,
                                   const edm4eic::ClusterCollection& seed) const;
};

} // namespace eicrecon
