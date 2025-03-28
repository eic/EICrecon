// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Simon Gardner
//

#pragma once

#include "extensions/jana/JOmniFactory.h"

#include "algorithms/digi/SiliconPulseGeneration.h"
#include <iostream>

namespace eicrecon {

class SiliconPulseGeneration_factory
    : public JOmniFactory<SiliconPulseGeneration_factory, SiliconPulseGenerationConfig> {
public:
  using AlgoT = eicrecon::SiliconPulseGeneration;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4hep::SimTrackerHit> m_in_sim_hits{this};

  PodioOutput<edm4hep::TimeSeries> m_out_pulses{this};

  ParameterRef<float> m_timestep{this, "timestep", config().timestep};
  ParameterRef<float> m_ignore_thres{this, "ignoreThreshold", config().ignore_thres};
  ParameterRef<float> m_min_sampling_time{this, "minSamplingTime", config().min_sampling_time};
  ParameterRef<int> m_max_time_bins{this, "maxTimeBins", config().max_time_bins};

  Service<AlgorithmsInit_service> m_algorithmsInit{this};

public:
  void Configure() {
    m_algo = std::make_unique<eicrecon::SiliconPulseGeneration>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void ChangeRun(int64_t run_number) {}

  void Process(int64_t run_number, uint64_t event_number) {
    m_algo->process({m_in_sim_hits()}, {m_out_pulses().get()});
  }
};

} // namespace eicrecon
