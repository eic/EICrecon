// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Chun Yuen Tsang

#pragma once

#include "extensions/jana/JOmniFactory.h"

#include "algorithms/digi/LGADPulseGeneration.h"
#include <iostream>

namespace eicrecon {

class LGADPulseGeneration_factory
    : public JOmniFactory<LGADPulseGeneration_factory, LGADPulseGenerationConfig> {
public:
  using AlgoT = eicrecon::LGADPulseGeneration;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4hep::SimTrackerHit> m_in_sim_track{this};

  PodioOutput<edm4hep::RawTimeSeries> m_out_reco_particles{this};

  ParameterRef<double> m_Vm{this, "Vm", config().Vm};
  ParameterRef<double> m_tMax{this, "tMax", config().tMax};
  ParameterRef<int> m_adc_range{this, "adcRange", config().adc_range};
  ParameterRef<int> m_tdc_range{this, "tdcRange", config().tdc_range};
  ParameterRef<double> m_ignore_thres{this, "ignoreThreshold", config().ignore_thres};

  Service<AlgorithmsInit_service> m_algorithmsInit{this};

public:
  void Configure() {
    std::unique_ptr<eicrecon::PulseShape> landau = std::make_unique<eicrecon::LandauPulse>(config().gain, config().Vm,
                                                          config().sigma_analog, config().adc_range);
    m_algo = std::make_unique<eicrecon::LGADPulseGeneration>(GetPrefix(), std::move(landau));
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
