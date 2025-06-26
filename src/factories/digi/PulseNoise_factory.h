// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Simon Gardner

#pragma once

#include "algorithms/digi/PulseNoise.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class PulseNoise_factory : public JOmniFactory<PulseNoise_factory, PulseNoiseConfig> {
public:
  using AlgoT = eicrecon::PulseNoise;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4hep::EventHeader> m_in_headers{this};
#if EDM4EIC_VERSION_MAJOR > 8 || (EDM4EIC_VERSION_MAJOR == 8 && EDM4EIC_VERSION_MINOR >= 1)
  PodioInput<edm4eic::SimPulse> m_in_pulses{this};
  PodioOutput<edm4eic::SimPulse> m_out_pulses{this};
#else
  PodioInput<edm4hep::TimeSeries> m_in_pulses{this};
  PodioOutput<edm4hep::TimeSeries> m_out_pulses{this};
#endif

  ParameterRef<std::size_t> m_poles{this, "poles", config().poles};
  ParameterRef<double> m_variance{this, "variance", config().variance};
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

  void ChangeRun(int32_t /* run_number */) {}

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_in_headers(), m_in_pulses()}, {m_out_pulses().get()});
  }
};

} // namespace eicrecon
