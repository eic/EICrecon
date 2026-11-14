// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ePIC Collaboration

#pragma once

#include "algorithms/reco/ClustersToParticles.h"
#include "algorithms/reco/ClustersToParticlesConfig.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

class ClustersToParticles_factory
    : public JOmniAlgorithmFactory<ClustersToParticles_factory, eicrecon::ClustersToParticles,
                                   ClustersToParticlesConfig> {
private:
  // parameters
  ParameterRef<int> m_pdgCode{this, "pdgCode", config().pdgCode};
};

} // namespace eicrecon
