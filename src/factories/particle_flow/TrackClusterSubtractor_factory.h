// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Derek Anderson

#pragma once

#include <DD4hep/Detector.h>
#include <edm4eic/EDM4eicVersion.h>
#include <string>

#include "extensions/jana/JOmniAlgorithmFactory.h"
#include "services/geometry/dd4hep/DD4hep_service.h"
#include "algorithms/particle_flow/TrackClusterSubtractor.h"

namespace eicrecon {

class TrackClusterSubtractor_factory
    : public JOmniAlgorithmFactory<TrackClusterSubtractor_factory, eicrecon::TrackClusterSubtractor,
                                   TrackClusterSubtractorConfig> {
private:
  // parameter bindings
  ParameterRef<double> m_energyFractionToSubtract{this, "energyFractionToSubtract",
                                                  config().energyFractionToSubtract};
  ParameterRef<int32_t> m_defaultPDG{this, "defaultPDG", config().defaultPDG};
  ParameterRef<uint8_t> m_surfaceToUse{this, "surfaceToUse", config().surfaceToUse};
  ParameterRef<bool> m_doNSigmaCut{this, "doNSigmaCut", config().doNSigmaCut};
  ParameterRef<uint32_t> m_nSigmaMax{this, "nSigmaMax", config().nSigmaMax};
  ParameterRef<double> m_trackResolution{this, "trackResolution", config().trackResolution};
  ParameterRef<double> m_calorimeterResolution{this, "calorimeterResolution",
                                               config().calorimeterResolution};
}; // end TrackClusterSubtractor_factory

} // namespace eicrecon
