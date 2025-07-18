// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Chun Yuen Tsang

#pragma once

#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "algorithms/digi/SiliconPulseDiscretization.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class SiliconPulseDiscretization_factory
    : public JOmniFactory<SiliconPulseDiscretization_factory, SiliconPulseDiscretizationConfig> {
public:
  using AlgoT = eicrecon::SiliconPulseDiscretization;

private:
  std::unique_ptr<AlgoT> m_algo;

#if EDM4EIC_VERSION_MAJOR > 8 || (EDM4EIC_VERSION_MAJOR == 8 && EDM4EIC_VERSION_MINOR >= 1)
  PodioInput<edm4eic::SimPulse> m_in_pulses{this};
#else
  PodioInput<edm4hep::TimeSeries> m_in_pulses{this};
#endif
  PodioOutput<edm4hep::RawTimeSeries> m_out_pulses{this};

  ParameterRef<double> m_EICROC_period{this, "EICROCPeriod", config().EICROC_period};
  ParameterRef<double> m_local_period{this, "localPeriod", config().local_period};
  ParameterRef<double> m_global_offset{this, "globalOffset", config().global_offset};

  Service<AlgorithmsInit_service> m_algorithmsInit{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_in_pulses()}, {m_out_pulses().get()});
  }
};

} // namespace eicrecon
