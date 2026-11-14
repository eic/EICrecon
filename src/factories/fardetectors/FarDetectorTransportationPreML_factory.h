// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 - 2025, Simon Gardner

#pragma once

#include "algorithms/fardetectors/FarDetectorTransportationPreML.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

class FarDetectorTransportationPreML_factory
    : public JOmniAlgorithmFactory<FarDetectorTransportationPreML_factory,
                                   eicrecon::FarDetectorTransportationPreML,
                                   FarDetectorTransportationPreMLConfig> {
private:
  ParameterRef<float> m_beamE{this, "beamE", config().beamE};
  ParameterRef<bool> m_requireBeamElectron{this, "requireBeamElectron",
                                           config().requireBeamElectron};
};

} // namespace eicrecon
