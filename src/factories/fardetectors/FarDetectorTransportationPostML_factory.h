// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 - 2025, Simon Gardner

#pragma once

#include <edm4eic/EDM4eicVersion.h>
#include "algorithms/fardetectors/FarDetectorTransportationPostML.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

class FarDetectorTransportationPostML_factory
    : public JOmniAlgorithmFactory<FarDetectorTransportationPostML_factory,
                                   eicrecon::FarDetectorTransportationPostML,
                                   FarDetectorTransportationPostMLConfig> {
private:
  ParameterRef<float> m_beamE{this, "beamE", config().beamE};
  ParameterRef<bool> m_requireBeamElectron{this, "requireBeamElectron",
                                           config().requireBeamElectron};
  ParameterRef<int> m_pdg_value{
      this, "pdgValue", config().pdg_value,
      "PDG value for the particle type to identify (default is electron)"};
};

} // namespace eicrecon
