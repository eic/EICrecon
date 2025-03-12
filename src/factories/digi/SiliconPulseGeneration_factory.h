// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Chun Yuen Tsang

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

  PodioInput<edm4hep::SimTrackerHit> m_in_sim_track{this};

  PodioOutput<edm4hep::TimeSeries> m_out_reco_particles{this};

  ParameterRef<double> m_timestep{this, "timestep", config().timestep};
  ParameterRef<double> m_ignore_thres{this, "ignoreThreshold", config().ignore_thres};

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
    m_algo->process({m_in_sim_track()}, {m_out_reco_particles().get()});
  }
};

} // namespace eicrecon
