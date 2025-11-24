// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <algorithms/logger.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/MCRecoClusterParticleAssociationCollection.h>
#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <spdlog/logger.h>
#include <stdint.h>
#include <memory>

#include "algorithms/reco/MatchClusters.h"
#include "extensions/jana/JOmniFactory.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"

namespace eicrecon {

class MatchClusters_factory : public JOmniFactory<MatchClusters_factory> {
private:
  // Underlying algorithm
  std::unique_ptr<eicrecon::MatchClusters> m_algo;

  // Declare inputs
  PodioInput<edm4hep::MCParticle> m_mc_parts_input{this};
  PodioInput<edm4eic::ReconstructedParticle> m_rec_parts_input{this};
  PodioInput<edm4eic::MCRecoParticleAssociation> m_rec_assocs_input{this};
  PodioInput<edm4eic::Cluster> m_clusters_input{this};
  PodioInput<edm4eic::MCRecoClusterParticleAssociation> m_cluster_assocs_input{this};

  // Declare outputs
  PodioOutput<edm4eic::ReconstructedParticle> m_rec_parts_output{this};
  PodioOutput<edm4eic::MCRecoParticleAssociation> m_rec_assocs_output{this};

  Service<AlgorithmsInit_service> m_algorithmsInit{this};

public:
  void Configure() {
    m_algo = std::make_unique<MatchClusters>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process(
        {
            m_mc_parts_input(),
            m_rec_parts_input(),
            m_rec_assocs_input(),
            m_clusters_input(),
            m_cluster_assocs_input(),
        },
        {
            m_rec_parts_output().get(),
            m_rec_assocs_output().get(),
        });
  }
};

} // namespace eicrecon
