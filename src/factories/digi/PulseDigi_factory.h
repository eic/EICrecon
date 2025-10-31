// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Minho Kim

#pragma once

#include "algorithms/digi/PulseDigi.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class PulseDigi_factory : public JOmniFactory<PulseDigi_factory, PulseDigiConfig> {

public:
  using AlgoT = eicrecon::PulseDigi;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4eic::SimPulse> m_pulse_input{this};
  PodioOutput<edm4eic::RawHGCROCHit> m_digi_output{this};

  ParameterRef<double> m_time_window{this, "time_window", config().time_window};
  ParameterRef<double> m_adc_phase{this, "adc_phase", config().adc_phase};
  ParameterRef<double> m_toa_thres{this, "toa_thres", config().toa_thres};
  ParameterRef<double> m_tot_thres{this, "tot_thres", config().tot_thres};
  ParameterRef<unsigned int> m_capADC{this, "capADC", config().capADC};
  ParameterRef<double> m_dyRangeADC{this, "dyRangeADC", config().dyRangeADC};
  ParameterRef<unsigned int> m_capTOA{this, "capTOA", config().capTOA};
  ParameterRef<double> m_dyRangeTOA{this, "dyRangeTOA", config().dyRangeTOA};
  ParameterRef<unsigned int> m_capTOT{this, "capTOT", config().capTOT};
  ParameterRef<double> m_dyRangeTOT{this, "dyRangeTOT", config().dyRangeTOT};

  Service<AlgorithmsInit_service> m_algorithmsInit{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_pulse_input()}, {m_digi_output().get()});
  }
};
} // namespace eicrecon
