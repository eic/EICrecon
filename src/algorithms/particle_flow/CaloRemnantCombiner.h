// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Subhadip Pal

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <cstddef>
#include <optional>
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
/*! Input is a vector of calorimeter cluster collections. For now:
  *    - 1st entry in the vector should be the Ecal collection, and
  *    - 2nd entry in the vector should be the Hcal collection.
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
  static std::optional<std::size_t>
  find_seed_cluster_index(const edm4eic::ClusterCollection& clusters, std::vector<bool>& visits);

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

  // ----------------------------------------------------------------------------
  //! Find cluster indices for merging
  // ----------------------------------------------------------------------------
  /*! Collects indices of clusters within `delta_r_add` of the seed cluster,
 *  removes them from `remaining`, and returns the collected indices.
 */
  std::vector<std::size_t>
  get_cluster_indices_for_merging(const edm4eic::ClusterCollection& clusters,
                                  std::set<std::size_t, ClusterEnergyCompare>& remaining,
                                  std::size_t seed_cluster_index, double delta_r_add,
                                  const edm4eic::ClusterCollection& seed) const {

    std::vector<std::size_t> merged_indices;

    // get the position of the seed cluster to calculate distance to other clusters
    edm4hep::Vector3f seed_pos = seed[seed_cluster_index].getPosition();
    float eta_seed             = edm4hep::utils::eta(seed_pos);
    float phi_seed             = edm4hep::utils::angleAzimuthal(seed_pos);

    // Iterate over remaining indices; collect those within delta_r_add
    auto it = remaining.begin();
    while (it != remaining.end()) {
      std::size_t i = *it;

      edm4hep::Vector3f cluster_pos = clusters[i].getPosition();
      float eta_cluster             = edm4hep::utils::eta(cluster_pos);
      float phi_cluster             = edm4hep::utils::angleAzimuthal(cluster_pos);

      float dphi = phi_cluster - phi_seed;
      if (dphi > M_PI) {
        dphi -= 2 * M_PI;
      } else if (dphi < -M_PI) {
        dphi += 2 * M_PI;
      }
      float deta     = eta_cluster - eta_seed;
      float distance = std::sqrt(deta * deta + dphi * dphi);

      if (distance < delta_r_add) {
        merged_indices.push_back(i);
        it = remaining.erase(it);
      } else {
        ++it;
      }
    }
    return merged_indices;
  }
};

} // namespace eicrecon
