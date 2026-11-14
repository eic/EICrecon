// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Xin Dong

#pragma once

#include <JANA/JEvent.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "algorithms/reco/SecondaryVerticesHelix.h"
#include "algorithms/reco/SecondaryVerticesHelixConfig.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

class SecondaryVerticesHelix_factory
    : public JOmniAlgorithmFactory<SecondaryVerticesHelix_factory, eicrecon::SecondaryVerticesHelix,
                                   SecondaryVerticesHelixConfig> {
private:
  // Declare parameters
  ParameterRef<bool> m_unlikesign{this, "unlikesign", config().unlikesign};
  ParameterRef<float> m_minDca{this, "minDca", config().minDca};
  ParameterRef<float> m_maxDca12{this, "maxDca12", config().maxDca12};
  ParameterRef<float> m_maxDca{this, "maxDca", config().maxDca};
  ParameterRef<float> m_minCostheta{this, "minCostheta", config().minCostheta};
};

} // namespace eicrecon
