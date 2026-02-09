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
#include <DD4hep/Readout.h>
#include <DD4hep/IDDescriptor.h>
#include <algorithms/service.h>
#include <Evaluator/DD4hepUnits.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <fmt/core.h>
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

  m_idSpec = m_detector->readout(m_cfg.readout).idSpec();

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
  sameLayerDistXY[0] = std::visit(_toDouble, m_cfg.sameLayerDistXY[0]) / dd4hep::mm;
  sameLayerDistXY[1] = std::visit(_toDouble, m_cfg.sameLayerDistXY[1]) / dd4hep::mm;
  diffLayerDistXY[0] = std::visit(_toDouble, m_cfg.diffLayerDistXY[0]) / dd4hep::mm;
  diffLayerDistXY[1] = std::visit(_toDouble, m_cfg.diffLayerDistXY[1]) / dd4hep::mm;

  ScFi_sameLayerDistXY[0] = std::visit(_toDouble, m_cfg.ScFi_sameLayerDistXY[0]) / dd4hep::mm;
  ScFi_sameLayerDistXY[1] = std::visit(_toDouble, m_cfg.ScFi_sameLayerDistXY[1]) / dd4hep::mm;
  ScFi_diffLayerDistXY[0] = std::visit(_toDouble, m_cfg.ScFi_diffLayerDistXY[0]) / dd4hep::mm;
  ScFi_diffLayerDistXY[1] = std::visit(_toDouble, m_cfg.ScFi_diffLayerDistXY[1]) / dd4hep::mm;

  Img_sameLayerDistXY[0] = std::visit(_toDouble, m_cfg.Img_sameLayerDistXY[0]) / dd4hep::mm;
  Img_sameLayerDistXY[1] = std::visit(_toDouble, m_cfg.Img_sameLayerDistXY[1]) / dd4hep::mm;
  Img_diffLayerDistXY[0] = std::visit(_toDouble, m_cfg.Img_diffLayerDistXY[0]) / dd4hep::mm;
  Img_diffLayerDistXY[1] = std::visit(_toDouble, m_cfg.Img_diffLayerDistXY[1]) / dd4hep::mm;

  sameLayerDistXYZ[0] = m_cfg.sameLayerDistXYZ[0] / dd4hep::mm;
  sameLayerDistXYZ[1] = m_cfg.sameLayerDistXYZ[1] / dd4hep::mm;
  sameLayerDistXYZ[2] = m_cfg.sameLayerDistXYZ[2] / dd4hep::mm;
  diffLayerDistXYZ[0] = m_cfg.diffLayerDistXYZ[0] / dd4hep::mm;
  diffLayerDistXYZ[1] = m_cfg.diffLayerDistXYZ[1] / dd4hep::mm;
  diffLayerDistXYZ[2] = m_cfg.diffLayerDistXYZ[2] / dd4hep::mm;

  ScFi_sameLayerDistXYZ[0] = m_cfg.ScFi_sameLayerDistXYZ[0] / dd4hep::mm;
  ScFi_sameLayerDistXYZ[1] = m_cfg.ScFi_sameLayerDistXYZ[1] / dd4hep::mm;
  ScFi_sameLayerDistXYZ[2] = m_cfg.ScFi_sameLayerDistXYZ[2] / dd4hep::mm;
  ScFi_diffLayerDistXYZ[0] = m_cfg.ScFi_diffLayerDistXYZ[0] / dd4hep::mm;
  ScFi_diffLayerDistXYZ[1] = m_cfg.ScFi_diffLayerDistXYZ[1] / dd4hep::mm;
  ScFi_diffLayerDistXYZ[2] = m_cfg.ScFi_diffLayerDistXYZ[2] / dd4hep::mm;

  sameLayerDistEtaPhi[0] = m_cfg.sameLayerDistEtaPhi[0];
  sameLayerDistEtaPhi[1] = m_cfg.sameLayerDistEtaPhi[1] / dd4hep::rad;
  diffLayerDistEtaPhi[0] = m_cfg.diffLayerDistEtaPhi[0];
  diffLayerDistEtaPhi[1] = m_cfg.diffLayerDistEtaPhi[1] / dd4hep::rad;
  sameLayerDistTZ[0]     = m_cfg.sameLayerDistTZ[0] / dd4hep::mm;
  sameLayerDistTZ[1]     = m_cfg.sameLayerDistTZ[1] / dd4hep::mm;
  diffLayerDistTZ[0]     = m_cfg.diffLayerDistTZ[0] / dd4hep::mm;
  diffLayerDistTZ[1]     = m_cfg.diffLayerDistTZ[1] / dd4hep::mm;

  Img_sameLayerDistEtaPhi[0] = m_cfg.Img_sameLayerDistEtaPhi[0];
  Img_sameLayerDistEtaPhi[1] = m_cfg.Img_sameLayerDistEtaPhi[1] / dd4hep::rad;
  Img_diffLayerDistEtaPhi[0] = m_cfg.Img_diffLayerDistEtaPhi[0];
  Img_diffLayerDistEtaPhi[1] = m_cfg.Img_diffLayerDistEtaPhi[1] / dd4hep::rad;
  Img_sameLayerDistTZ[0]     = m_cfg.Img_sameLayerDistTZ[0] / dd4hep::mm;
  Img_sameLayerDistTZ[1]     = m_cfg.Img_sameLayerDistTZ[1] / dd4hep::mm;
  Img_diffLayerDistTZ[0]     = m_cfg.Img_diffLayerDistTZ[0] / dd4hep::mm;
  Img_diffLayerDistTZ[1]     = m_cfg.Img_diffLayerDistTZ[1] / dd4hep::mm;

  cross_system_DistXYZ[0] = m_cfg.cross_system_DistXYZ[0] / dd4hep::mm;
  cross_system_DistXYZ[1] = m_cfg.cross_system_DistXYZ[1] / dd4hep::mm;
  cross_system_DistXYZ[2] = m_cfg.cross_system_DistXYZ[2] / dd4hep::mm;

  sectorDist              = m_cfg.sectorDist / dd4hep::mm;
  cross_system_sectorDist = m_cfg.sectorDist / dd4hep::mm;
  ScFi_sectorDist         = m_cfg.ScFi_sectorDist / dd4hep::mm;
  Img_sectorDist          = m_cfg.Img_sectorDist / dd4hep::mm;
  minClusterHitEdep       = m_cfg.minClusterHitEdep / dd4hep::GeV;
  minClusterCenterEdep    = m_cfg.minClusterCenterEdep / dd4hep::GeV;
  minClusterEdep          = m_cfg.minClusterEdep / dd4hep::GeV;

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
  case ImagingTopoClusterConfig::ELayerMode::xyz:
    if (m_cfg.sameLayerDistXYZ.size() != 3) {
      const std::string msg = "Expected 3 values (x_dist, y_dist, z_dist) for sameLayerDistXYZ";
      error(msg);
      throw std::runtime_error(msg);
    }
    info("Same-layer clustering (same sector and same layer): "
         "Local [x, y, z] distance between hits <= [{:.4f} mm, {:.4f} mm, {:.4f} mm].",
         sameLayerDistXYZ[0], sameLayerDistXYZ[1], sameLayerDistXYZ[2]);
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
  case ImagingTopoClusterConfig::ELayerMode::xyz:
    if (m_cfg.diffLayerDistXYZ.size() != 3) {
      const std::string msg = "Expected 3 values (x_dist, y_dist, y_dist) for diffLayerDistXYZ";
      error(msg);
      throw std::runtime_error(msg);
    }
    info("Neighbour layers clustering (same sector and layer id within +- {:d}): "
         "Global [x, y, z] distance between hits <= [{:.4f} mm, {:.4f} mm, {:.4f} mm].",
         m_cfg.neighbourLayersRange, diffLayerDistXYZ[0], diffLayerDistXYZ[1], diffLayerDistXYZ[2]);
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

  // ScFi_layer mode
  switch (m_cfg.ScFi_sameLayerMode) {
  case ImagingTopoClusterConfig::ELayerMode::xy:
    if (m_cfg.ScFi_sameLayerDistXY.size() != 2) {
      const std::string msg = "Expected 2 values (x_dist, y_dist) for sameLayerDistXY";
      error(msg);
      throw std::runtime_error(msg);
    }
    info("ScFi_Same-layer clustering (same sector and same layer): "
         "Local [x, y] distance between hits <= [{:.4f} mm, {:.4f} mm].",
         ScFi_sameLayerDistXY[0], ScFi_sameLayerDistXY[1]);
    break;
  case ImagingTopoClusterConfig::ELayerMode::xyz:
    if (m_cfg.ScFi_sameLayerDistXYZ.size() != 3) {
      const std::string msg =
          "Expected 3 values (x_dist, y_dist, z_dist) for ScFi_sameLayerDistXYZ";
      error(msg);
      throw std::runtime_error(msg);
    }
    info("ScFi_Same-layer clustering (same sector and same layer): "
         "Local [x, y, z] distance between hits <= [{:.4f} mm, {:.4f} mm, {:.4f} mm].",
         ScFi_sameLayerDistXYZ[0], ScFi_sameLayerDistXYZ[1], ScFi_sameLayerDistXYZ[2]);
    break;
  case ImagingTopoClusterConfig::ELayerMode::etaphi:
    if (m_cfg.ScFi_sameLayerDistEtaPhi.size() != 2) {
      const std::string msg = "Expected 2 values (eta_dist, phi_dist) for ScFi_sameLayerDistEtaPhi";
      error(msg);
      throw std::runtime_error(msg);
    }
    info("ScFi_Same-layer clustering (same sector and same layer): "
         "Global [eta, phi] distance between hits <= [{:.4f}, {:.4f} rad].",
         ScFi_sameLayerDistEtaPhi[0], ScFi_sameLayerDistEtaPhi[1]);
    break;
  case ImagingTopoClusterConfig::ELayerMode::tz:
    if (m_cfg.ScFi_sameLayerDistTZ.size() != 2) {
      const std::string msg = "Expected 2 values (t_dist, z_dist) for ScFi_sameLayerDistTZ";
      error(msg);
      throw std::runtime_error(msg);
    }
    info("ScFi_Same-layer clustering (same sector and same layer): "
         "Global [t, z] distance between hits <= [{:.4f} mm, {:.4f} mm].",
         ScFi_sameLayerDistTZ[0], ScFi_sameLayerDistTZ[1]);
    break;
  default:
    throw std::runtime_error("Unknown same-layer mode.");
  }

  switch (m_cfg.ScFi_diffLayerMode) {
  case ImagingTopoClusterConfig::ELayerMode::etaphi:
    if (m_cfg.ScFi_diffLayerDistEtaPhi.size() != 2) {
      const std::string msg = "Expected 2 values (eta_dist, phi_dist) for ScFi_diffLayerDistEtaPhi";
      error(msg);
      throw std::runtime_error(msg);
    }
    info("ScFi_Neighbour layers clustering (same sector and layer id within +- {:d}): "
         "Global [eta, phi] distance between hits <= [{:.4f}, {:.4f} rad].",
         m_cfg.neighbourLayersRange, ScFi_diffLayerDistEtaPhi[0], ScFi_diffLayerDistEtaPhi[1]);
    break;
  case ImagingTopoClusterConfig::ELayerMode::xy:
    if (m_cfg.ScFi_diffLayerDistXY.size() != 2) {
      const std::string msg = "Expected 2 values (x_dist, y_dist) for ScFi_diffLayerDistXY";
      error(msg);
      throw std::runtime_error(msg);
    }
    info("ScFi_Neighbour layers clustering (same sector and layer id within +- {:d}): "
         "Global [x, y] distance between hits <= [{:.4f} mm, {:.4f} mm].",
         m_cfg.neighbourLayersRange, ScFi_diffLayerDistXY[0], ScFi_diffLayerDistXY[1]);
    break;
  case ImagingTopoClusterConfig::ELayerMode::xyz:
    if (m_cfg.ScFi_diffLayerDistXYZ.size() != 3) {
      const std::string msg =
          "Expected 3 values (x_dist, y_dist, y_dist) for ScFi_diffLayerDistXYZ";
      error(msg);
      throw std::runtime_error(msg);
    }
    info("ScFi_Neighbour layers clustering (same sector and layer id within +- {:d}): "
         "Global [x, y, z] distance between hits <= [{:.4f} mm, {:.4f} mm, {:.4f} mm].",
         m_cfg.neighbourLayersRange, ScFi_diffLayerDistXYZ[0], ScFi_diffLayerDistXYZ[1],
         ScFi_diffLayerDistXYZ[2]);
    break;
  case ImagingTopoClusterConfig::ELayerMode::tz:
    if (m_cfg.ScFi_diffLayerDistTZ.size() != 2) {
      const std::string msg = "Expected 2 values (t_dist, z_dist) for ScFi_diffLayerDistTZ";
      error(msg);
      throw std::runtime_error(msg);
    }
    info("ScFi_Neighbour layers clustering (same sector and layer id within +- {:d}): "
         "Global [t, z] distance between hits <= [{:.4f} mm, {:.4f} mm].",
         m_cfg.neighbourLayersRange, ScFi_diffLayerDistTZ[0], ScFi_diffLayerDistTZ[1]);
    break;
  default:
    error("Unknown different-layer mode.");
    throw std::runtime_error("Unknown different-layer mode.");
  }
  info("ScFi_Neighbour sectors clustering (different sector): "
       "Global distance between hits <= {:.4f} mm.",
       ScFi_sectorDist);

  // Img_layer mode

  switch (m_cfg.Img_sameLayerMode) {
  case ImagingTopoClusterConfig::ELayerMode::xy:
    if (m_cfg.Img_sameLayerDistXY.size() != 2) {
      const std::string msg = "Expected 2 values (x_dist, y_dist) for Img_sameLayerDistXY";
      error(msg);
      throw std::runtime_error(msg);
    }
    info("Img_Same-layer clustering (same sector and same layer): "
         "Local [x, y] distance between hits <= [{:.4f} mm, {:.4f} mm].",
         Img_sameLayerDistXY[0], Img_sameLayerDistXY[1]);
    break;
  case ImagingTopoClusterConfig::ELayerMode::xyz:
    if (m_cfg.Img_sameLayerDistXYZ.size() != 3) {
      const std::string msg = "Expected 3 values (x_dist, y_dist, z_dist) for Img_sameLayerDistXYZ";
      error(msg);
      throw std::runtime_error(msg);
    }
    info("Img_Same-layer clustering (same sector and same layer): "
         "Local [x, y, z] distance between hits <= [{:.4f} mm, {:.4f} mm, {:.4f} mm].",
         Img_sameLayerDistXYZ[0], Img_sameLayerDistXYZ[1], Img_sameLayerDistXYZ[2]);
    break;
  case ImagingTopoClusterConfig::ELayerMode::etaphi:
    if (m_cfg.Img_sameLayerDistEtaPhi.size() != 2) {
      const std::string msg = "Expected 2 values (eta_dist, phi_dist) for Img_sameLayerDistEtaPhi";
      error(msg);
      throw std::runtime_error(msg);
    }
    info("Img_Same-layer clustering (same sector and same layer): "
         "Global [eta, phi] distance between hits <= [{:.4f}, {:.4f} rad].",
         Img_sameLayerDistEtaPhi[0], Img_sameLayerDistEtaPhi[1]);
    break;
  case ImagingTopoClusterConfig::ELayerMode::tz:
    if (m_cfg.Img_sameLayerDistTZ.size() != 2) {
      const std::string msg = "Expected 2 values (t_dist, z_dist) for Img_sameLayerDistTZ";
      error(msg);
      throw std::runtime_error(msg);
    }
    info("Img_Same-layer clustering (same sector and same layer): "
         "Global [t, z] distance between hits <= [{:.4f} mm, {:.4f} mm].",
         Img_sameLayerDistTZ[0], Img_sameLayerDistTZ[1]);
    break;
  default:
    throw std::runtime_error("Unknown same-layer mode.");
  }

  switch (m_cfg.Img_diffLayerMode) {
  case ImagingTopoClusterConfig::ELayerMode::etaphi:
    if (m_cfg.Img_diffLayerDistEtaPhi.size() != 2) {
      const std::string msg = "Expected 2 values (eta_dist, phi_dist) for Img_diffLayerDistEtaPhi";
      error(msg);
      throw std::runtime_error(msg);
    }
    info("Img_Neighbour layers clustering (same sector and layer id within +- {:d}): "
         "Global [eta, phi] distance between hits <= [{:.4f}, {:.4f} rad].",
         m_cfg.neighbourLayersRange, Img_diffLayerDistEtaPhi[0], Img_diffLayerDistEtaPhi[1]);
    break;
  case ImagingTopoClusterConfig::ELayerMode::xy:
    if (m_cfg.Img_diffLayerDistXY.size() != 2) {
      const std::string msg = "Expected 2 values (x_dist, y_dist) for Img_diffLayerDistXY";
      error(msg);
      throw std::runtime_error(msg);
    }
    info("Img_Neighbour layers clustering (same sector and layer id within +- {:d}): "
         "Global [x, y] distance between hits <= [{:.4f} mm, {:.4f} mm].",
         m_cfg.neighbourLayersRange, Img_diffLayerDistXY[0], Img_diffLayerDistXY[1]);
    break;
  case ImagingTopoClusterConfig::ELayerMode::xyz:
    if (m_cfg.Img_diffLayerDistXYZ.size() != 3) {
      const std::string msg = "Expected 3 values (x_dist, y_dist, y_dist) for Img_diffLayerDistXYZ";
      error(msg);
      throw std::runtime_error(msg);
    }
    info("Img_Neighbour layers clustering (same sector and layer id within +- {:d}): "
         "Global [x, y, z] distance between hits <= [{:.4f} mm, {:.4f} mm, {:.4f} mm].",
         m_cfg.neighbourLayersRange, Img_diffLayerDistXYZ[0], Img_diffLayerDistXYZ[1],
         Img_diffLayerDistXYZ[2]);
    break;
  case ImagingTopoClusterConfig::ELayerMode::tz:
    if (m_cfg.Img_diffLayerDistTZ.size() != 2) {
      const std::string msg = "Expected 2 values (t_dist, z_dist) for Img_diffLayerDistTZ";
      error(msg);
      throw std::runtime_error(msg);
    }
    info("Img_Neighbour layers clustering (same sector and layer id within +- {:d}): "
         "Global [t, z] distance between hits <= [{:.4f} mm, {:.4f} mm].",
         m_cfg.neighbourLayersRange, Img_diffLayerDistTZ[0], Img_diffLayerDistTZ[1]);
    break;
  default:
    error("Unknown different-layer mode.");
    throw std::runtime_error("Unknown different-layer mode.");
  }
  info("Img_Neighbour sectors clustering (different sector): "
       "Global distance between hits <= {:.4f} mm.",
       Img_sectorDist);
}

