// Copyright 2023, Wouter Deconinck
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include "algorithms/calorimetry/CalorimeterTruthClustering.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"
#include <vector>

namespace eicrecon {

class CalorimeterTruthClustering_factory
    : public JOmniAlgorithmFactory<CalorimeterTruthClustering_factory,
                                   eicrecon::CalorimeterTruthClustering,
                                   CalorimeterTruthClusteringConfig> {
private:
  ParameterRef<std::vector<int>> m_promptDecayPDGs{this, "promptDecayPDGs",
                                                   config().promptDecayPDGs};
};

} // namespace eicrecon
