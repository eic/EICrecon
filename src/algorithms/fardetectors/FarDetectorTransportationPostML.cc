// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 - 2025 Simon Gardner

#include <edm4eic/EDM4eicVersion.h>
#include <edm4hep/Vector3f.h>

#if EDM4EIC_VERSION_MAJOR >= 8
#include <fmt/core.h>
#include <podio/RelationRange.h>
#include <cmath>
#include <cstddef>
#include <gsl/pointers>
#include <stdexcept>

#include "FarDetectorTransportationPostML.h"

namespace eicrecon {

void FarDetectorTransportationPostML::init() { m_beamE = m_cfg.beamE; }

void FarDetectorTransportationPostML::process(
    const FarDetectorTransportationPostML::Input& input,
    const FarDetectorTransportationPostML::Output& output) const {

  const auto [prediction_tensors, beamElectrons] = input;
  auto [out_particles]                           = output;

  //Set beam energy from first MCBeamElectron, using std::call_once
  if (beamElectrons != nullptr) {
    std::call_once(m_initBeamE, [&]() {
      // Check if beam electrons are present
      if (beamElectrons->empty()) { // NOLINT(clang-analyzer-core.NullDereference)
        if (m_cfg.requireBeamElectron) {
          critical("No beam electrons found");
          throw std::runtime_error("No beam electrons found");
        }
        return;
      }
      m_beamE = beamElectrons->at(0).getEnergy();
      //Round beam energy to nearest GeV - Should be 5, 10 or 18GeV
      m_beamE = round(m_beamE);
    });
  }

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

  if (prediction_tensor.getShape(1) != 3) {
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

  auto prediction_tensor_data = prediction_tensor.getFloatData();

  // Ensure the size of prediction_tensor_data is a multiple of its shape
  if (prediction_tensor_data.size() % 3 != 0) {
    error("The size of prediction_tensor_data is not a multiple of 3.");
    throw std::runtime_error("The size of prediction_tensor_data is not a multiple of 3.");
  }

  edm4eic::MutableReconstructedParticle particle;

  // Iterate over the prediction_tensor_data in steps of three
  for (std::size_t i = 0; i < prediction_tensor_data.size(); i += 3) {
    if (i + 2 >= prediction_tensor_data.size()) {
      error("Incomplete data for a prediction tensor at the end of the vector.");
      throw std::runtime_error("Incomplete data for a prediction tensor at the end of the vector.");
    }

    // Extract the current prediction
    float px = prediction_tensor_data[i] * m_beamE;
    float py = prediction_tensor_data[i + 1] * m_beamE;
    float pz = prediction_tensor_data[i + 2] * m_beamE;

    // Calculate reconstructed electron energy
    double energy = sqrt(px * px + py * py + pz * pz + 0.000511 * 0.000511);

    particle = out_particles->create();

    particle.setEnergy(energy);
    particle.setMomentum({px, py, pz});
    particle.setCharge(-1);
    particle.setMass(0.000511);
    particle.setPDG(11);
  }

  // TODO: Implement the association of the reconstructed particles with the tracks
}

} // namespace eicrecon
#endif
