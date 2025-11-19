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
    std::size_t seed_ecal_index = findSeedCluster_index(*calo_clusters[0], visits_ecal);

    if (seed_ecal_index == -1) {
      info("No Seed Ecal cluster found for remnant combination.");
    }

    if (seed_ecal_index != -1) {
      // Get the cluster indices for merging
      std::set<std::size_t> ecalcluster_indices = getcluster_indices_for_merging(
          *calo_clusters[0], visits_ecal, seed_ecal_index, m_cfg.delta_r_add_em, *calo_clusters[0]);

      for (const auto& idx : ecalcluster_indices) {
        neutral_candidate_eh.addToClusters((*calo_clusters[0])[idx]);
      }

      std::set<std::size_t> e_hcalcluster_indices = getcluster_indices_for_merging(
          *calo_clusters[1], visits_hcal, seed_ecal_index, m_cfg.delta_r_add_h, *calo_clusters[0]);

      for (const auto& idx : e_hcalcluster_indices) {
        neutral_candidate_eh.addToClusters((*calo_clusters[1])[idx]);
      }

      out_neutral_candidates->push_back(neutral_candidate_eh);

    } // end of if (seed_ecal_index != -1)

  } // end of while (visits_ecal != visits_ecal_true)

  while (visits_hcal != visits_hcal_true) {

    edm4eic::MutableReconstructedParticle neutral_candidate_h;
    std::size_t seed_rem_hcal_index = findSeedCluster_index(*calo_clusters[1], visits_hcal);

    if (seed_rem_hcal_index == -1) {
      info("No Seed Hcal cluster found for remnant combination.");
    }

    if (seed_rem_hcal_index != -1) {

      std::set<std::size_t> rem_hcalcluster_indices;

      rem_hcalcluster_indices =
          getcluster_indices_for_merging(*calo_clusters[1], visits_hcal, seed_rem_hcal_index,
                                         m_cfg.delta_r_add_h, *calo_clusters[1]);

      for (const auto& idx : rem_hcalcluster_indices) {
        neutral_candidate_h.addToClusters((*calo_clusters[1])[idx]);
      }
      out_neutral_candidates->push_back(neutral_candidate_h);

    } // end of if (seed_rem_hcal_index != -1)

  } // end of while (visits_hcal != visits_hcal_true)

} // end of process

std::size_t CaloRemnantCombiner::findSeedCluster_index(const edm4eic::ClusterCollection& clusters,
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

std::set<std::size_t> CaloRemnantCombiner::getcluster_indices_for_merging(
    const edm4eic::ClusterCollection& clusters, std::vector<bool>& visits,
    std::size_t seed_cluster_index, double delta_r, const edm4eic::ClusterCollection& seed) {
  std::set<std::size_t> cluster_indices;

  for (std::size_t i = 0; i < clusters.size(); ++i) {
    if (visits[i]) {
      continue;
    }
    // get the distance between current cluster and seed cluster

    // Using simple Euclidean distance in 3D space
    ///double distance = edm4hep::utils::magnitude(clusters[i].getPosition() - seed[seed_cluster_index].getPosition());

    // Using delta R in the transverse plane (x-y plane)
    /*double dx = clusters[i].getPosition().x - seed[seed_cluster_index].getPosition().x;
    double dy = clusters[i].getPosition().y - seed[seed_cluster_index].getPosition().y;
    double distance = std::sqrt(dx * dx + dy * dy);*/

    // Using angular distance (delta R) in eta-phi space
    edm4hep::Vector3f seed_pos    = seed[seed_cluster_index].getPosition();
    edm4hep::Vector3f cluster_pos = clusters[i].getPosition();

    float eta_seed    = edm4hep::utils::eta(seed_pos);
    float phi_seed    = edm4hep::utils::angleAzimuthal(seed_pos);
    float eta_cluster = edm4hep::utils::eta(cluster_pos);
    float phi_cluster = edm4hep::utils::angleAzimuthal(cluster_pos);

    float dphi     = phi_cluster - phi_seed;
    float deta     = eta_cluster - eta_seed;
    float distance = std::sqrt(deta * deta + dphi * dphi);

    if (distance < delta_r) { // distance threshold for merging
      cluster_indices.insert(i);
      visits[i] = true;
    }
  }
  return cluster_indices;
}

} // namespace eicrecon
