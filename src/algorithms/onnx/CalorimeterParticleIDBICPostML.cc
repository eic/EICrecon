// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Tomas Sosa, Wouter Deconinck

#include "CalorimeterParticleIDBICPostML.h"

#include <cstddef>
#include <fmt/core.h>
#include <stdexcept>
#include <vector>

namespace eicrecon {

namespace {

  bool hasElectronPID(const edm4eic::Cluster& cl) {
    for (auto const& pid : cl.getParticleIDs()) {
      if (pid.getPDG() == 11) {
        return true;
      }
    }
    return false;
  }

} // namespace

void CalorimeterParticleIDBICPostML::init() {
  // Nothing
}

void CalorimeterParticleIDBICPostML::process(
    const CalorimeterParticleIDBICPostML::Input& input,
    const CalorimeterParticleIDBICPostML::Output& output) const {

  const auto [in_clusters, in_matches, in_ep_pids, prediction_tensors] = input;
  auto [out_clusters, out_matches, out_particle_ids]                   = output;
  (void)in_ep_pids;

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

  std::vector<std::size_t> selected;
  selected.reserve(in_clusters->size());
  for (std::size_t i = 0; i < in_clusters->size(); ++i) {
    if (hasElectronPID((*in_clusters)[i])) {
      selected.push_back(i);
    }
  }

  if (prediction_tensor.getShape(0) != static_cast<long>(selected.size())) {
    error("Prediction rows ({}) do not match selected ScFi clusters ({})",
          prediction_tensor.getShape(0), selected.size());
    throw std::runtime_error(
        fmt::format("Prediction rows ({}) do not match selected ScFi clusters ({})",
                    prediction_tensor.getShape(0), selected.size()));
  }

  std::size_t j = 0;
  for (std::size_t i = 0; i < in_clusters->size(); ++i) {
    const auto in_cluster = (*in_clusters)[i];
    auto out_cluster      = in_cluster.clone();
    out_clusters->push_back(out_cluster);

    for (auto const& m : *in_matches) {
      if (m.getCluster() == in_cluster) {
        auto out_match = m.clone();
        out_match.setCluster(out_cluster);
        out_matches->push_back(out_match);
      }
    }

    if (!(j < selected.size() && selected[j] == i)) {
      continue;
    }

    const float probPion = prediction_tensor.getFloatData(j * prediction_tensor.getShape(1) + 0);
    const float probElectron =
        prediction_tensor.getFloatData(j * prediction_tensor.getShape(1) + 1);

    out_cluster.addToParticleIDs(out_particle_ids->create(0,    // type
                                                          -211, // PDG: pi-
                                                          0,    // algorithmType
                                                          probPion));

    out_cluster.addToParticleIDs(out_particle_ids->create(0,  // type
                                                          11, // PDG: e-
                                                          0,  // algorithmType
                                                          probElectron));

    ++j;
  }
}

} // namespace eicrecon
