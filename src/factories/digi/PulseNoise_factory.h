// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Simon Gardner

#pragma once

#include "extensions/jana/JOmniFactory.h"

#include "algorithms/digi/PulseNoise.h"
#include <iostream>

namespace eicrecon {

class PulseNoise_factory
    : public JOmniFactory<PulseNoise_factory, PulseNoiseConfig> {
public:
  using AlgoT = eicrecon::PulseNoise;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4hep::TimeSeries> m_in_pulses{this};

  PodioOutput<edm4hep::TimeSeries> m_out_pulses{this};

  ParameterRef<size_t> m_poles{this, "poles", config().poles};
  ParameterRef<double> m_varience{this, "varience", config().varience};
  ParameterRef<double> m_alpha{this, "alpha", config().alpha};
  ParameterRef<double> m_scale{this, "scale", config().scale};

  Service<AlgorithmsInit_service> m_algorithmsInit{this};

public:
  void Configure() {
    m_algo = std::make_unique<eicrecon::PulseNoise>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void ChangeRun(int64_t run_number) {}

  void Process(int64_t run_number, uint64_t event_number) {
    m_algo->process({m_in_pulses()}, {m_out_pulses().get()});
  }
};

} // namespace eicrecon
