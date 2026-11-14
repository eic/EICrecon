// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Daniel Brandenburg

#pragma once

#include <JANA/JEvent.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "algorithms/reco/ScatteredElectronsEMinusPz.h"
#include "algorithms/reco/ScatteredElectronsEMinusPzConfig.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

class ScatteredElectronsEMinusPz_factory
    : public JOmniAlgorithmFactory<ScatteredElectronsEMinusPz_factory,
                                   eicrecon::ScatteredElectronsEMinusPz,
                                   ScatteredElectronsEMinusPzConfig> {
private:
  // Declare parameters
  ParameterRef<float> m_min_e_minus_pz{this, "minEMinusPz", config().minEMinusPz};
  ParameterRef<float> m_max_e_minus_pz{this, "maxEMinusPz", config().maxEMinusPz};
};

} // namespace eicrecon
