// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Chun Yuen Tsang

#pragma once

#include "algorithms/tracking/LGADHitClustering.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"
#include "services/geometry/dd4hep/DD4hep_service.h"

namespace eicrecon {

class LGADHitClustering_factory
    : public JOmniAlgorithmFactory<LGADHitClustering_factory, eicrecon::LGADHitClustering,
                                   LGADHitClusteringConfig> {
private:
  ParameterRef<std::string> m_readout{this, "readout", config().readout};
  ParameterRef<double> m_deltaT{this, "deltaT", config().deltaT};
  ParameterRef<bool> m_useAve{this, "useAve", config().useAve};
};

} // namespace eicrecon
