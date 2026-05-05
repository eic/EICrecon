// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Derek Anderson

#pragma once

#include <string>

#include "extensions/jana/JOmniFactory.h"
#include "algorithms/particle_flow/ChargedCandidateMaker.h"

namespace eicrecon {

class ChargedCandidateMaker_factory : public JOmniFactory<ChargedCandidateMaker_factory, NoConfig> {

public:
  using AlgoT = eicrecon::ChargedCandidateMaker;

private:
  std::unique_ptr<AlgoT> m_algo;

  // input collection
  PodioInput<edm4eic::TrackClusterMatch> m_track_cluster_match_input{this};

  // output collection
  PodioOutput<edm4eic::ReconstructedParticle> m_charged_candidate_output{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /*run_number*/, uint64_t /*event_number*/) {
    m_algo->process({m_track_cluster_match_input()}, {m_charged_candidate_output().get()});
  }
}; // end ChargedCandidateMaker_factory

} // namespace eicrecon
