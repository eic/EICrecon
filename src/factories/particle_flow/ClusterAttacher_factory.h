// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Derek Anderson

#pragma once

#include <string>

#include "extensions/jana/JOmniFactory.h"
#include "algorithms/particle_flow/ClusterAttacher.h"

namespace eicrecon {

class ClusterAttacher_factory : public JOmniFactory<ClusterAttacher_factory, NoConfig> {

public:
  using AlgoT = eicrecon::ClusterAttacher;

private:
  std::unique_ptr<AlgoT> m_algo;

  // input collections
  PodioInput<edm4eic::ReconstructedParticle> m_particle_input{this};
  PodioInput<edm4eic::ClusterCollection> m_cluster_input{this};
  PodioInput<edm4eic::TrackClusterMatch> m_match_input{this};
  PodioInput<edm4eic::MCRecoParticleLink, true> m_link_input{this}; // Optional: for truth links

  // output collections
  PodioOutput<edm4eic::ReconstructedParticle> m_particle_output{this};
  PodioOutput<edm4eic::MCRecoParticleLilnk, true> m_link_output{this};  // OPtional: for truth links

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /*run_number*/, uint64_t /*event_number*/) {
    m_algo->process({m_particle_input(), m_cluster_input(), m_match_input(), m_link_input()},
                    {m_charged_candidate_output().get(), m_link_output().get()});
  }
}; // end ClusterAttacher_factory

} // namespace eicrecon
