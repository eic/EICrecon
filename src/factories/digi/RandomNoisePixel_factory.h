// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Minjung Kim, Joshua Sobaljic, Shujie Li

#pragma once

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
  ParameterRef<std::string> m_readout_name{this, "readout_name", config().readout_name};

public:
  // Read JANA parameters, construct the algorithm, and build its geometry cache.
  void Configure() {
    // Step 1: register one unprefixed rate shared by BVTX, BTRK, and ECTRK.
    GetApplication()->SetDefaultParameter("SVT:noise_rate_per_pixel_per_event",
                                          config().noise_rate_per_pixel_per_event,
                                          "SVT noise occupancy per pixel per event");

    // Step 2: forward the completed configuration and initialize the static cache.
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  // Forward the required EventHeader and the new output collection for each event.
  void Process(int32_t /* run_number */, uint64_t /* event_number */) override {
    m_algo->process({m_in_event_header()}, {m_out_hits().get()});
  }
};

} // namespace eicrecon
