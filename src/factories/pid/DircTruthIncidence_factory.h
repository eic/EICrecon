// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 hpDIRC GR effort

#pragma once

#include <edm4eic/TrackSegmentCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <memory>

#include "algorithms/pid/DircTruthIncidence.h"
#include "extensions/jana/JOmniFactory.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"

namespace eicrecon {

class DircTruthIncidence_factory
    : public JOmniFactory<DircTruthIncidence_factory, DircTruthIncidenceConfig> {
public:
  using AlgoT = eicrecon::DircTruthIncidence;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4hep::SimTrackerHit> m_quartz_hits_input{this};
  PodioOutput<edm4eic::TrackSegment> m_segments_output{this};
  PodioOutput<edm4hep::SimTrackerHit> m_incidence_hits_output{this};

  ParameterRef<double> m_refractiveIndex{this, "refractiveIndex", config().refractiveIndex};
  ParameterRef<double> m_betaMargin{this, "betaMargin", config().betaMargin};
  ParameterRef<bool> m_primaryOnly{this, "primaryOnly", config().primaryOnly};
  ParameterRef<bool> m_dropBornInside{this, "dropBornInside", config().dropBornInside};
  ParameterRef<double> m_bornInsideTolerance{this, "bornInsideTolerance",
                                             config().bornInsideTolerance};
  ParameterRef<bool> m_fixEndsInside{this, "fixEndsInside", config().fixEndsInside};
  ParameterRef<double> m_endsInsideTolerance{this, "endsInsideTolerance",
                                             config().endsInsideTolerance};

  Service<AlgorithmsInit_service> m_algorithmsInit{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_quartz_hits_input()},
                    {m_segments_output().get(), m_incidence_hits_output().get()});
  }
};

} // namespace eicrecon
