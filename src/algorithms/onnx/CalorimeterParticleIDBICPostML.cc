// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Tomas Sosa, Wouter Deconinck

#include "CalorimeterParticleIDBICPostML.h"

#include <edm4eic/CalorimeterHit.h>
#include <edm4hep/MCParticle.h>
#include <fmt/format.h>
#include <podio/ObjectID.h>
#include <podio/RelationRange.h>
#include <podio/detail/Link.h>
#include <podio/detail/LinkCollectionImpl.h>
#include <algorithm>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <vector>

namespace eicrecon {

namespace {

  struct BICBranches {
    edm4eic::Cluster imaging;
    edm4eic::Cluster scifi;
    bool has_imaging = false;
    bool has_scifi   = false;
  };

  bool sameHitObject(const edm4eic::CalorimeterHit& first, const edm4eic::CalorimeterHit& second) {
    const auto first_id  = first.getObjectID();
    const auto second_id = second.getObjectID();
    return first_id.collectionID == second_id.collectionID && first_id.index == second_id.index;
  }

  bool sameHits(const edm4eic::Cluster& first, const edm4eic::Cluster& second) {
    const auto first_hits  = first.getHits();
    const auto second_hits = second.getHits();
    if (first_hits.empty() || first_hits.size() != second_hits.size()) {
      return false;
    }
    return std::all_of(first_hits.begin(), first_hits.end(), [&second_hits](const auto& hit) {
      return std::any_of(second_hits.begin(), second_hits.end(),
                         [&hit](const auto& other_hit) { return sameHitObject(hit, other_hit); });
    });
  }

  bool containsCluster(const edm4eic::ClusterCollection& clusters,
                       const edm4eic::Cluster& cluster) {
    return std::any_of(clusters.begin(), clusters.end(),
                       [&cluster](const auto& item) { return sameHits(item, cluster); });
  }

  BICBranches findBranches(const edm4eic::Cluster& merged,
                           const edm4eic::ClusterCollection& imaging_clusters,
                           const edm4eic::ClusterCollection& scifi_clusters) {
    BICBranches branches;
    for (const auto& child : merged.getClusters()) {
      if (containsCluster(imaging_clusters, child)) {
        branches.imaging     = child;
        branches.has_imaging = true;
      }
      if (containsCluster(scifi_clusters, child)) {
        branches.scifi     = child;
        branches.has_scifi = true;
      }
    }
    return branches;
  }

} // namespace

void CalorimeterParticleIDBICPostML::init() {
  // Nothing
}

void CalorimeterParticleIDBICPostML::process(
    const CalorimeterParticleIDBICPostML::Input& input,
    const CalorimeterParticleIDBICPostML::Output& output) const {

  const auto [standard_clusters, standard_assocs, bic_clusters, imaging_clusters,
              standard_scifi_clusters, selected_scifi_clusters, prediction_tensors] = input;
  auto [out_clusters, out_links, out_assocs, out_particle_ids]                      = output;

  // As in the EEMC PID chain, PreML and ONNX always provide one tensor.  For
  // an event with no BIC candidates its shape is [0, 2], which is a valid
  // zero-sized batch rather than a missing prediction collection.
  if (prediction_tensors->size() != 1) {
    error("Expected one prediction tensor collection entry, found {}", prediction_tensors->size());
    throw std::runtime_error("Bad prediction tensor count");
  }

  const edm4eic::Tensor prediction_tensor = (*prediction_tensors)[0];

  if (prediction_tensor.shape_size() != 2) {
    error("Expected prediction tensor rank 2, got {}", prediction_tensor.shape_size());
    throw std::runtime_error(
        fmt::format("Expected prediction tensor rank 2, got {}", prediction_tensor.shape_size()));
  }

  if (prediction_tensor.getShape(1) != 2) {
    error("Expected prediction tensor shape [N,2], got second dimension {}",
          prediction_tensor.getShape(1));
    throw std::runtime_error(
        fmt::format("Expected prediction tensor shape [N,2], got second dimension {}",
                    prediction_tensor.getShape(1)));
  }

  if (prediction_tensor.getElementType() != 1) {
    error("Expected float prediction tensor, got element type {}",
          prediction_tensor.getElementType());
    throw std::runtime_error(fmt::format("Expected float prediction tensor, got element type {}",
                                         prediction_tensor.getElementType()));
  }

  if (prediction_tensor.getShape(0) != static_cast<long>(bic_clusters->size())) {
    error("Prediction rows ({}) do not match E/p-selected merged BIC clusters ({})",
          prediction_tensor.getShape(0), bic_clusters->size());
    throw std::runtime_error(
        fmt::format("Prediction rows ({}) do not match E/p-selected merged BIC clusters ({})",
                    prediction_tensor.getShape(0), bic_clusters->size()));
  }

  std::vector<BICBranches> bic_branches;
  bic_branches.reserve(bic_clusters->size());
  for (const auto& bic_cluster : *bic_clusters) {
    bic_branches.push_back(findBranches(bic_cluster, *imaging_clusters, *selected_scifi_clusters));
  }

  for (const auto& standard_cluster : *standard_clusters) {
    auto out_cluster = standard_cluster.clone();
    out_clusters->push_back(out_cluster);

    for (const auto& in_assoc : *standard_assocs) {
      if (sameHits(in_assoc.getRec(), standard_cluster)) {
        auto out_link = out_links->create();
        out_link.setFrom(out_cluster);
        out_link.setTo(in_assoc.getSim());
        out_link.setWeight(in_assoc.getWeight());
        auto out_assoc = in_assoc.clone();
        out_assoc.setRec(out_cluster);
        out_assocs->push_back(out_assoc);
      }
    }

    const auto standard_branches =
        findBranches(standard_cluster, *imaging_clusters, *standard_scifi_clusters);
    if (!standard_branches.has_imaging || !standard_branches.has_scifi) {
      continue;
    }

    std::size_t matching_bic = bic_clusters->size();
    for (std::size_t i = 0; i < bic_branches.size(); ++i) {
      const auto& bic = bic_branches[i];
      if (!bic.has_imaging || !bic.has_scifi || !sameHits(bic.imaging, standard_branches.imaging) ||
          !sameHits(bic.scifi, standard_branches.scifi)) {
        continue;
      }
      if (matching_bic != bic_clusters->size()) {
        warning("Ambiguous BIC-to-standard BEMC match for standard cluster {}; no PID attached",
                standard_cluster.getObjectID().index);
        matching_bic = bic_clusters->size();
        break;
      }
      matching_bic = i;
    }

    if (matching_bic == bic_clusters->size()) {
      continue;
    }

    const float prob_pion =
        prediction_tensor.getFloatData(matching_bic * prediction_tensor.getShape(1));
    const float prob_electron =
        prediction_tensor.getFloatData(matching_bic * prediction_tensor.getShape(1) + 1);
    out_cluster.addToParticleIDs(out_particle_ids->create(0, -211, 0, prob_pion));
    out_cluster.addToParticleIDs(out_particle_ids->create(0, 11, 0, prob_electron));
  }
}

} // namespace eicrecon