void ImagingTopoCluster::process(const Input& input, const Output& output) const {

  const auto [hits] = input;
  auto [proto]      = output;

  // Group hit indices by system type
  std::map<int, std::vector<size_t>> hits_by_system;

  auto* sys_field = m_idSpec.field("system");
  if (!sys_field) {
    error("Field 'system' not found in IDSpec for readout {}", m_cfg.readout);
    return;
  }

  for (size_t i = 0; i < hits->size(); ++i) {
    int sys = sys_field->value((*hits)[i].getCellID());
    hits_by_system[sys].push_back(i);
  }

  // for cluster info
  std::vector<std::vector<size_t>> all_clusters;
  std::map<int, std::vector<size_t>> clusters_by_system;

  // Loop over systems
  for (auto& [sys, hit_indices] : hits_by_system) {

    debug("Processing system {} with {} hits", sys, hit_indices.size());

    // Sort hit indices (podio collections do not support std::sort)
    auto compare = [&hits](const auto& a, const auto& b) {
      // if !(a < b) and !(b < a), then a and b are equivalent
      // and only one of them will be allowed in a set
      const auto& ha = (*hits)[a];
      const auto& hb = (*hits)[b];

      if (ha.getObjectID().collectionID != hb.getObjectID().collectionID)
        return ha.getObjectID().collectionID < hb.getObjectID().collectionID;
      if (ha.getLayer() != hb.getLayer())
        return ha.getLayer() < hb.getLayer();
      return ha.getObjectID().index < hb.getObjectID().index;
    };

    // indices contains the remaining hit indices that have not
    // been assigned to a group yet
    std::set<size_t, decltype(compare)> indices(compare);

    // set does not have a size yet, so cannot fill with iota
    for (auto i : hit_indices) {
      indices.insert(i);
    }

    if (hit_indices.size() != indices.size()) {
      error("System {}: equivalent hits were dropped: #hits {:d}, #indices {:d}", sys,
            hit_indices.size(), indices.size());
      for (auto i : hit_indices) {
        const auto& h = (*hits)[i];
        error("  dropped hit {}: cellID=0x{:x}, layer={}, index={}", i, h.getCellID(), h.getLayer(),
              h.getObjectID().index);
      }
    }

    // Group neighboring hits
    std::vector<std::list<size_t>> groups;
    std::vector<std::vector<std::pair<size_t, size_t>>> group_edges; //for is_neighbour

    // because indices changes, the loop over indices requires some care:
    // - we must use iterators instead of range-for
    // - erase returns an incremented iterator and therefore acts as idx++
    // - when the set becomes empty on erase, idx is invalid and idx++ will be too
    // (also applies to loop in bfs_group below)

    for (auto idx = indices.begin(); idx != indices.end();
         indices.empty() ? idx = indices.end() : idx) {

      trace("hit {:d}: local position = ({}, {}, {}), global position = ({}, {}, {}), energy = {}",
            *idx, (*hits)[*idx].getLocal().x, (*hits)[*idx].getLocal().y,
            (*hits)[*idx].getLocal().z, (*hits)[*idx].getPosition().x,
            (*hits)[*idx].getPosition().y, (*hits)[*idx].getPosition().z,
            (*hits)[*idx].getEnergy());

      // not energetic enough for cluster center, but could still be cluster hit
      if ((*hits)[*idx].getEnergy() < minClusterCenterEdep) {
        idx++;
        continue;
      }

      // create a new group, and group all the neighbouring hits
      groups.emplace_back(std::list{*idx});
      group_edges.emplace_back(); // create matching edges vector
      bfs_group(*hits, indices, groups.back(), group_edges.back(), *idx);

      // wait with erasing until after bfs_group to ensure iterator is not invalidated in bfs_group
      idx = indices.erase(idx); // takes role of idx++
    }

    debug("found {} potential clusters (groups of hits)", groups.size());
    for (std::size_t i = 0; i < groups.size(); ++i) {
      debug("group {}: {} hits", i, groups[i].size());
      for (auto idx : groups[i]) {
        const auto& hit = (*hits)[idx];
        // debug("  hit {} -> energy = {:.6f}, layer = {}, sector = {}, local = ({:.2f}, {:.2f}, "
        //       "{:.2f}), global = ({:.2f}, {:.2f}, {:.2f})",
        //       idx, hit.getEnergy(), hit.getLayer(), hit.getSector(), hit.getLocal().x,
        //       hit.getLocal().y, hit.getLocal().z, hit.getPosition().x, hit.getPosition().y,
        //       hit.getPosition().z);
        debug("T{}_{} [label=\"hit {}\", fillcolor=lightcoral, group=T{}];", i, idx, idx, i);
      }
      for (auto& e : group_edges[i]) {
        debug("T{}_{} -- T{}_{};", i, e.first, i, e.second);
      }
    }

    // form clusters
    for (const auto& group : groups) {
      if (group.size() < m_cfg.minClusterNhits)
        continue;

      double energy = 0.;
      for (auto idx : group)
        energy += (*hits)[idx].getEnergy();
      if (energy < minClusterEdep)
        continue;

      clusters_by_system[sys].push_back(all_clusters.size());
      all_clusters.emplace_back(group.begin(), group.end());
    }
  }

  // mergeCrossSystemClusters
  std::vector<std::vector<size_t>> final_clusters;
  std::set<size_t> used_Img;
  std::set<size_t> used_ScFi;

  // Track cross-system neighbor pairs for visualization
  std::set<std::pair<size_t, size_t>> cross_system_neighbor_pairs;

  bool have_Img  = clusters_by_system.count(101);
  bool have_ScFi = clusters_by_system.count(105);

  if (have_Img && have_ScFi) {

    const auto& Img  = clusters_by_system.at(101);
    const auto& ScFi = clusters_by_system.at(105);

    debug("Performing simple cross-system merging: {} Img clusters, {} ScFi clusters", Img.size(),
          ScFi.size());

    // Loop over Imaging clusters
    for (size_t idx_Img : Img) {

      if (used_Img.count(idx_Img))
        continue;

      const auto& cl_Img         = all_clusters[idx_Img];
      std::vector<size_t> merged = cl_Img;
      used_Img.insert(idx_Img);

      // Check against all ScFi clusters
      for (size_t idx_ScFi : ScFi) {

        const auto& cl_ScFi = all_clusters[idx_ScFi];
        bool neighbor_found = false;

        for (auto h1 : cl_Img) {
          for (auto h2 : cl_ScFi) {
            if (cross_system_is_neighbour((*hits)[h1], (*hits)[h2])) {
              debug("  Cross system neighbour: Img cluster {} hit {}  <->  ScFi cluster {} hit {}",
                    idx_Img, h1, idx_ScFi, h2);
              // Track this cross-system neighbor pair
              size_t min_h = std::min(h1, h2);
              size_t max_h = std::max(h1, h2);
              cross_system_neighbor_pairs.insert({min_h, max_h});
              neighbor_found = true;
              break;
            }
          }
          if (neighbor_found)
            break;
        }

        if (neighbor_found) {
          merged.insert(merged.end(), cl_ScFi.begin(), cl_ScFi.end());
          used_ScFi.insert(idx_ScFi);

          // merge any other Imaging clusters neighbour to ScFi
          for (size_t idx2_Img : Img) {
            if (used_Img.count(idx2_Img))
              continue;

            const auto& cl2_Img   = all_clusters[idx2_Img];
            bool second_neighbour = false;

            for (auto h1 : cl2_Img) {
              for (auto h2 : cl_ScFi) {
                if (cross_system_is_neighbour((*hits)[h1], (*hits)[h2])) {
                  debug("  second Neighbour: Img cluster {} hit {} is neighbour to same ScFi "
                        "cluster {} via hit {}",
                        idx2_Img, h1, idx_ScFi, h2);
                  // Track this cross-system neighbor pair
                  size_t min_h = std::min(h1, h2);
                  size_t max_h = std::max(h1, h2);
                  cross_system_neighbor_pairs.insert({min_h, max_h});
                  second_neighbour = true;
                  break;
                }
              }
              if (second_neighbour)
                break;
            }

            if (second_neighbour) {
              merged.insert(merged.end(), cl2_Img.begin(), cl2_Img.end());
              used_Img.insert(idx2_Img);
            }
          }
        }
      }

      final_clusters.push_back(std::move(merged));
    }

    // Add ScFi clusters that are not a part of any other cluster
    for (size_t idx_ScFi : ScFi) {
      if (!used_ScFi.count(idx_ScFi)) {
        final_clusters.push_back(all_clusters[idx_ScFi]);
      }
    }
  } else {
    // If only one subsystem exists : no merging
    final_clusters = all_clusters;
  }

  debug("Outputting {} final proto-clusters", final_clusters.size());

  // Write output
  for (auto& cl : final_clusters) {
    auto pcl = proto->create();
    for (auto idx : cl) {
      pcl.addToHits((*hits)[idx]);
      pcl.addToWeights(1);
    }
  }
}

