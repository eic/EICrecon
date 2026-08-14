// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Tomas Sosa, Wouter Deconinck

#include "CalorimeterParticleIDBICPostML.h"

#include <cstddef>
#include <fmt/core.h>
#include <stdexcept>
#include <vector>

namespace eicrecon {

void CalorimeterParticleIDBICPostML::init() {
  // Nothing
}

void CalorimeterParticleIDBICPostML::process(
    const CalorimeterParticleIDBICPostML::Input& input,
    const CalorimeterParticleIDBICPostML::Output& output) const {

  const auto [in_clusters, prediction_tensors] = input;
  auto [out_clusters, out_particle_ids]        = output;

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

  if (prediction_tensor.getShape(0) != static_cast<long>(in_clusters->size())) {
    error("Prediction rows ({}) do not match E/p-selected merged BIC clusters ({})",
          prediction_tensor.getShape(0), in_clusters->size());
    throw std::runtime_error(
        fmt::format("Prediction rows ({}) do not match E/p-selected merged BIC clusters ({})",
                    prediction_tensor.getShape(0), in_clusters->size()));
  }

  for (std::size_t i = 0; i < in_clusters->size(); ++i) {
    const auto in_cluster = (*in_clusters)[i];
    auto out_cluster      = in_cluster.clone();
    out_clusters->push_back(out_cluster);
    const float probPion = prediction_tensor.getFloatData(i * prediction_tensor.getShape(1) + 0);
    const float probElectron =
        prediction_tensor.getFloatData(i * prediction_tensor.getShape(1) + 1);

    out_cluster.addToParticleIDs(out_particle_ids->create(0,    // type
                                                          -211, // PDG: pi-
                                                          0,    // algorithmType
                                                          probPion));

    out_cluster.addToParticleIDs(out_particle_ids->create(0,  // type
                                                          11, // PDG: e-
                                                          0,  // algorithmType
                                                          probElectron));

  }
}

} // namespace eicrecon
