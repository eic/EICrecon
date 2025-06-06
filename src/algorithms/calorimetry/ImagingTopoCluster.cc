// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Chao Peng, Sylvester Joosten, Whitney Armstrong, Wouter Deconinck

/*
 *  Topological Cell Clustering Algorithm for Imaging Calorimetry
 *  1. group all the adjacent pixels
 *
 *  Author: Chao Peng (ANL), 06/02/2021
 *  Original reference: https://arxiv.org/pdf/1603.02934.pdf
 *
 *  Modifications:
 *
 *  Wouter Deconinck (Manitoba), 08/24/2024
 *  - converted hit storage model from std::vector to std::set sorted on layer
 *    where only hits remaining to be assigned to a group are in the set
 *  - erase hits that are too low in energy to be part of a cluster
 *  - converted group storage model from std::set to std::list to allow adding
 *    hits while keeping iterators valid
 *
 */

#include "algorithms/calorimetry/ImagingTopoCluster.h"

#include <Evaluator/DD4hepUnits.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <fmt/core.h>
#include <cmath>
#include <cstdlib>
#include <gsl/pointers>
#include <vector>

#include "algorithms/calorimetry/ImagingTopoClusterConfig.h"

namespace eicrecon {

void ImagingTopoCluster::init() {
  // unitless conversion
  // sanity checks
  if (m_cfg.localDistXY.size() != 2) {
    error("Expected 2 values (x_dist, y_dist) for localDistXY");
    return;
  }
  if (m_cfg.layerDistEtaPhi.size() != 2) {
    error("Expected 2 values (eta_dist, phi_dist) for layerDistEtaPhi");
    return;
  }
  if (m_cfg.minClusterCenterEdep < m_cfg.minClusterHitEdep) {
    error("minClusterCenterEdep must be greater than or equal to minClusterHitEdep");
    return;
  }

  // using juggler internal units (GeV, dd4hep::mm, dd4hep::ns, dd4hep::rad)
  localDistXY[0]       = m_cfg.localDistXY[0] / dd4hep::mm;
  localDistXY[1]       = m_cfg.localDistXY[1] / dd4hep::mm;
  layerDistXY[0]       = m_cfg.layerDistXY[0] / dd4hep::mm;
  layerDistXY[1]       = m_cfg.layerDistXY[1] / dd4hep::mm;
  layerDistEtaPhi[0]   = m_cfg.layerDistEtaPhi[0];
  layerDistEtaPhi[1]   = m_cfg.layerDistEtaPhi[1] / dd4hep::rad;
  sectorDist           = m_cfg.sectorDist / dd4hep::mm;
  minClusterHitEdep    = m_cfg.minClusterHitEdep / dd4hep::GeV;
  minClusterCenterEdep = m_cfg.minClusterCenterEdep / dd4hep::GeV;
  minClusterEdep       = m_cfg.minClusterEdep / dd4hep::GeV;

  // summarize the clustering parameters
  info("Local clustering (same sector and same layer): "
       "Local [x, y] distance between hits <= [{:.4f} mm, {:.4f} mm].",
       localDistXY[0], localDistXY[1]);
  switch (m_cfg.layerMode) {
  case ImagingTopoClusterConfig::ELayerMode::etaphi:
    info("Neighbour layers clustering (same sector and layer id within +- {:d}: "
         "Global [eta, phi] distance between hits <= [{:.4f}, {:.4f} rad].",
         m_cfg.neighbourLayersRange, layerDistEtaPhi[0], layerDistEtaPhi[1]);
    break;
  case ImagingTopoClusterConfig::ELayerMode::xy:
    info("Neighbour layers clustering (same sector and layer id within +- {:d}: "
         "Local [x, y] distance between hits <= [{:.4f} mm, {:.4f} mm].",
         m_cfg.neighbourLayersRange, layerDistXY[0], layerDistXY[1]);
    break;
  default:
    error("Unknown layer mode.");
  }
  info("Neighbour sectors clustering (different sector): "
       "Global distance between hits <= {:.4f} mm.",
       sectorDist);
}

void ImagingTopoCluster::process(const Input& input, const Output& output) const {

  const auto [hits] = input;
  auto [proto]      = output;

  // Sort hit indices (podio collections do not support std::sort)
  auto compare = [&hits](const auto& a, const auto& b) {
    // if !(a < b) and !(b < a), then a and b are equivalent
    // and only one of them will be allowed in a set
    if ((*hits)[a].getLayer() == (*hits)[b].getLayer()) {
      return (*hits)[a].getObjectID().index < (*hits)[b].getObjectID().index;
    }
    return (*hits)[a].getLayer() < (*hits)[b].getLayer();
  };
  // indices contains the remaining hit indices that have not
  // been assigned to a group yet
  std::set<std::size_t, decltype(compare)> indices(compare);
  // set does not have a size yet, so cannot fill with iota
  for (std::size_t i = 0; i < hits->size(); ++i) {
    indices.insert(i);
  }
  // ensure no hits were dropped due to equivalency in set
  if (hits->size() != indices.size()) {
    error("equivalent hits were dropped: #hits {:d}, #indices {:d}", hits->size(), indices.size());
  }

  // Group neighbouring hits
  std::vector<std::list<std::size_t>> groups;
  // because indices changes, the loop over indices requires some care:
  // - we must use iterators instead of range-for
  // - erase returns an incremented iterator and therefore acts as idx++
  // - when the set becomes empty on erase, idx is invalid and idx++ will be too
  // (also applies to loop in bfs_group below)
  for (auto idx = indices.begin(); idx != indices.end();
       indices.empty() ? idx = indices.end() : idx) {

    debug("hit {:d}: local position = ({}, {}, {}), global position = ({}, {}, {}), energy = {}",
          *idx, (*hits)[*idx].getLocal().x, (*hits)[*idx].getLocal().y,
          (*hits)[*idx].getPosition().z, (*hits)[*idx].getPosition().x,
          (*hits)[*idx].getPosition().y, (*hits)[*idx].getPosition().z, (*hits)[*idx].getEnergy());

    // not energetic enough for cluster center, but could still be cluster hit
    if ((*hits)[*idx].getEnergy() < minClusterCenterEdep) {
      idx++;
      continue;
    }

    // create a new group, and group all the neighbouring hits
    groups.emplace_back(std::list{*idx});
    bfs_group(*hits, indices, groups.back(), *idx);

    // wait with erasing until after bfs_group to ensure iterator is not invalidated in bfs_group
    idx = indices.erase(idx); // takes role of idx++
  }
  debug("found {} potential clusters (groups of hits)", groups.size());
  for (std::size_t i = 0; i < groups.size(); ++i) {
    debug("group {}: {} hits", i, groups[i].size());
  }

  // form clusters
  for (const auto& group : groups) {
    if (group.size() < m_cfg.minClusterNhits) {
      continue;
    }
    double energy = 0.;
    for (std::size_t idx : group) {
      energy += (*hits)[idx].getEnergy();
    }
    if (energy < minClusterEdep) {
      continue;
    }
    auto pcl = proto->create();
    for (std::size_t idx : group) {
      pcl.addToHits((*hits)[idx]);
      pcl.addToWeights(1);
    }
  }
}

// helper function to group hits
bool ImagingTopoCluster::is_neighbour(const edm4eic::CalorimeterHit& h1,
                                      const edm4eic::CalorimeterHit& h2) const {
  // different sectors, simple distance check
  if (h1.getSector() != h2.getSector()) {
    return std::hypot((h1.getPosition().x - h2.getPosition().x),
                      (h1.getPosition().y - h2.getPosition().y),
                      (h1.getPosition().z - h2.getPosition().z)) <= sectorDist;
  }

  // layer check
  int ldiff = std::abs(h1.getLayer() - h2.getLayer());
  // same layer, check local positions
  if (ldiff == 0) {
    return (std::abs(h1.getLocal().x - h2.getLocal().x) <= localDistXY[0]) &&
           (std::abs(h1.getLocal().y - h2.getLocal().y) <= localDistXY[1]);
  } else if (ldiff <= m_cfg.neighbourLayersRange) {
    switch (m_cfg.layerMode) {
    case eicrecon::ImagingTopoClusterConfig::ELayerMode::etaphi:
      return (std::abs(edm4hep::utils::eta(h1.getPosition()) -
                       edm4hep::utils::eta(h2.getPosition())) <= layerDistEtaPhi[0]) &&
             (std::abs(edm4hep::utils::angleAzimuthal(h1.getPosition()) -
                       edm4hep::utils::angleAzimuthal(h2.getPosition())) <= layerDistEtaPhi[1]);
    case eicrecon::ImagingTopoClusterConfig::ELayerMode::xy:
      return (std::abs(h1.getPosition().x - h2.getPosition().x) <= layerDistXY[0]) &&
             (std::abs(h1.getPosition().y - h2.getPosition().y) <= layerDistXY[1]);
    }
  }
  // not in adjacent layers
  return false;
}

} // namespace eicrecon