bool ImagingTopoCluster::cross_system_is_neighbour(const edm4eic::CalorimeterHit& h1,
                                                   const edm4eic::CalorimeterHit& h2) const {

  // Get the "system" field from the ID specification
  auto* sys_field = m_idSpec.field("system");
  if (!sys_field) {
    error("Field 'system' not found in IDSpec for readout {}", m_cfg.readout);
    return false;
  }
  // Extract system IDs for both hits
  int sys1 = sys_field->value(h1.getCellID());
  int sys2 = sys_field->value(h2.getCellID());

  // If different systems, allow Imaging(101) <-> ScFi(105) cross-linking
  if (sys1 != sys2) {
    if (h1.getSector() != h2.getSector()) {
      return std::hypot((h1.getPosition().x - h2.getPosition().x),
                        (h1.getPosition().y - h2.getPosition().y),
                        (h1.getPosition().z - h2.getPosition().z)) <= cross_system_sectorDist;
    } else {
      return (std::abs(h1.getPosition().x - h2.getPosition().x) <= cross_system_DistXYZ[0]) &&
             (std::abs(h1.getPosition().y - h2.getPosition().y) <= cross_system_DistXYZ[1]) &&
             (std::abs(h1.getPosition().z - h2.getPosition().z) <= cross_system_DistXYZ[2]);
    }
  }
  return false;
}

