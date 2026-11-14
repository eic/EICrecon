// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Simon Gardner

#pragma once

#include <edm4eic/EDM4eicVersion.h>
#include "extensions/jana/JOmniAlgorithmFactory.h"

#include "algorithms/digi/PulseCombiner.h"
#include <iostream>

namespace eicrecon {

class PulseCombiner_factory
    : public JOmniAlgorithmFactory<PulseCombiner_factory, eicrecon::PulseCombiner,
                                   PulseCombinerConfig> {
private:
  ParameterRef<double> m_minimum_separation{this, "minimumSeperation", config().minimum_separation};
  ParameterRef<std::string> m_readout{this, "readout", config().readout};
  ParameterRef<std::string> m_combine_field{this, "combineField", config().combine_field};
};

} // namespace eicrecon
