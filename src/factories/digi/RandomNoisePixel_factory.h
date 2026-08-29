// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Minjung Kim, Joshua Sobaljic, Shujie Li

#pragma once
#include "algorithms/digi/RandomNoisePixelConfig.h"
#include "extensions/jana/JOmniFactory.h"
#ifndef EICRECON_FACTORY_PRECOMPILE

namespace eicrecon {
class RandomNoisePixel_factory;
}

extern template class JOmniFactory<eicrecon::RandomNoisePixel_factory, eicrecon::RandomNoisePixelConfig>;

#else

#include <cstdint>
#include <memory>
#include <edm4hep/EventHeader.h>
#include "algorithms/digi/RandomNoisePixel.h"
#include "algorithms/digi/RandomNoisePixelConfig.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class RandomNoisePixel_factory
    : public JOmniFactory<RandomNoisePixel_factory, RandomNoisePixelConfig> {
public:
  using AlgoT = eicrecon::RandomNoisePixel;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4hep::EventHeader> m_in_event_header{this};
  PodioOutput<edm4eic::RawTrackerHit> m_out_hits{this};

  ParameterRef<bool> m_addNoise{this, "addNoise", config().addNoise};
  ParameterRef<double> m_noise_rate{this, "noiseRate", config().noise_rate_per_pixel_per_event,
                                    "Noise occupancy per pixel per event"};
  ParameterRef<std::string> m_readout_name{this, "readout_name", config().readout_name};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) override {
    m_algo->process({m_in_event_header()}, {m_out_hits().get()});
  }
};

} // namespace eicrecon

#endif // EICRECON_FACTORY_PRECOMPILE
