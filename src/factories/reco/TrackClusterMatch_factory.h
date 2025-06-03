// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Tristan Protzman

#pragma once

#include <spdlog/logger.h>
#include "extensions/jana/JOmniFactory.h"

#include "algorithms/reco/TrackClusterMatch.h"
#include "algorithms/reco/TrackClusterMatchConfig.h"
#include "services/geometry/dd4hep/DD4hep_service.h"

namespace eicrecon {
class TrackClusterMatch_factory
    : public JOmniFactory<TrackClusterMatch_factory, TrackClusterMatchConfig> {
private:
  // Underlying algorithm
  std::unique_ptr<eicrecon::TrackClusterMatch> m_algo;

  // Declare inputs
  PodioInput<edm4eic::TrackSegment> m_tracks{this};
  PodioInput<edm4eic::Cluster> m_clusters{this};

  // Declare outputs
  PodioOutput<edm4eic::TrackClusterMatch> m_matched_particles{this};

  // Declare parameters
  ParameterRef<double> m_matching_distance{this, "matchingDistance", config().matching_distance};

public:
  void Configure() {
    m_algo = std::make_unique<eicrecon::TrackClusterMatch>(GetPrefix());
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void ChangeRun(int32_t /* run_number */) {}

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_tracks(), m_clusters()}, {m_matched_particles().get()});
  }
};
} // namespace eicrecon
