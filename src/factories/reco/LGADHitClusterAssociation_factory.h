// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Chun Yuen Tsang

#pragma once

#include "algorithms/reco/LGADHitClusterAssociation.h"
#include "extensions/jana/JOmniFactory.h"
#include "services/geometry/dd4hep/DD4hep_service.h"

namespace eicrecon {

class LGADHitClusterAssociation_factory
    : public JOmniFactory<LGADHitClusterAssociation_factory, LGADHitClusterAssociationConfig> {
private:
  std::unique_ptr<eicrecon::LGADHitClusterAssociation> m_algo;

  PodioInput<edm4eic::Measurement2D> m_meas_hits_input{this};
  PodioInput<edm4eic::RawTrackerHit> m_raw_hits_input{this};
  PodioOutput<edm4eic::TrackerHit> m_asso_hits_output{this};

  ParameterRef<std::string> m_readout{this, "readout", config().readout};

public:
  void Configure() {
    m_algo = std::make_unique<eicrecon::LGADHitClusterAssociation>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void ChangeRun(int64_t run_number) {}

  void Process(int64_t run_number, uint64_t event_number) {
    m_algo->process({m_meas_hits_input(), m_raw_hits_input()}, {m_asso_hits_output().get()});
  }
};

} // namespace eicrecon
