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

// --------------------------------------------------------------------------
//! Algorithm input/output
// --------------------------------------------------------------------------
/*! Input is a vector of calorimeter cluster collections. For now:
  *    - 1st entry in the vector should be the EMCal collection, and
  *    - 2nd entry in the vector should be the HCal collection.
  *  This can be generalized in the future.
  */
using CaloRemnantCombinerAlgorithm =
    algorithms::Algorithm<algorithms::Input<std::vector<edm4eic::ClusterCollection>>,
                          algorithms::Output<edm4eic::ReconstructedParticleCollection>>;

// ==========================================================================
//! Calorimeter Remnant Cluster Combiner
// ==========================================================================
/*! An algorithm which takes multiple calorimeter cluster collections and combines them into
 *  neutral-particle candidates based on distance matching.
 */
class CaloRemnantCombiner : public CaloRemnantCombinerAlgorithm,
                            public WithPodConfig<CaloRemnantCombinerConfig> {

public:
  CaloRemnantCombiner(std::string_view name)
      : CaloRemnantCombinerAlgorithm{name,
                                     {"CaloClusters"},
                                     {"NeutralParticleCandidate"},
                                     "make neutral candidates from remnant clusters"} {}

  void process(const Input&, const Output&) const final;
  static std::size_t find_seed_cluster_index(const edm4eic::ClusterCollection& clusters,
                                           std::vector<bool>& visits);
  static std::set<std::size_t>
  get_cluster_indices_for_merging(const edm4eic::ClusterCollection& clusters,
                                 std::vector<bool>& visits, std::size_t seed_cluster_index,
                                 double delta_r, const edm4eic::ClusterCollection& seed);
};

} // namespace eicrecon
