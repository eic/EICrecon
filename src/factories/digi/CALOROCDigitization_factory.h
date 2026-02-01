// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Minho Kim

#pragma once

#include "algorithms/digi/CALOROCDigitization.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class CALOROCDigitization_factory : public JOmniFactory<CALOROCDigitization_factory, CALOROCDigitizationConfig> {

public:
  using AlgoT = eicrecon::CALOROCDigitization;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4eic::SimPulse> m_pulse_input{this};
  PodioOutput<edm4eic::RawCALOROCHit> m_digi_output{this};

  ParameterRef<double> m_time_window{this, "timeWindow", config().time_window};
  ParameterRef<double> m_adc_phase{this, "adcPhase", config().adc_phase};
  ParameterRef<double> m_toa_thres{this, "toaThres", config().toa_thres};
  ParameterRef<double> m_tot_thres{this, "totThres", config().tot_thres};
  ParameterRef<unsigned int> m_capADC{this, "capADC", config().capADC};
  ParameterRef<double> m_dyRangeSingleGainADC{this, "dyRangeSingleGainADC", config().dyRangeSingleGainADC};
  ParameterRef<double> m_dyRangeHighGainADC{this, "dyRangeHighGainADC", config().dyRangeHighGainADC};
  ParameterRef<double> m_dyRangeLowGainADC{this, "dyRangeLowGainADC", config().dyRangeLowGainADC};
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
