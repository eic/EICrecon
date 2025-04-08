// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Chun Yuen Tsang

#pragma once

#include "extensions/jana/JOmniFactory.h"

#include "algorithms/digi/PulseCombiner.h"
#include <iostream>

namespace eicrecon {

class PulseCombiner_factory
    : public JOmniFactory<PulseCombiner_factory, PulseCombinerConfig> {
public:
  using AlgoT = eicrecon::PulseCombiner;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4hep::TimeSeries> m_in_pulses{this};

  PodioOutput<edm4hep::TimeSeries> m_out_pulses{this};

  ParameterRef<double>      m_minimum_separation{this, "minimumSeperation", config().minimum_separation};
  ParameterRef<bool>        m_interpolate_pulses{this, "interpolatePulses", config().interpolate_pulses};
  ParameterRef<std::string> m_readout{this, "readout", config().readout};
  ParameterRef<std::string> m_combine_field{this, "combineField", config().combine_field};

  Service<AlgorithmsInit_service> m_algorithmsInit{this};

public:
  void Configure() {
    m_algo = std::make_unique<eicrecon::PulseCombiner>(GetPrefix());
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
