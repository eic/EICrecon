// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Wouter Deconinck

#pragma once

#include "algorithms/digi/SiliconTrackerDigi.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class SiliconTrackerDigi_factory
    : public JOmniFactory<SiliconTrackerDigi_factory, SiliconTrackerDigiConfig> {

public:
  using AlgoT = eicrecon::SiliconTrackerDigi;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4hep::EventHeader> m_event_headers_input{this};
  PodioInput<edm4hep::SimTrackerHit> m_sim_hits_input{this};

  PodioOutput<edm4eic::RawTrackerHit> m_raw_hits_output{this};
  PodioOutput<edm4eic::MCRecoTrackerHitAssociation> m_assoc_output{this};

  ParameterRef<double> m_threshold{this, "threshold", config().threshold};
  ParameterRef<double> m_timeResolution{this, "timeResolution", config().timeResolution};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_event_headers_input(), m_sim_hits_input()},
                    {m_raw_hits_output().get(), m_assoc_output().get()});
  }
};

} // namespace eicrecon
