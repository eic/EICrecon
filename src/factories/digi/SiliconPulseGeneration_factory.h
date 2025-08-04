// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024-2025 Simon Gardner, Chun Yuen Tsang, Prithwish Tribedy
//

#pragma once

#include "algorithms/digi/SiliconPulseGeneration.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class SiliconPulseGeneration_factory
    : public JOmniFactory<SiliconPulseGeneration_factory, SiliconPulseGenerationConfig> {
public:
  using AlgoT = eicrecon::SiliconPulseGeneration;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4hep::SimTrackerHit> m_in_sim_hits{this};
#if EDM4EIC_VERSION_MAJOR > 8 || (EDM4EIC_VERSION_MAJOR == 8 && EDM4EIC_VERSION_MINOR >= 1)
  PodioOutput<edm4eic::SimPulse> m_out_pulses{this};
#else
  PodioOutput<edm4hep::TimeSeries> m_out_pulses{this};
#endif

  ParameterRef<std::string> m_pulse_shape_function{this, "pulseShapeFunction",
                                                   config().pulse_shape_function};
  ParameterRef<std::vector<double>> m_pulse_shape_params{this, "pulseShapeParams",
                                                         config().pulse_shape_params};
  ParameterRef<double> m_timestep{this, "timestep", config().timestep};
  ParameterRef<double> m_ignore_thres{this, "ignoreThreshold", config().ignore_thres};
  ParameterRef<double> m_min_sampling_time{this, "minSamplingTime", config().min_sampling_time};
  ParameterRef<uint32_t> m_max_time_bins{this, "maxTimeBins", config().max_time_bins};

  Service<AlgorithmsInit_service> m_algorithmsInit{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_in_sim_hits()}, {m_out_pulses().get()});
  }
};

} // namespace eicrecon
