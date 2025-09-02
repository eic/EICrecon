// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 - 2025 Simon Gardner

#include <edm4eic/EDM4eicVersion.h>

#if EDM4EIC_VERSION_MAJOR >= 8
#include <edm4hep/Vector3f.h>
#include <fmt/core.h>
#include <podio/RelationRange.h>
#include <cmath>
#include <cstddef>
#include <gsl/pointers>
#include <stdexcept>

#include "FarDetectorTransportationPostML.h"
#include "services/particle/ParticleSvc.h"

namespace eicrecon {

void FarDetectorTransportationPostML::init() {
  m_beamE           = m_cfg.beamE;
  auto& particleSvc = algorithms::ParticleSvc::instance();
  m_mass            = particleSvc.particle(m_cfg.pdg_value).mass;
  m_charge          = particleSvc.particle(m_cfg.pdg_value).charge;
}

void FarDetectorTransportationPostML::process(
    const FarDetectorTransportationPostML::Input& input,
    const FarDetectorTransportationPostML::Output& output) const {

  const auto [prediction_tensors, track_associations, beamElectrons] = input;
  auto [out_particles, out_associations]                             = output;

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
  if (prediction_tensor_data.size() % 3 != 0 || prediction_tensor.getShape(1) != 3) {
    error("The size of prediction_tensor_data is not a multiple of 3.");
    throw std::runtime_error("The size of prediction_tensor_data is not a multiple of 3.");
  }

  edm4eic::MutableReconstructedParticle particle;

  // Iterate over the prediction_tensor_data in steps of three
  for (std::size_t i = 0; i < static_cast<std::size_t>(prediction_tensor.getShape(0)); i++) {

    std::size_t base_index = i * 3;

    if (base_index + 2 >= prediction_tensor_data.size()) {
      error("Incomplete data for a prediction tensor at the end of the vector.");
      throw std::runtime_error("Incomplete data for a prediction tensor at the end of the vector.");
    }

    // Extract the current prediction
    float px = prediction_tensor_data[base_index] * m_beamE;
    float py = prediction_tensor_data[base_index + 1] * m_beamE;
    float pz = prediction_tensor_data[base_index + 2] * m_beamE;

    // Calculate reconstructed electron energy
    double energy = sqrt(px * px + py * py + pz * pz + m_mass * m_mass);

    particle = out_particles->create();

    particle.setEnergy(energy);
    particle.setMomentum({px, py, pz});
    particle.setCharge(m_charge);
    particle.setMass(m_mass);
    particle.setPDG(m_cfg.pdg_value);

    //Check if both association collections are set and copy the MCParticle association
    if ((track_associations != nullptr) && (track_associations->size() > i)) {
      // Copy the association from the input to the output
      auto association     = track_associations->at(i);
      auto out_association = out_associations->create();
      out_association.setSim(association.getSim());
      out_association.setRec(particle);
      out_association.setWeight(association.getWeight());
    }
  }

  // TODO: Implement the association of the reconstructed particles with the tracks
}

} // namespace eicrecon
#endif
