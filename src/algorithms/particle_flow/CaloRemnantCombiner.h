// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Subhadip Pal

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <cmath>
#include <cstddef>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#include "CaloRemnantCombinerConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

// --------------------------------------------------------------------------
//! Algorithm input/output
// --------------------------------------------------------------------------
using CaloRemnantCombinerAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4eic::ClusterCollection, edm4eic::ClusterCollection>,
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
                                     {"ECalClusters", "HCalClusters"},
                                     {"NeutralParticleCandidate"},
                                     "make neutral candidates from remnant clusters"} {}

  void process(const Input&, const Output&) const final;

private:
  struct ClusterEnergyCompare {
    const edm4eic::ClusterCollection* clusters;

    bool operator()(std::size_t a, std::size_t b) const {
      float ea = (*clusters)[a].getEnergy();
      float eb = (*clusters)[b].getEnergy();
      if (ea != eb) {
        return ea > eb; // highest energy first
      }
      return a < b; // tie-break by index
    }
  };

  std::vector<std::size_t>
  get_cluster_indices_for_merging(const edm4eic::ClusterCollection& clusters,
                                  std::set<std::size_t, ClusterEnergyCompare>& remaining,
                                  std::size_t seed_cluster_index, double delta_r_add,
                                  const edm4eic::ClusterCollection& seed) const;
};

} // namespace eicrecon
