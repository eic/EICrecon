// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Chun Yuen Tsang

#pragma once

#include <iostream>

#include "algorithms/digi/CFDROCDigitization.h"
#include "extensions/jana/JOmniFactory.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"

namespace eicrecon {

class CFDROCDigitization_factory
    : public JOmniFactory<CFDROCDigitization_factory, CFDROCDigitizationConfig> {
public:
  using AlgoT = eicrecon::CFDROCDigitization;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4hep::RawTimeSeries> m_in_sim_track{this};

  PodioOutput<edm4eic::RawTrackerHit> m_out_reco_particles{this};

  ParameterRef<double> m_fraction{this, "fraction", config().fraction};

  Service<AlgorithmsInit_service> m_algorithmsInit{this};

public:
  void Configure() {
    m_algo = std::make_unique<eicrecon::CFDROCDigitization>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void ChangeRun(int32_t /* run_number */) {}

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_in_sim_track()}, {m_out_reco_particles().get()});
  }
};
} // namespace eicrecon
