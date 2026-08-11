// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Subhadip Pal

#include <edm4eic/ClusterCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <gsl/pointers>
#include <set>
#include <tuple>

#include "CaloRemnantCombiner.h"
#include "algorithms/particle_flow/CaloRemnantCombinerConfig.h"

namespace eicrecon {

// ----------------------------------------------------------------------------
//! Process inputs
// ----------------------------------------------------------------------------
/*! Construct a candidate neutral particle via the
 *  following algorithm.
 *    1. Repeat the following the steps until every Ecal
 *       cluster has been used:
 *       a. Identify seed Ecal cluster
 *       b. Identify all Ecal clusters and Hcal clusters which
 *          lie within a radius of ecalDeltaR and hcalDeltaR
 *          around seed Ecal cluster respectively
 *       c. Combine all identified clusters into a neutral particle
 *          candidate
 *    2. Repeat the following steps until every Hcal
 *       cluster has been used:
 *       a. Identify seed Hcal cluster
 *       b. Identify all Hcal clusters which lie within a
 *          radius of hcalDeltaR around seed Hcal
 *          cluster
 *       c. Combine all identified clusters into a neutral particle
 *          candidate
 */
void CaloRemnantCombiner::process(const CaloRemnantCombiner::Input& input,
                                  const CaloRemnantCombiner::Output& output) const {

  const auto [ecal_clusters, hcal_clusters] = input;
  auto [out_neutral_candidates] = output;

  // Skip event if both cluster collections are empty
  if ((ecal_clusters->size() == 0) && (hcal_clusters->size() == 0)) {
    error("Both ECAL and HCAL inputs are empty; skipping event.");
    return;
  }

  // Build ordered sets of remaining cluster indices (highest energy first)
  ClusterEnergyCompare ecal_cmp{ecal_clusters};
  ClusterEnergyCompare hcal_cmp{hcal_clusters};

  std::set<std::size_t, ClusterEnergyCompare> remaining_ecal(ecal_cmp);
  std::set<std::size_t, ClusterEnergyCompare> remaining_hcal(hcal_cmp);

  for (std::size_t i = 0; i < ecal_clusters->size(); ++i) {
    remaining_ecal.insert(i);
  }
  for (std::size_t i = 0; i < hcal_clusters->size(); ++i) {
    remaining_hcal.insert(i);
  }

  // Phase 1: Ecal-seeded candidates
  while (!remaining_ecal.empty()) {

    edm4eic::MutableReconstructedParticle neutral_candidate_eh;

    // Seed is the first element (highest energy)
    std::size_t seed_ecal_index = *remaining_ecal.begin();

    // Gather ecal clusters within ecalDeltaR of the seed
    std::vector<std::size_t> ecal_to_merge = get_cluster_indices_for_merging(
        *ecal_clusters, remaining_ecal, seed_ecal_index, m_cfg.ecalDeltaR, *ecal_clusters);

    for (const auto& idx : ecal_to_merge) {
      neutral_candidate_eh.addToClusters((*ecal_clusters)[idx]);
    }

    // Gather hcal clusters within hcalDeltaR of the ecal seed
    std::vector<std::size_t> hcal_to_merge = get_cluster_indices_for_merging(
        *hcal_clusters, remaining_hcal, seed_ecal_index, m_cfg.hcalDeltaR, *ecal_clusters);

    for (const auto& idx : hcal_to_merge) {
      neutral_candidate_eh.addToClusters((*hcal_clusters)[idx]);
    }

    out_neutral_candidates->push_back(neutral_candidate_eh);

  } // end of ecal-seeded loop

  // Phase 2: Hcal-seeded candidates (remaining hcal clusters)
  while (!remaining_hcal.empty()) {

    edm4eic::MutableReconstructedParticle neutral_candidate_h;

    // Seed is the first element (highest energy)
    std::size_t seed_hcal_index = *remaining_hcal.begin();

    std::vector<std::size_t> hcal_to_merge = get_cluster_indices_for_merging(
        *hcal_clusters, remaining_hcal, seed_hcal_index, m_cfg.hcalDeltaR, *hcal_clusters);

    for (const auto& idx : hcal_to_merge) {
      neutral_candidate_h.addToClusters((*hcal_clusters)[idx]);
    }

    out_neutral_candidates->push_back(neutral_candidate_h);

  } // end of hcal-seeded loop

} // end of process

} // namespace eicrecon