// helper function to group hits
bool ImagingTopoCluster::is_neighbour(const edm4eic::CalorimeterHit& h1,
                                      const edm4eic::CalorimeterHit& h2) const {

  // Get the "system" field from the ID specification
  auto* sys_field = m_idSpec.field("system");
  if (!sys_field) {
    error("Field 'system' not found in IDSpec for readout {}", m_cfg.readout);
    return false;
  }
  // Extract system IDs for both hits
  int sys1 = sys_field->value(h1.getCellID());
  int sys2 = sys_field->value(h2.getCellID());

  if (sys1 != sys2) {
    return false;
  }

  // Same-system logic

  //ScFi
  if (sys1 == 105) {
    // different sectors,distance check
    if (h1.getSector() != h2.getSector()) {
      return std::hypot((h1.getPosition().x - h2.getPosition().x),
                        (h1.getPosition().y - h2.getPosition().y),
                        (h1.getPosition().z - h2.getPosition().z)) <= ScFi_sectorDist;
    }

    int ldiff = std::abs(h1.getLayer() - h2.getLayer());

    if (ldiff == 0) {
      switch (m_cfg.ScFi_sameLayerMode) {
      case ImagingTopoClusterConfig::ELayerMode::xy:
        return (std::abs(h1.getLocal().x - h2.getLocal().x) <= ScFi_sameLayerDistXY[0]) &&
               (std::abs(h1.getLocal().y - h2.getLocal().y) <= ScFi_sameLayerDistXY[1]);
      case ImagingTopoClusterConfig::ELayerMode::xyz:
        return (std::abs(h1.getLocal().x - h2.getLocal().x) <= ScFi_sameLayerDistXYZ[0]) &&
               (std::abs(h1.getLocal().y - h2.getLocal().y) <= ScFi_sameLayerDistXYZ[1]) &&
               (std::abs(h1.getLocal().z - h2.getLocal().z) <= ScFi_sameLayerDistXYZ[2]);
      case ImagingTopoClusterConfig::ELayerMode::etaphi:
        return (std::abs(edm4hep::utils::eta(h1.getPosition()) - 
                edm4hep::utils::eta(h2.getPosition())) <= ScFi_sameLayerDistEtaPhi[0]) &&
               (std::abs(edm4hep::utils::angleAzimuthal(h1.getPosition()) -                          
               edm4hep::utils::angleAzimuthal(h2.getPosition()))<= ScFi_sameLayerDistEtaPhi[1]);
      case ImagingTopoClusterConfig::ELayerMode::tz: {
        auto phi  = 0.5 * (edm4hep::utils::angleAzimuthal(h1.getPosition()) +
                          edm4hep::utils::angleAzimuthal(h2.getPosition()));
        auto h1_t = (h1.getPosition().x * sin(phi)) - (h1.getPosition().y * cos(phi));
        auto h2_t = (h2.getPosition().x * sin(phi)) - (h2.getPosition().y * cos(phi));
        auto h1_z = h1.getPosition().z;
        auto h2_z = h2.getPosition().z;
        return (std::abs(h1_t - h2_t) <= ScFi_sameLayerDistTZ[0]) &&
               (std::abs(h1_z - h2_z) <= ScFi_sameLayerDistTZ[1]);
      }
      default:
        error("Unknown layer mode for same-layer clustering.");
        return false;
      }
    } else if (ldiff <= m_cfg.neighbourLayersRange) {
      switch (m_cfg.ScFi_diffLayerMode) {
      case eicrecon::ImagingTopoClusterConfig::ELayerMode::etaphi:
        return (std::abs(edm4hep::utils::eta(h1.getPosition()) -                          
               edm4hep::utils::eta(h2.getPosition())) <= ScFi_diffLayerDistEtaPhi[0]) &&
               (std::abs(edm4hep::utils::angleAzimuthal(h1.getPosition()) -                          
               edm4hep::utils::angleAzimuthal(h2.getPosition()))<= ScFi_diffLayerDistEtaPhi[1]);
      case eicrecon::ImagingTopoClusterConfig::ELayerMode::xy:
        return (std::abs(h1.getPosition().x - h2.getPosition().x) <= ScFi_diffLayerDistXY[0]) &&
               (std::abs(h1.getPosition().y - h2.getPosition().y) <= ScFi_diffLayerDistXY[1]);
      case ImagingTopoClusterConfig::ELayerMode::xyz:
        return (std::abs(h1.getPosition().x - h2.getPosition().x) <= ScFi_diffLayerDistXYZ[0]) &&
               (std::abs(h1.getPosition().y - h2.getPosition().y) <= ScFi_diffLayerDistXYZ[1]) &&
               (std::abs(h1.getPosition().z - h2.getPosition().z) <= ScFi_diffLayerDistXYZ[2]);
      case eicrecon::ImagingTopoClusterConfig::ELayerMode::tz: {
        auto phi  = 0.5 * (edm4hep::utils::angleAzimuthal(h1.getPosition()) +
                          edm4hep::utils::angleAzimuthal(h2.getPosition()));
        auto h1_t = (h1.getPosition().x * sin(phi)) - (h1.getPosition().y * cos(phi));
        auto h2_t = (h2.getPosition().x * sin(phi)) - (h2.getPosition().y * cos(phi));
        auto h1_z = h1.getPosition().z;
        auto h2_z = h2.getPosition().z;
        return (std::abs(h1_t - h2_t) <= ScFi_diffLayerDistTZ[0]) &&
               (std::abs(h1_z - h2_z) <= ScFi_diffLayerDistTZ[1]);
      }
      default:
        error("Unknown layer mode for same-layer clustering.");
        return false;
      }
    }
  } // sys1 == 105

  // Imaging
  else if (sys1 == 101) {
    // different sectors, simple distance check
    if (h1.getSector() != h2.getSector()) {
      return std::hypot((h1.getPosition().x - h2.getPosition().x),
                        (h1.getPosition().y - h2.getPosition().y),
                        (h1.getPosition().z - h2.getPosition().z)) <= Img_sectorDist;
    }

    int ldiff = std::abs(h1.getLayer() - h2.getLayer());

    if (ldiff == 0) {
      switch (m_cfg.Img_sameLayerMode) {
      case ImagingTopoClusterConfig::ELayerMode::xy:
        return (std::abs(h1.getLocal().x - h2.getLocal().x) <= Img_sameLayerDistXY[0]) &&
               (std::abs(h1.getLocal().y - h2.getLocal().y) <= Img_sameLayerDistXY[1]);
      case ImagingTopoClusterConfig::ELayerMode::xyz:
        return (std::abs(h1.getLocal().x - h2.getLocal().x) <= Img_sameLayerDistXYZ[0]) &&
               (std::abs(h1.getLocal().y - h2.getLocal().y) <= Img_sameLayerDistXYZ[1]) &&
               (std::abs(h1.getLocal().z - h2.getLocal().z) <= Img_sameLayerDistXYZ[2]);
      case ImagingTopoClusterConfig::ELayerMode::etaphi:
        return (std::abs(edm4hep::utils::eta(h1.getPosition()) -                          
               edm4hep::utils::eta(h2.getPosition())) <= Img_sameLayerDistEtaPhi[0]) &&
               (std::abs(edm4hep::utils::angleAzimuthal(h1.getPosition()) -                         
               edm4hep::utils::angleAzimuthal(h2.getPosition()))<= Img_sameLayerDistEtaPhi[1]);
      case ImagingTopoClusterConfig::ELayerMode::tz: {
        auto phi  = 0.5 * (edm4hep::utils::angleAzimuthal(h1.getPosition()) +
                          edm4hep::utils::angleAzimuthal(h2.getPosition()));
        auto h1_t = (h1.getPosition().x * sin(phi)) - (h1.getPosition().y * cos(phi));
        auto h2_t = (h2.getPosition().x * sin(phi)) - (h2.getPosition().y * cos(phi));
        auto h1_z = h1.getPosition().z;
        auto h2_z = h2.getPosition().z;
        return (std::abs(h1_t - h2_t) <= Img_sameLayerDistTZ[0]) &&
               (std::abs(h1_z - h2_z) <= Img_sameLayerDistTZ[1]);
      }
      default:
        error("Unknown layer mode for same-layer clustering.");
        return false;
      }
    } else if (ldiff <= m_cfg.neighbourLayersRange) {
      switch (m_cfg.Img_diffLayerMode) {
      case eicrecon::ImagingTopoClusterConfig::ELayerMode::etaphi:
        return (std::abs(edm4hep::utils::eta(h1.getPosition()) -
                         edm4hep::utils::eta(h2.getPosition())) <= Img_diffLayerDistEtaPhi[0]) &&
               (std::abs(edm4hep::utils::angleAzimuthal(h1.getPosition()) -
                         edm4hep::utils::angleAzimuthal(h2.getPosition()))  <= Img_diffLayerDistEtaPhi[1]);
      case eicrecon::ImagingTopoClusterConfig::ELayerMode::xy:
        return (std::abs(h1.getPosition().x - h2.getPosition().x) <= Img_diffLayerDistXY[0]) &&
               (std::abs(h1.getPosition().y - h2.getPosition().y) <= Img_diffLayerDistXY[1]);
      case ImagingTopoClusterConfig::ELayerMode::xyz:
        return (std::abs(h1.getPosition().x - h2.getPosition().x) <= Img_diffLayerDistXYZ[0]) &&
               (std::abs(h1.getPosition().y - h2.getPosition().y) <= Img_diffLayerDistXYZ[1]) &&
               (std::abs(h1.getPosition().z - h2.getPosition().z) <= Img_diffLayerDistXYZ[2]);
      case eicrecon::ImagingTopoClusterConfig::ELayerMode::tz: {
        auto phi  = 0.5 * (edm4hep::utils::angleAzimuthal(h1.getPosition()) +
                          edm4hep::utils::angleAzimuthal(h2.getPosition()));
        auto h1_t = (h1.getPosition().x * sin(phi)) - (h1.getPosition().y * cos(phi));
        auto h2_t = (h2.getPosition().x * sin(phi)) - (h2.getPosition().y * cos(phi));
        auto h1_z = h1.getPosition().z;
        auto h2_z = h2.getPosition().z;
        return (std::abs(h1_t - h2_t) <= Img_diffLayerDistTZ[0]) &&
               (std::abs(h1_z - h2_z) <= Img_diffLayerDistTZ[1]);
      }
      default:
        error("Hits are not neighbors: idx1 = {}, idx2 = {}, layer1 = {}, layer2 = {}, ldiff = {}",
              h1.getObjectID().index, h2.getObjectID().index, h1.getLayer(), h2.getLayer(), ldiff);
        return false;
      }
    }
  } // sys1 == 101

  // any other system other than ScFi and Img
  else {

    // different sectors, simple distance check
    if (h1.getSector() != h2.getSector()) {
      return std::hypot((h1.getPosition().x - h2.getPosition().x),
                        (h1.getPosition().y - h2.getPosition().y),
                        (h1.getPosition().z - h2.getPosition().z)) <= sectorDist;
    }

    // layer check
    int ldiff = std::abs(h1.getLayer() - h2.getLayer());
    
    if (ldiff == 0) {
      switch (m_cfg.sameLayerMode) {
      case ImagingTopoClusterConfig::ELayerMode::xy:
        return (std::abs(h1.getLocal().x - h2.getLocal().x) <= sameLayerDistXY[0]) &&
               (std::abs(h1.getLocal().y - h2.getLocal().y) <= sameLayerDistXY[1]);
      case ImagingTopoClusterConfig::ELayerMode::xyz:
        return (std::abs(h1.getLocal().x - h2.getLocal().x) <= sameLayerDistXYZ[0]) &&
               (std::abs(h1.getLocal().y - h2.getLocal().y) <= sameLayerDistXYZ[1]) &&
               (std::abs(h1.getLocal().z - h2.getLocal().z) <= sameLayerDistXYZ[2]);
      case ImagingTopoClusterConfig::ELayerMode::etaphi:
        return (std::abs(edm4hep::utils::eta(h1.getPosition()) -
                         edm4hep::utils::eta(h2.getPosition())) <= sameLayerDistEtaPhi[0]) &&
               (std::abs(edm4hep::utils::angleAzimuthal(h1.getPosition()) -
                         edm4hep::utils::angleAzimuthal(h2.getPosition())) <=
                sameLayerDistEtaPhi[1]);
      case ImagingTopoClusterConfig::ELayerMode::tz: {
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
                         edm4hep::utils::angleAzimuthal(h2.getPosition())) <=
                diffLayerDistEtaPhi[1]);
      case eicrecon::ImagingTopoClusterConfig::ELayerMode::xy:
        return (std::abs(h1.getPosition().x - h2.getPosition().x) <= diffLayerDistXY[0]) &&
               (std::abs(h1.getPosition().y - h2.getPosition().y) <= diffLayerDistXY[1]);
      case ImagingTopoClusterConfig::ELayerMode::xyz:
        return (std::abs(h1.getPosition().x - h2.getPosition().x) <= diffLayerDistXYZ[0]) &&
               (std::abs(h1.getPosition().y - h2.getPosition().y) <= diffLayerDistXYZ[1]) &&
               (std::abs(h1.getPosition().z - h2.getPosition().z) <= diffLayerDistXYZ[2]);
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
  } // any other system

  return false;
}

} // namespace eicrecon
