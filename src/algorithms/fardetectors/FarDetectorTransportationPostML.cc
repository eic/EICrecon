// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Simon Gardner

#include <edm4eic/EDM4eicVersion.h>

#if EDM4EIC_VERSION_MAJOR >= 8
#include <cstddef>
#include <fmt/core.h>
#include <gsl/pointers>
#include <stdexcept>

#include "FarDetectorTransportationPostML.h"

namespace eicrecon {

  void FarDetectorTransportationPostML::init() {
    
    m_beamE = m_cfg.beamE;

  }

  void FarDetectorTransportationPostML::process(
      const FarDetectorTransportationPostML::Input& input,
      const FarDetectorTransportationPostML::Output& output) const {

    const auto [prediction_tensors,beamElectrons] = input;
    auto [out_particles] = output;

    //Set beam energy from first MCBeamElectron, using std::call_once
    if (beamElectrons)
    {
      std::call_once(m_initBeamE,[&](){
        // Check if beam electrons are present
        if(beamElectrons->size() == 0){
          error("No beam electrons found keeping default 10GeV beam energy.");
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
      throw std::runtime_error(fmt::format("Expected tensor rank to be 2, but it is {}", prediction_tensor.shape_size()));
    }

    if (prediction_tensor.getShape(1) != 3) {
      error("Expected 2 values per cluster in the output tensor, got {}", prediction_tensor.getShape(0));
      throw std::runtime_error(fmt::format("Expected 2 values per cluster in the output tensor, got {}", prediction_tensor.getShape(0)));
    }

    if (prediction_tensor.getElementType() != 1) { // 1 - float
      error("Expected a tensor of floats, but element type is {}", prediction_tensor.getElementType());
      throw std::runtime_error(fmt::format("Expected a tensor of floats, but element type is {}", prediction_tensor.getElementType()));
    }

    auto prediction_tensor_data = prediction_tensor.getFloatData();

    edm4eic::MutableReconstructedParticle particle;

    for ( auto prediction: prediction_tensor_data) {

      // Scale prediction to beam energy
      prediction[0] *= m_beamE; 
      prediction[1] *= m_beamE;
      prediction[2] *= m_beamE;
      
      // Calculate reconstructed electron energy
      double energy = sqrt(pow(prediction[0],2) + pow(prediction[1],2) + pow(prediction[2],2) + pow(0.000511,2));

      particle = out_particles->create();

      particle.setEnergy(energy);
      particle.setMomentum({prediction[0], prediction[1], prediction[2]});
      particle.setCharge(-1);
      particle.setMass(0.000511);
      particle.setPID(11);
    }

    
  }

} // namespace eicrecon
#endif
