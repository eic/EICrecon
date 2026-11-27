// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 W.J. Llope

#pragma once

#include <edm4eic/MCRecoTrackerHitLinkCollection.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4hep/EventHeaderCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <memory>

#include "algorithms/digi/DircGRdig.h"
#include "algorithms/digi/DircGRdigConfig.h"
#include "extensions/jana/JOmniFactory.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"

namespace eicrecon {

class DircGRdig_factory : public JOmniFactory<DircGRdig_factory, DircGRdigConfig> {
public:
  using AlgoT = eicrecon::DircGRdig;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4hep::EventHeader> m_headers_input{this};
  PodioInput<edm4hep::SimTrackerHit> m_sim_hits_input{this};
  PodioOutput<edm4eic::RawTrackerHit> m_raw_hits_output{this};
  PodioOutput<edm4eic::MCRecoTrackerHitLink> m_links_output{this};

  ParameterRef<double> m_deadTime{this, "deadTime", config().deadTime,
                                  "pixel dead time from the hit start [ns]; 0 = every photon"};
  ParameterRef<double> m_timeResolution{this, "timeResolution", config().timeResolution,
                                        "TDC LSB [ns]"};
  ParameterRef<double> m_timeJitter{this, "timeJitter", config().timeJitter,
                                    "Gaussian time jitter per photon [ns]; 0 = off"};

  Service<AlgorithmsInit_service> m_algorithmsInit{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_headers_input(), m_sim_hits_input()},
                    {m_raw_hits_output().get(), m_links_output().get()});
  }
};

} // namespace eicrecon
