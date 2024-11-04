// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Chun Yuen Tsang

#pragma once

#include "extensions/jana/JOmniFactory.h"

#include "algorithms/digi/TOFPulseGeneration.h"
#include <iostream>

namespace eicrecon {

class TOFPulseGeneration_factory : public JOmniFactory<TOFPulseGeneration_factory, TOFHitDigiConfig> {
public:
    using AlgoT = eicrecon::TOFPulseGeneration;
private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4hep::SimTrackerHit> m_in_sim_track{this};

  PodioOutput<edm4hep::RawTimeSeries> m_out_reco_particles{this};

  ParameterRef<double> m_Vm{this, "Vm", config().Vm};
  ParameterRef<double> m_tMin{this, "tMin", config().tMin};
  ParameterRef<double> m_tMax{this, "tMax", config().tMax};
  ParameterRef<int> m_adc_range{this, "adcRange", config().adc_range};
  ParameterRef<int> m_tdc_range{this, "tdcRange", config().tdc_range};
  ParameterRef<int> m_nBins{this, "nBins", config().nBins};
  ParameterRef<double> m_ignore_thres{this, "ignoreThreshold", config().ignore_thres};
public:
  void Configure() {
    m_algo = std::make_unique<eicrecon::TOFPulseGeneration>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void ChangeRun(int64_t run_number) {
  }

  void Process(int64_t run_number, uint64_t event_number) {
    m_algo->process({m_in_sim_track()}, {m_out_reco_particles().get()});
  }
};

} // namespace eicrecon
