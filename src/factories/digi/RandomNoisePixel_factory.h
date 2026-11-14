// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Minjung Kim, Joshua Sobaljic, Shujie Li

#pragma once

#include <cstdint>
#include <memory>

#include <edm4hep/EventHeader.h>

#include "algorithms/digi/RandomNoisePixel.h"
#include "algorithms/digi/RandomNoisePixelConfig.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

class RandomNoisePixel_factory
    : public JOmniAlgorithmFactory<RandomNoisePixel_factory, eicrecon::RandomNoisePixel,
                                   RandomNoisePixelConfig> {
private:
  ParameterRef<bool> m_addNoise{this, "addNoise", config().addNoise};
  ParameterRef<double> m_noise_rate{this, "noiseRate", config().noise_rate_per_pixel_per_event,
                                    "Noise occupancy per pixel per event"};
  ParameterRef<std::string> m_readout_name{this, "readout_name", config().readout_name};
};

} // namespace eicrecon
