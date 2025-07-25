// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024-2025 Chun Yuen Tsang, Simon Gardner

#pragma once

#include "algorithms/digi/SiliconChargeSharing.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class SiliconChargeSharing_factory
    : public JOmniFactory<SiliconChargeSharing_factory, SiliconChargeSharingConfig> {
public:
  using AlgoT = eicrecon::SiliconChargeSharing;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4hep::SimTrackerHit> m_in_sim_track{this};
  PodioOutput<edm4hep::SimTrackerHit> m_out_reco_particles{this};

  Service<AlgorithmsInit_service> m_algorithmsInit{this};

  ParameterRef<float> m_sigma_sharingx{this, "sigmaSharingX", config().sigma_sharingx};
  ParameterRef<float> m_sigma_sharingy{this, "sigmaSharingY", config().sigma_sharingy};
  ParameterRef<float> m_min_edep{this, "minEDep", config().min_edep};
  ParameterRef<std::string> m_readout{this, "readout", config().readout};
  ParameterRef<eicrecon::SiliconChargeSharingConfig::ESigmaMode> m_sigma_mode{this, "sigmaMode",
                                                                              config().sigma_mode};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
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
