// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Dmitry Kalinkin

#include <fmt/format.h>
#include <cstddef>
#include <gsl/pointers>
#include <stdexcept>

#include "CalorimeterParticleIDPostML.h"

namespace eicrecon {

void CalorimeterParticleIDPostML::init() {
  // Nothing
}

void CalorimeterParticleIDPostML::process(const CalorimeterParticleIDPostML::Input& input,
                                          const CalorimeterParticleIDPostML::Output& output) const {

  const auto [in_clusters, in_assocs, prediction_tensors] = input;
  auto [out_clusters, out_assocs, out_particle_ids]       = output;

  if (prediction_tensors->size() != 1) {
    error("Expected to find a single tensor, found {}", prediction_tensors->size());
    throw std::runtime_error("");
  }
  edm4eic::Tensor prediction_tensor = (*prediction_tensors)[0];

  if (prediction_tensor.shape_size() != 2) {
    error("Expected tensor rank to be 2, but it is {}", prediction_tensor.shape_size());
    throw std::runtime_error(
        fmt::format("Expected tensor rank to be 2, but it is {}", prediction_tensor.shape_size()));
  }

  if (prediction_tensor.getShape(0) != static_cast<long>(in_clusters->size())) {
    error("Length mismatch between tensor's 0th axis and number of clusters: {} != {}",
          prediction_tensor.getShape(0), in_clusters->size());
    throw std::runtime_error(
        fmt::format("Length mismatch between tensor's 0th axis and number of clusters: {} != {}",
                    prediction_tensor.getShape(0), in_clusters->size()));
  }

  if (prediction_tensor.getShape(1) != 2) {
    error("Expected 2 values per cluster in the output tensor, got {}",
          prediction_tensor.getShape(0));
    throw std::runtime_error(
        fmt::format("Expected 2 values per cluster in the output tensor, got {}",
                    prediction_tensor.getShape(0)));
  }

  if (prediction_tensor.getElementType() != 1) { // 1 - float
    error("Expected a tensor of floats, but element type is {}",
          prediction_tensor.getElementType());
    throw std::runtime_error(fmt::format("Expected a tensor of floats, but element type is {}",
                                         prediction_tensor.getElementType()));
  }

  for (std::size_t cluster_ix = 0; cluster_ix < in_clusters->size(); cluster_ix++) {
    edm4eic::Cluster in_cluster         = (*in_clusters)[cluster_ix];
    edm4eic::MutableCluster out_cluster = in_cluster.clone();
    out_clusters->push_back(out_cluster);

    float prob_pion =
        prediction_tensor.getFloatData(cluster_ix * prediction_tensor.getShape(1) + 0);
    float prob_electron =
        prediction_tensor.getFloatData(cluster_ix * prediction_tensor.getShape(1) + 1);

    out_cluster.addToParticleIDs(out_particle_ids->create(0,        // std::int32_t type
                                                          211,      // std::int32_t PDG
                                                          0,        // std::int32_t algorithmType
                                                          prob_pion // float likelihood
                                                          ));
    out_cluster.addToParticleIDs(out_particle_ids->create(0,  // std::int32_t type
                                                          11, // std::int32_t PDG
                                                          0,  // std::int32_t algorithmType
                                                          prob_electron // float likelihood
                                                          ));

    // propagate associations
    for (auto in_assoc : *in_assocs) {
      if (in_assoc.getRec() == in_cluster) {
        auto out_assoc = in_assoc.clone();
        out_assoc.setRec(out_cluster);
        out_assocs->push_back(out_assoc);
      }
    }
  }
}

} // namespace eicrecon
