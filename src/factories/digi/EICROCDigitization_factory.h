// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Chun Yuen Tsang

#pragma once

#include <iostream>

#include "algorithms/digi/EICROCDigitization.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

class EICROCDigitization_factory
    : public JOmniAlgorithmFactory<EICROCDigitization_factory, eicrecon::EICROCDigitization,
                                   EICROCDigitizationConfig> {
private:
  ParameterRef<double> m_t_thres{this, "tThreshold", config().t_thres};
};
} // namespace eicrecon
