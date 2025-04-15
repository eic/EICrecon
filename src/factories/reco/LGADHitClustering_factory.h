// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Wouter Deconinck

#pragma once

#include "algorithms/reco/LGADHitClustering.h"
#include "extensions/jana/JOmniFactory.h"
#include "services/geometry/dd4hep/DD4hep_service.h"

namespace eicrecon {

class LGADHitClustering_factory
    : public JOmniFactory<LGADHitClustering_factory, LGADHitClusteringConfig> {
private:
  std::unique_ptr<eicrecon::LGADHitClustering> m_algo;

  PodioInput<edm4eic::TrackerHit> m_hits_input{this};
  PodioOutput<edm4eic::Measurement2D> m_clusters_output{this};

  ParameterRef<std::string> m_readout{this, "readout", config().readout};

public:
  void Configure() {
    m_algo = std::make_unique<eicrecon::LGADHitClustering>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void ChangeRun(int64_t run_number) {}

  void Process(int64_t run_number, uint64_t event_number) {
    m_algo->process({m_hits_input()}, {m_clusters_output().get()});
  }
};

} // namespace eicrecon
