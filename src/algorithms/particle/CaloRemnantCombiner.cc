// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Subhadip Pal

#include <Evaluator/DD4hepUnits.h>
#include <algorithms/logger.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>

#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <fmt/core.h>
#include <podio/ObjectID.h>
#include <cmath>
#include <gsl/pointers>
#include <set>

#include "algorithms/particle/CaloRemnantCombinerConfig.h"

#include "CaloRemnantCombiner.h"

namespace eicrecon {

// ----------------------------------------------------------------------------
//! Process inputs
// ----------------------------------------------------------------------------
/*! Construct a candidate neutral particle via the
 *  following algorithm.
 *    1. Repeat the following the steps until every EMCal
 *       cluster has been used:
 *       a. Identify seed EMCal cluster
 *       b. Identify all EMCal clusters and HCal clusters which
 *          lie within a radius of deltaRAddEM and deltaRAddH
 *          around seed EMCal cluster respectively
 *       c. Combine all identified clusters into a neutral particle
 *          candidate
 *    2. Repeat the following steps until every HCal
 *       cluster has been used:
 *       a. Identify seed HCal cluster
 *       b. Identify all HCal clusters which lie within a
 *          radius of deltaRAddH around seed HCal
 *          cluster
 *       c. Combine all identified clusters into a neutral particle
 *          candidate
 */
void CaloRemnantCombiner::process(const CaloRemnantCombiner::Input& input,
                                  const CaloRemnantCombiner::Output& output) const {

  const auto [calo_clusters]    = input;
  auto [out_neutral_candidates] = output;

  std::vector<bool> visits_ecal(calo_clusters[0]->size(), false);
  std::vector<bool> visits_hcal(calo_clusters[1]->size(), false);

  std::vector<bool> visits_ecal_true(calo_clusters[0]->size(), true);
  std::vector<bool> visits_hcal_true(calo_clusters[1]->size(), true);

  while (visits_ecal != visits_ecal_true) {

    edm4eic::MutableReconstructedParticle neutral_candidate_eh;

    // Step 1: Find the seed Ecal cluster with highest energy
    std::size_t seed_ecal_index = find_seed_cluster_index(*calo_clusters[0], visits_ecal);

    if (seed_ecal_index == -1) {
      debug("No Seed Ecal cluster found for remnant combination");
    }

    if (seed_ecal_index != -1) {
      // Get the cluster indices for merging
      std::set<std::size_t> ecalcluster_indices = get_cluster_indices_for_merging(
          *calo_clusters[0], visits_ecal, seed_ecal_index, m_cfg.deltaRAddEM, *calo_clusters[0]);

      for (const auto& idx : ecalcluster_indices) {
        neutral_candidate_eh.addToClusters((*calo_clusters[0])[idx]);
      }

      std::set<std::size_t> e_hcalcluster_indices = get_cluster_indices_for_merging(
          *calo_clusters[1], visits_hcal, seed_ecal_index, m_cfg.deltaRAddH, *calo_clusters[0]);

      for (const auto& idx : e_hcalcluster_indices) {
        neutral_candidate_eh.addToClusters((*calo_clusters[1])[idx]);
      }

      out_neutral_candidates->push_back(neutral_candidate_eh);

    } // end of if (seed_ecal_index != -1)

  } // end of while (visits_ecal != visits_ecal_true)

  while (visits_hcal != visits_hcal_true) {

    edm4eic::MutableReconstructedParticle neutral_candidate_h;
    std::size_t seed_rem_hcal_index = find_seed_cluster_index(*calo_clusters[1], visits_hcal);

    if (seed_rem_hcal_index == -1) {
      info("No Seed Hcal cluster found for remnant combination.");
    }

    if (seed_rem_hcal_index != -1) {

      std::set<std::size_t> rem_hcalcluster_indices;

      rem_hcalcluster_indices = get_cluster_indices_for_merging(
          *calo_clusters[1], visits_hcal, seed_rem_hcal_index, m_cfg.deltaRAddH, *calo_clusters[1]);

      for (const auto& idx : rem_hcalcluster_indices) {
        neutral_candidate_h.addToClusters((*calo_clusters[1])[idx]);
      }
      out_neutral_candidates->push_back(neutral_candidate_h);

    } // end of if (seed_rem_hcal_index != -1)

  } // end of while (visits_hcal != visits_hcal_true)

} // end of process

// ----------------------------------------------------------------------------
//! Find seed cluster
// ----------------------------------------------------------------------------
/*! Identifies a seed (highest energy) cluster in a collection
 *  which sets the center of the cone in which clusters are
 *  combined.
 */
std::size_t CaloRemnantCombiner::find_seed_cluster_index(const edm4eic::ClusterCollection& clusters,
                                                         std::vector<bool>& visits) {
  double max_cluster_energy      = -1.0;
  std::size_t seed_cluster_index = -1;
  for (std::size_t i = 0; i < clusters.size(); ++i) {

    if (visits[i]) {
      continue;
    }

    if (clusters[i].getEnergy() > max_cluster_energy) {
      max_cluster_energy = clusters[i].getEnergy();
      seed_cluster_index = i;
    }
  }
  return seed_cluster_index;
}

// ----------------------------------------------------------------------------
//! Find cluster indices for merging
// ----------------------------------------------------------------------------
/*! Creates a set of indices corresponding to clusters which lie within
 *  a radius of `delta_r_add` around the seed cluster with index
 *  `seed_cluster_index`.
 */
std::set<std::size_t> CaloRemnantCombiner::get_cluster_indices_for_merging(
    const edm4eic::ClusterCollection& clusters, std::vector<bool>& visits,
    std::size_t seed_cluster_index, double delta_r_add, const edm4eic::ClusterCollection& seed) {
  std::set<std::size_t> cluster_indices;

  for (std::size_t i = 0; i < clusters.size(); ++i) {
    if (visits[i]) {
      continue;
    }
    // get the distance between current cluster and seed cluster
    // using angular distance (delta R) in eta-phi space
    //   - FIXME expand to allow for other distance metrics
    edm4hep::Vector3f seed_pos    = seed[seed_cluster_index].getPosition();
    edm4hep::Vector3f cluster_pos = clusters[i].getPosition();

    float eta_seed    = edm4hep::utils::eta(seed_pos);
    float phi_seed    = edm4hep::utils::angleAzimuthal(seed_pos);
    float eta_cluster = edm4hep::utils::eta(cluster_pos);
    float phi_cluster = edm4hep::utils::angleAzimuthal(cluster_pos);

    float dphi     = phi_cluster - phi_seed;
    float deta     = eta_cluster - eta_seed;
    float distance = std::sqrt(deta * deta + dphi * dphi);

    if (distance < delta_r_add) {
      cluster_indices.insert(i);
      visits[i] = true;
    }
  }
  return cluster_indices;
}

} // namespace eicrecon
