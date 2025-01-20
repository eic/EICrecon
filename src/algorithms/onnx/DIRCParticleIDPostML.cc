// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Dmitry Kalinkin

#include <edm4eic/EDM4eicVersion.h>

#if EDM4EIC_VERSION_MAJOR >= 8
#include <cstddef>
#include <fmt/core.h>
#include <gsl/pointers>
#include <stdexcept>

#include "DIRCParticleIDPostML.h"

namespace eicrecon {

  void DIRCParticleIDPostML::init() {
    // Nothing
  }

  void DIRCParticleIDPostML::process(
      const DIRCParticleIDPostML::Input& input,
      const DIRCParticleIDPostML::Output& output) const {

    const auto [in_particles, in_assocs, prediction_tensors] = input;
    auto [out_particles, out_assocs, out_particle_ids] = output;

    if (prediction_tensors->size() != 1) {
      error("Expected to find a single tensor, found {}", prediction_tensors->size());
      throw std::runtime_error("");
    }
    edm4eic::Tensor prediction_tensor = (*prediction_tensors)[0];

    if (prediction_tensor.shape_size() != 1) {
      error("Expected tensor rank to be 1, but it is {}", prediction_tensor.shape_size());
      throw std::runtime_error(fmt::format("Expected tensor rank to be 1, but it is {}", prediction_tensor.shape_size()));
    }

    /*
    if (prediction_tensor.getShape(0) != in_particles->size()) {
      error("Length mismatch between tensor's 0th axis and number of particles: {} != {}", prediction_tensor.getShape(0), in_particles->size());
      throw std::runtime_error(fmt::format("Length mismatch between tensor's 0th axis and number of particles: {} != {}", prediction_tensor.getShape(0), in_particles->size()));
    }
    */

    if (prediction_tensor.getShape(0) != 2) {
      error("Expected 2 values per particle in the output tensor, got {}", prediction_tensor.getShape(0));
      throw std::runtime_error(fmt::format("Expected 2 values per particle in the output tensor, got {}", prediction_tensor.getShape(0)));
    }

    if (prediction_tensor.getElementType() != 1) { // 1 - float
      error("Expected a tensor of floats, but element type is {}", prediction_tensor.getElementType());
      throw std::runtime_error(fmt::format("Expected a tensor of floats, but element type is {}", prediction_tensor.getElementType()));
    }

    for (size_t particle_ix = 0; particle_ix < 1; particle_ix++) {
      edm4eic::ReconstructedParticle in_particle = (*in_particles)[particle_ix];
      edm4eic::MutableReconstructedParticle out_particle = in_particle.clone();
      out_particles->push_back(out_particle);

      float prob_pion = prediction_tensor.getFloatData(/* particle_ix * prediction_tensor.getShape(1) + */0);
      float prob_kaon = prediction_tensor.getFloatData(/* particle_ix * prediction_tensor.getShape(1) + */1);

      if (prob_pion > prob_kaon) {
        out_particle.setPDG(211);
      } else {
        out_particle.setPDG(321);
      }
      out_particle.addToParticleIDs(out_particle_ids->create(
        0,        // std::int32_t type
        211,      // std::int32_t PDG
        0,        // std::int32_t algorithmType
        prob_pion // float likelihood
      ));
      out_particle.addToParticleIDs(out_particle_ids->create(
        0,        // std::int32_t type
        321,      // std::int32_t PDG
        0,        // std::int32_t algorithmType
        prob_kaon // float likelihood
      ));

      // propagate associations
      for (auto in_assoc : *in_assocs) {
        if (in_assoc.getRec() == in_particle) {
          auto out_assoc = in_assoc.clone();
          out_assoc.setRec(out_particle);
          out_assocs->push_back(out_assoc);
        }
      }
    }
  }

} // namespace eicrecon
#endif
