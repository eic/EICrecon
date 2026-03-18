// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ePIC Collaboration

#pragma once

#include "extensions/jana/JOmniFactory.h"
#include "algorithms/reco/ClustersToParticles.h"

namespace eicrecon {

class ClustersToParticles_factory
    : public JOmniFactory<ClustersToParticles_factory, NoConfig> {
public:
  using AlgoT = eicrecon::ClustersToParticles;

private:
  // algorithm
  std::unique_ptr<AlgoT> m_algo;

  // input collections
  PodioInput<edm4eic::Cluster> m_clusters_in{this};
  PodioInput<edm4eic::MCRecoClusterParticleAssociation, true> m_cluster_assocs_in{this};

  // output collections
  PodioOutput<edm4eic::ReconstructedParticle> m_parts_out{this};
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
  PodioOutput<edm4eic::MCRecoParticleLink, true> m_part_links_out{this};
#endif
  PodioOutput<edm4eic::MCRecoParticleAssociation, true> m_part_assocs_out{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_clusters_in(), m_cluster_assocs_in()}, 
                    {m_parts_out().get(),
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                     m_part_links_out(),
#endif
                     m_part_assocs_out()});
  }
};

} // namespace eicrecon
