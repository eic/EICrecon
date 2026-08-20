// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Subhadip Pal

#include <edm4eic/ClusterCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <cmath>
#include <set>
#include <tuple>

#include "CaloRemnantCombiner.h"
#include "algorithms/particle_flow/CaloRemnantCombinerConfig.h"

namespace eicrecon {

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
  auto [out_neutral_candidates]             = output;

  // Skip event if both cluster collections are empty
  if ((ecal_clusters->size() == 0) && (hcal_clusters->size() == 0)) {
    debug("Both ECAL and HCAL inputs are empty; skipping event.");
    return;
  }

  auto ecal_cmp = [ecal_clusters](std::size_t a, std::size_t b) {
    float ea = (*ecal_clusters)[a].getEnergy();
    float eb = (*ecal_clusters)[b].getEnergy();
    if (ea != eb) {
      return ea > eb; // highest energy first
    }
    return a < b; // tie-break by index
  };

  auto hcal_cmp = [hcal_clusters](std::size_t a, std::size_t b) {
    float ea = (*hcal_clusters)[a].getEnergy();
    float eb = (*hcal_clusters)[b].getEnergy();
    if (ea != eb) {
      return ea > eb; // highest energy first
    }
    return a < b; // tie-break by index
  };

  std::set<std::size_t, decltype(ecal_cmp)> remaining_ecal(ecal_cmp);
  std::set<std::size_t, decltype(hcal_cmp)> remaining_hcal(hcal_cmp);

  for (std::size_t i = 0; i < ecal_clusters->size(); ++i) {
    remaining_ecal.insert(i);
  }
  for (std::size_t i = 0; i < hcal_clusters->size(); ++i) {
    remaining_hcal.insert(i);
  }

  // Phase 1: Ecal-seeded candidates
  while (!remaining_ecal.empty()) {

    auto neutral_candidate_eh = out_neutral_candidates->create();

    // Seed is the first element (highest energy)
    std::size_t seed_ecal_index = *remaining_ecal.begin();

    // Gather ecal clusters within ecalDeltaR of the seed
    std::vector<std::size_t> ecal_to_merge = move_cluster_indices_for_merging(
        *ecal_clusters, remaining_ecal, seed_ecal_index, m_cfg.ecalDeltaR, *ecal_clusters);

    for (const auto& idx : ecal_to_merge) {
      neutral_candidate_eh.addToClusters((*ecal_clusters)[idx]);
    }

    // Gather hcal clusters within hcalDeltaR of the ecal seed
    std::vector<std::size_t> hcal_to_merge = move_cluster_indices_for_merging(
        *hcal_clusters, remaining_hcal, seed_ecal_index, m_cfg.hcalDeltaR, *ecal_clusters);

    for (const auto& idx : hcal_to_merge) {
      neutral_candidate_eh.addToClusters((*hcal_clusters)[idx]);
    }

  } // end of ecal-seeded loop

  // Phase 2: Hcal-seeded candidates (remaining hcal clusters)
  while (!remaining_hcal.empty()) {

    auto neutral_candidate_h = out_neutral_candidates->create();

    // Seed is the first element (highest energy)
    std::size_t seed_hcal_index = *remaining_hcal.begin();

    std::vector<std::size_t> hcal_to_merge = move_cluster_indices_for_merging(
        *hcal_clusters, remaining_hcal, seed_hcal_index, m_cfg.hcalDeltaR, *hcal_clusters);

    for (const auto& idx : hcal_to_merge) {
      neutral_candidate_h.addToClusters((*hcal_clusters)[idx]);
    }

  } // end of hcal-seeded loop
} // end of process

/*! Collects indices of clusters within `delta_r_add` of the seed cluster,
 *  removes them from `remaining`, and returns the collected indices.
 */
std::vector<std::size_t> CaloRemnantCombiner::move_cluster_indices_for_merging(
    const edm4eic::ClusterCollection& clusters, auto& remaining, std::size_t seed_cluster_index,
    double delta_r_add, const edm4eic::ClusterCollection& seed) const {

  std::vector<std::size_t> merged_indices;

  // get the position of the seed cluster to calculate distance to other clusters
  edm4hep::Vector3f seed_pos = seed[seed_cluster_index].getPosition();
  float eta_seed             = edm4hep::utils::eta(seed_pos);
  float phi_seed             = edm4hep::utils::angleAzimuthal(seed_pos);

  if (delta_r_add < 0.0)
    delta_r_add = 0.0;

  // Iterate over remaining indices; collect those within delta_r_add
  auto it = remaining.begin();
  while (it != remaining.end()) {
    std::size_t i = *it;

    edm4hep::Vector3f cluster_pos = clusters[i].getPosition();
    float eta_cluster             = edm4hep::utils::eta(cluster_pos);
    float phi_cluster             = edm4hep::utils::angleAzimuthal(cluster_pos);

    float dphi     = std::remainder(phi_cluster - phi_seed, 2 * M_PI);
    float deta     = eta_cluster - eta_seed;
    float distance = std::sqrt(deta * deta + dphi * dphi);

    if (distance <= delta_r_add) {
      merged_indices.push_back(i);
      it = remaining.erase(it);
    } else {
      ++it;
    }
  }
  return merged_indices;
}
} // namespace eicrecon
