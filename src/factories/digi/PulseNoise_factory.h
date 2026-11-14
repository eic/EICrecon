// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Simon Gardner

#pragma once

#include <edm4eic/EDM4eicVersion.h>
#include "algorithms/digi/PulseNoise.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

class PulseNoise_factory
    : public JOmniAlgorithmFactory<PulseNoise_factory, eicrecon::PulseNoise, PulseNoiseConfig> {
private:
  ParameterRef<std::size_t> m_poles{this, "poles", config().poles};
  ParameterRef<double> m_variance{this, "variance", config().variance};
  ParameterRef<double> m_alpha{this, "alpha", config().alpha};
  ParameterRef<double> m_scale{this, "scale", config().scale};
};

} // namespace eicrecon
