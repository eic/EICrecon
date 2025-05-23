// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 - 2025, Simon Gardner

#include <edm4eic/EDM4eicVersion.h>

#if EDM4EIC_VERSION_MAJOR >= 8

#include <edm4hep/Vector2f.h>
#include <edm4hep/Vector3d.h>
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

  const auto [inputTracks, MCElectrons, beamElectrons] = input;
  auto [feature_tensors, target_tensors]               = output;

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
  feature_tensor.addToShape(4);     // x,z,dirx,diry
  feature_tensor.setElementType(1); // 1 - float

  edm4eic::MutableTensor target_tensor;
  if (MCElectrons != nullptr) {
    target_tensor = target_tensors->create();
    target_tensor.addToShape(inputTracks->size());
    target_tensor.addToShape(3);     // px,py,pz
    target_tensor.setElementType(1); // 1 - float
  }

  for (const auto& track : *inputTracks) {

    auto pos        = track.getLoc();
    auto trackphi   = track.getPhi();
    auto tracktheta = track.getTheta();

    feature_tensor.addToFloatData(pos.a);                           // x
    feature_tensor.addToFloatData(pos.b);                           // z
    feature_tensor.addToFloatData(sin(trackphi) * sin(tracktheta)); // dirx
    feature_tensor.addToFloatData(cos(trackphi) * sin(tracktheta)); // diry

    if (MCElectrons != nullptr) {
      // FIXME: use proper MC matching once available again, assume training sample is indexed correctly
      // Take the first scattered/simulated electron
      auto MCElectron         = MCElectrons->at(0);
      auto MCElectronMomentum = MCElectron.getMomentum() / m_beamE;
      target_tensor.addToFloatData(MCElectronMomentum.x);
      target_tensor.addToFloatData(MCElectronMomentum.y);
      target_tensor.addToFloatData(MCElectronMomentum.z);
    }
  }
}

} // namespace eicrecon
#endif
