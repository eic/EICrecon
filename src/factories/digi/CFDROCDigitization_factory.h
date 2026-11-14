// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Chun Yuen Tsang

#pragma once

#include <iostream>

#include "algorithms/digi/CFDROCDigitization.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

class CFDROCDigitization_factory
    : public JOmniAlgorithmFactory<CFDROCDigitization_factory, eicrecon::CFDROCDigitization,
                                   CFDROCDigitizationConfig> {
private:
  ParameterRef<double> m_fraction{this, "fraction", config().fraction};
};
} // namespace eicrecon
