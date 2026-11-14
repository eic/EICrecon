// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Subhadip Pal

#pragma once

#include "algorithms/particle_flow/CaloRemnantCombiner.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

class CaloRemnantCombiner_factory
    : public JOmniAlgorithmFactory<CaloRemnantCombiner_factory, eicrecon::CaloRemnantCombiner,
                                   CaloRemnantCombinerConfig> {
private:
  // Declare parameters
  ParameterRef<double> m_ecalDeltaR{this, "ecalDeltaR", config().ecalDeltaR};
  ParameterRef<double> m_hcalDeltaR{this, "hcalDeltaR", config().hcalDeltaR};
};
} // namespace eicrecon
