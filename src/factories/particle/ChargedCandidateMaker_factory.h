// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Derek Anderson

#pragma once

#include <string>

#include "extensions/jana/JOmniFactory.h"
#include "algorithms/particle/ChargedCandidateMaker.h"

namespace eicrecon {

class ChargedCandidateMaker_factory : public JOmniFactory<ChargedCandidateMaker_factory> {

public:
  ///! alias for algorithm name
  using AlgoT = eicrecon::ChargedCandidateMaker;

private:
  // pointer to algorithm
  std::unique_ptr<AlgoT> m_algo;

  // input collection
  PodioInput<edm4eic::TrackClusterMatch> m_track_cluster_match_input{this};

  // output collection
  PodioOutput<edm4eic::ReconstructedParticle> m_charged_candidate_output{this};

  // services
  Service<AlgorithmsInit_service> m_algoInitSvc{this};
public:
  ///! Configures algorithm
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  ///! Primary algorithm call
  void Process(int32_t /*run_number*/, uint64_t /*event_number*/) {
    m_algo->process({m_track_cluster_match_input()}, {m_charged_candidate_output().get()});
  }
}; // end ChargedCandidateMaker_factory

} // namespace eicrecon
