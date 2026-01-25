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

#include <DD4hep/Handle.h>
#include <Evaluator/DD4hepUnits.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <cmath>
#include <cstdlib>
#include <gsl/pointers>
#include <stdexcept>
#include <utility>
#include <variant>
#include <vector>

#include "algorithms/calorimetry/ImagingTopoClusterConfig.h"

namespace eicrecon {
template <typename... L> struct multilambda : L... {
  using L::operator()...;
  constexpr multilambda(L... lambda) : L(std::move(lambda))... {}
};

void ImagingTopoCluster::init() {

  multilambda _toDouble = {
      [](const std::string& v) { return dd4hep::_toDouble(v); },
      [](const double& v) { return v; },
  };

  // unitless conversion
  // sanity checks
  if (m_cfg.minClusterCenterEdep < m_cfg.minClusterHitEdep) {
    const std::string msg =
        "minClusterCenterEdep must be greater than or equal to minClusterHitEdep";
    error(msg);
    throw std::runtime_error(msg);
  }

  // using juggler internal units (GeV, dd4hep::mm, dd4hep::ns, dd4hep::rad)
  sameLayerDistXY[0]     = std::visit(_toDouble, m_cfg.sameLayerDistXY[0]) / dd4hep::mm;
  sameLayerDistXY[1]     = std::visit(_toDouble, m_cfg.sameLayerDistXY[1]) / dd4hep::mm;
  diffLayerDistXY[0]     = std::visit(_toDouble, m_cfg.diffLayerDistXY[0]) / dd4hep::mm;
  diffLayerDistXY[1]     = std::visit(_toDouble, m_cfg.diffLayerDistXY[1]) / dd4hep::mm;
  sameLayerDistEtaPhi[0] = m_cfg.sameLayerDistEtaPhi[0];
  sameLayerDistEtaPhi[1] = m_cfg.sameLayerDistEtaPhi[1] / dd4hep::rad;
  diffLayerDistEtaPhi[0] = m_cfg.diffLayerDistEtaPhi[0];
  diffLayerDistEtaPhi[1] = m_cfg.diffLayerDistEtaPhi[1] / dd4hep::rad;
  sameLayerDistTZ[0]     = m_cfg.sameLayerDistTZ[0] / dd4hep::mm;
  sameLayerDistTZ[1]     = m_cfg.sameLayerDistTZ[1] / dd4hep::mm;
  diffLayerDistTZ[0]     = m_cfg.diffLayerDistTZ[0] / dd4hep::mm;
  diffLayerDistTZ[1]     = m_cfg.diffLayerDistTZ[1] / dd4hep::mm;

  sectorDist           = m_cfg.sectorDist / dd4hep::mm;
  minClusterHitEdep    = m_cfg.minClusterHitEdep / dd4hep::GeV;
  minClusterCenterEdep = m_cfg.minClusterCenterEdep / dd4hep::GeV;
  minClusterEdep       = m_cfg.minClusterEdep / dd4hep::GeV;

  // same layer clustering parameters
  switch (m_cfg.sameLayerMode) {
  case ImagingTopoClusterConfig::ELayerMode::xy:
    if (m_cfg.sameLayerDistXY.size() != 2) {
      const std::string msg = "Expected 2 values (x_dist, y_dist) for sameLayerDistXY";
      error(msg);
      throw std::runtime_error(msg);
    }
    info("Same-layer clustering (same sector and same layer): "
         "Local [x, y] distance between hits <= [{:.4f} mm, {:.4f} mm].",
         sameLayerDistXY[0], sameLayerDistXY[1]);
    break;
  case ImagingTopoClusterConfig::ELayerMode::etaphi:
    if (m_cfg.sameLayerDistEtaPhi.size() != 2) {
      const std::string msg = "Expected 2 values (eta_dist, phi_dist) for sameLayerDistEtaPhi";
      error(msg);
      throw std::runtime_error(msg);
    }
    info("Same-layer clustering (same sector and same layer): "
         "Global [eta, phi] distance between hits <= [{:.4f}, {:.4f} rad].",
         sameLayerDistEtaPhi[0], sameLayerDistEtaPhi[1]);
    break;
  case ImagingTopoClusterConfig::ELayerMode::tz:
    if (m_cfg.sameLayerDistTZ.size() != 2) {
      const std::string msg = "Expected 2 values (t_dist, z_dist) for sameLayerDistTZ";
      error(msg);
      throw std::runtime_error(msg);
    }
    info("Same-layer clustering (same sector and same layer): "
         "Global [t, z] distance between hits <= [{:.4f} mm, {:.4f} mm].",
         sameLayerDistTZ[0], sameLayerDistTZ[1]);
    break;
  default:
    throw std::runtime_error("Unknown same-layer mode.");
  }

  // different layer clustering parameters
  switch (m_cfg.diffLayerMode) {
  case ImagingTopoClusterConfig::ELayerMode::etaphi:
    if (m_cfg.diffLayerDistEtaPhi.size() != 2) {
      const std::string msg = "Expected 2 values (eta_dist, phi_dist) for diffLayerDistEtaPhi";
      error(msg);
      throw std::runtime_error(msg);
    }
    info("Neighbour layers clustering (same sector and layer id within +- {:d}): "
         "Global [eta, phi] distance between hits <= [{:.4f}, {:.4f} rad].",
         m_cfg.neighbourLayersRange, diffLayerDistEtaPhi[0], diffLayerDistEtaPhi[1]);
    break;
  case ImagingTopoClusterConfig::ELayerMode::xy:
    if (m_cfg.diffLayerDistXY.size() != 2) {
      const std::string msg = "Expected 2 values (x_dist, y_dist) for diffLayerDistXY";
      error(msg);
      throw std::runtime_error(msg);
    }
    info("Neighbour layers clustering (same sector and layer id within +- {:d}): "
         "Global [x, y] distance between hits <= [{:.4f} mm, {:.4f} mm].",
         m_cfg.neighbourLayersRange, diffLayerDistXY[0], diffLayerDistXY[1]);
    break;
  case ImagingTopoClusterConfig::ELayerMode::tz:
    if (m_cfg.diffLayerDistTZ.size() != 2) {
      const std::string msg = "Expected 2 values (t_dist, z_dist) for diffLayerDistTZ";
      error(msg);
      throw std::runtime_error(msg);
    }
    info("Neighbour layers clustering (same sector and layer id within +- {:d}): "
         "Global [t, z] distance between hits <= [{:.4f} mm, {:.4f} mm].",
         m_cfg.neighbourLayersRange, diffLayerDistTZ[0], diffLayerDistTZ[1]);
    break;
  default:
    error("Unknown different-layer mode.");
    throw std::runtime_error("Unknown different-layer mode.");
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

    trace("hit {:d}: local position = ({}, {}, {}), global position = ({}, {}, {}), energy = {}",
          *idx, (*hits)[*idx].getLocal().x, (*hits)[*idx].getLocal().y, (*hits)[*idx].getLocal().z,
          (*hits)[*idx].getPosition().x, (*hits)[*idx].getPosition().y,
          (*hits)[*idx].getPosition().z, (*hits)[*idx].getEnergy());

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
    for (auto idx : groups[i]) {
      const auto& hit = (*hits)[idx];
      trace("  hit {} -> energy = {:.6f}, layer = {}, sector = {}, local = ({:.2f}, {:.2f}, "
            "{:.2f}), global = ({:.2f}, {:.2f}, {:.2f})",
            idx, hit.getEnergy(), hit.getLayer(), hit.getSector(), hit.getLocal().x,
            hit.getLocal().y, hit.getLocal().z, hit.getPosition().x, hit.getPosition().y,
            hit.getPosition().z);
    }
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
    switch (m_cfg.sameLayerMode) {
    case ImagingTopoClusterConfig::ELayerMode::xy:
      return (std::abs(h1.getLocal().x - h2.getLocal().x) <= sameLayerDistXY[0]) &&
             (std::abs(h1.getLocal().y - h2.getLocal().y) <= sameLayerDistXY[1]);

    case ImagingTopoClusterConfig::ELayerMode::etaphi:
      return (std::abs(edm4hep::utils::eta(h1.getPosition()) -
                       edm4hep::utils::eta(h2.getPosition())) <= sameLayerDistEtaPhi[0]) &&
             (std::abs(edm4hep::utils::angleAzimuthal(h1.getPosition()) -
                       edm4hep::utils::angleAzimuthal(h2.getPosition())) <= sameLayerDistEtaPhi[1]);

    case ImagingTopoClusterConfig::ELayerMode::tz: {
      // Layer mode 'tz' uses the average phi of the hits to define a rotated direction. The coordinate is a distance, not an angle.
      auto phi  = 0.5 * (edm4hep::utils::angleAzimuthal(h1.getPosition()) +
                        edm4hep::utils::angleAzimuthal(h2.getPosition()));
      auto h1_t = (h1.getPosition().x * sin(phi)) - (h1.getPosition().y * cos(phi));
      auto h2_t = (h2.getPosition().x * sin(phi)) - (h2.getPosition().y * cos(phi));
      auto h1_z = h1.getPosition().z;
      auto h2_z = h2.getPosition().z;

      return (std::abs(h1_t - h2_t) <= sameLayerDistTZ[0]) &&
             (std::abs(h1_z - h2_z) <= sameLayerDistTZ[1]);
    }

    default:
      error("Unknown layer mode for same-layer clustering.");
      return false;
    }
  } else if (ldiff <= m_cfg.neighbourLayersRange) {
    switch (m_cfg.diffLayerMode) {
    case eicrecon::ImagingTopoClusterConfig::ELayerMode::etaphi:
      return (std::abs(edm4hep::utils::eta(h1.getPosition()) -
                       edm4hep::utils::eta(h2.getPosition())) <= diffLayerDistEtaPhi[0]) &&
             (std::abs(edm4hep::utils::angleAzimuthal(h1.getPosition()) -
                       edm4hep::utils::angleAzimuthal(h2.getPosition())) <= diffLayerDistEtaPhi[1]);

    case eicrecon::ImagingTopoClusterConfig::ELayerMode::xy:
      // Here, the xy layer mode is based on global XY positions rather than local XY positions, and thus it only works for endcap detectors.
      return (std::abs(h1.getPosition().x - h2.getPosition().x) <= diffLayerDistXY[0]) &&
             (std::abs(h1.getPosition().y - h2.getPosition().y) <= diffLayerDistXY[1]);

    case eicrecon::ImagingTopoClusterConfig::ELayerMode::tz: {
      auto phi  = 0.5 * (edm4hep::utils::angleAzimuthal(h1.getPosition()) +
                        edm4hep::utils::angleAzimuthal(h2.getPosition()));
      auto h1_t = (h1.getPosition().x * sin(phi)) - (h1.getPosition().y * cos(phi));
      auto h2_t = (h2.getPosition().x * sin(phi)) - (h2.getPosition().y * cos(phi));
      auto h1_z = h1.getPosition().z;
      auto h2_z = h2.getPosition().z;

      return (std::abs(h1_t - h2_t) <= diffLayerDistTZ[0]) &&
             (std::abs(h1_z - h2_z) <= diffLayerDistTZ[1]);
    }

    default:
      error("Unknown layer mode for different-layer clustering.");
      return false;
    }
  }

  // not in adjacent layers
  return false;
}

} // namespace eicrecon
