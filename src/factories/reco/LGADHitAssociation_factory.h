// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Chun Yuen Tsang

#pragma once

#include "algorithms/reco/LGADHitAssociation.h"
#include "extensions/jana/JOmniFactory.h"
#include "services/geometry/dd4hep/DD4hep_service.h"

namespace eicrecon {

class LGADHitAssociation_factory
    : public JOmniFactory<LGADHitAssociation_factory, LGADHitAssociationConfig> {
private:
  std::unique_ptr<eicrecon::LGADHitAssociation> m_algo;

  PodioInput<edm4eic::TrackerHit> m_cal_hits_input{this};
  PodioInput<edm4hep::SimTrackerHit> m_sim_hits_input{this};
  PodioOutput<edm4eic::MCRecoTrackerHitAssociation> m_assoc_output{this};

  ParameterRef<std::string> m_readout{this, "readout", config().readout};
  ParameterRef<double> m_assoDeltaT{this, "assoDeltaT", config().assoDeltaT};
  ParameterRef<std::vector<std::string>> m_subsensor_keys{this, "subsensorKeys",
                                                          config().subsensor_keys};

public:
  void Configure() {
    m_algo = std::make_unique<eicrecon::LGADHitAssociation>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void ChangeRun(int32_t /* run_number */) {}

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_cal_hits_input(), m_sim_hits_input()}, {m_assoc_output().get()});
  }
};

} // namespace eicrecon
