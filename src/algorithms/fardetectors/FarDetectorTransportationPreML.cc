// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 - 2025, Simon Gardner

#include <edm4hep/Vector3d.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <cmath>
#include <gsl/pointers>
#include <stdexcept>

#include "FarDetectorTransportationPreML.h"
#include "algorithms/fardetectors/FarDetectorTransportationPreML.h"

namespace eicrecon {

void FarDetectorTransportationPreML::init() { m_beamE = m_cfg.beamE; }

void FarDetectorTransportationPreML::process(
    const FarDetectorTransportationPreML::Input& input,
    const FarDetectorTransportationPreML::Output& output) const {

  const auto [inputTracks, mcAssociation, beamElectrons] = input;
  auto [feature_tensors, target_tensors]                 = output;

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

  edm4eic::MutableTensor feature_tensor = feature_tensors->create();
  feature_tensor.addToShape(inputTracks->size());
  feature_tensor.addToShape(6);     // x,y,z,dirx,diry,dirz
  feature_tensor.setElementType(1); // 1 - float

  edm4eic::MutableTensor target_tensor;
  if (mcAssociation != nullptr) {
    target_tensor = target_tensors->create();
    target_tensor.addToShape(inputTracks->size());
    target_tensor.addToShape(3);     // px,py,pz
    target_tensor.setElementType(1); // 1 - float
  }

  // Loop through inputTracks and simultaneously optionally associations if available
  // and fill the feature and target tensors
  for (const auto& track : *inputTracks) {

    auto position = track.getPosition();
    auto momentum = track.getMomentum();

    feature_tensor.addToFloatData(position.x); // x
    feature_tensor.addToFloatData(position.y); // y
    feature_tensor.addToFloatData(position.z); // z
    feature_tensor.addToFloatData(momentum.x); // dirx
    feature_tensor.addToFloatData(momentum.y); // diry
    feature_tensor.addToFloatData(momentum.z); // dirz

    if ((mcAssociation != nullptr) && (!mcAssociation->empty())) {
      //Loop through the MCRecoTrackParticleAssociationCollection finding the first one associated with the current track
      for (const auto& assoc : *mcAssociation) {
        if (assoc.getRec() == track) {
          // Process the association if it exists and is non-empty
          const auto& association = assoc.getSim(); // Assuming 1-to-1 mapping
          auto MCElectronMomentum = association.getMomentum() / m_beamE;
          target_tensor.addToFloatData(MCElectronMomentum.x);
          target_tensor.addToFloatData(MCElectronMomentum.y);
          target_tensor.addToFloatData(MCElectronMomentum.z);
          break; // Exit loop after finding the first association
        }
      }
    }
  }
}

} // namespace eicrecon
