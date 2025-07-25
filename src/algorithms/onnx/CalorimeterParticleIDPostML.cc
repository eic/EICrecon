// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Dmitry Kalinkin

#include <edm4eic/EDM4eicVersion.h>

#if EDM4EIC_VERSION_MAJOR >= 8
#include <cstddef>
#include <fmt/core.h>
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
    throw std::runtime_error("Bad prediction tensor count");
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
          prediction_tensor.getShape(1));
    throw std::runtime_error(
        fmt::format("Expected 2 values per cluster in the output tensor, got {}",
                    prediction_tensor.getShape(1)));
  }

  if (prediction_tensor.getElementType() != 1) { // 1 - float
    error("Expected a tensor of floats, but element type is {}",
          prediction_tensor.getElementType());
    throw std::runtime_error(fmt::format("Expected a tensor of floats, but element type is {}",
                                         prediction_tensor.getElementType()));
  }

  std::vector<std::size_t> selIdx;
  selIdx.reserve(in_clusters->size());
  for (std::size_t i = 0; i < in_clusters->size(); ++i) {
    auto const& pvec = (*in_clusters)[i].getParticleIDs();
    for (auto const& pid : pvec) {
      if (pid.getPDG() == 11) {
        selIdx.push_back(i);
        break;
      }
    }
  }
  if (prediction_tensor.getShape(0) != static_cast<long>(selIdx.size())) {
    error("Mismatch between tensor rows ({}) and selected clusters ({})",
          prediction_tensor.getShape(0), selIdx.size());
    throw std::runtime_error("Prediction vs selClusters mismatch");
  }

  std::size_t j = 0;
  const auto N  = in_clusters->size();
  for (std::size_t idx = 0; idx < N; ++idx) {
    auto in_cl  = (*in_clusters)[idx];
    auto out_cl = in_cl.clone();
    out_clusters->push_back(out_cl);

    if (in_assocs) {
      for (auto const& a : *in_assocs) {
        if (a.getRec() == in_cl) {
          auto oa = a.clone();
          oa.setRec(out_cl);
          out_assocs->push_back(oa);
        }
      }
    }

    if (j < selIdx.size() && selIdx[j] == idx) {
      float prob_pion     = prediction_tensor.getFloatData(j * 2 + 0);
      float prob_electron = prediction_tensor.getFloatData(j * 2 + 1);

      out_cl.addToParticleIDs(out_particle_ids->create(0,   // type
                                                       211, // PDG π
                                                       0,   // algo
                                                       prob_pion));
      out_cl.addToParticleIDs(out_particle_ids->create(0,  // type
                                                       11, // PDG e
                                                       0,  // algo
                                                       prob_electron));

      ++j;
    }
  }
}

} // namespace eicrecon
#endif
