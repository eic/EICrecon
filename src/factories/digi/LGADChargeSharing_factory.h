// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Chun Yuen Tsang

#pragma once

#include "extensions/jana/JOmniFactory.h"

#include "algorithms/digi/LGADChargeSharing.h"
#include <iostream>

namespace eicrecon {

class LGADChargeSharing_factory
    : public JOmniFactory<LGADChargeSharing_factory, LGADChargeSharingConfig> {
public:
  using AlgoT = eicrecon::LGADChargeSharing;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4hep::SimTrackerHit> m_in_sim_track{this};

  PodioOutput<edm4hep::SimTrackerHit> m_out_reco_particles{this};

  ParameterRef<double> m_sigma_sharingx{this, "sigmaSharingX", config().sigma_sharingx};
  ParameterRef<double> m_sigma_sharingy{this, "sigmaSharingY", config().sigma_sharingy};

  Service<AlgorithmsInit_service> m_algorithmsInit{this};

public:
  void Configure() {
    m_algo = std::make_unique<eicrecon::LGADChargeSharing>(GetPrefix());
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
