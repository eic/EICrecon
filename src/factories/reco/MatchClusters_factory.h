// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include "extensions/jana/JOmniFactory.h"

#include "extensions/jana/JOmniFactoryGeneratorT.h"
#ifndef EICRECON_FACTORY_PRECOMPILE

namespace eicrecon {
class MatchClusters_factory;
}

extern template class JOmniFactory<eicrecon::MatchClusters_factory, eicrecon::NoConfig>;
extern template class JOmniFactoryGeneratorT<eicrecon::MatchClusters_factory>;

#else

#include <algorithms/logger.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/EDM4eicVersion.h>
#include <edm4eic/MCRecoClusterParticleAssociationCollection.h>
#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <spdlog/logger.h>
#include <stdint.h>
#include <memory>

#include "algorithms/reco/MatchClusters.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"

namespace eicrecon {

class MatchClusters_factory :
        public JOmniFactory<MatchClusters_factory, NoConfig> {

private:
    std::unique_ptr<eicrecon::MatchClusters> m_algo;

    PodioInput<edm4hep::MCParticle> m_rc_particles_input {this};
    PodioInput<edm4eic::Cluster> m_rc_clusters_input {this};
    PodioInput<edm4eic::MCRecoParticleAssociation> m_rc_particle_assocs_input {this};
    PodioInput<edm4eic::MCRecoClusterParticleAssociation> m_rc_cluster_assocs_input {this};

    PodioOutput<edm4eic::ReconstructedParticle> m_rc_particles_output {this};
    PodioOutput<edm4eic::MCRecoParticleAssociation> m_rc_particle_assocs_output {this};
    PodioOutput<edm4eic::MCRecoClusterParticleAssociation> m_rc_cluster_assocs_output {this};

public:
    void Configure() {
        m_algo = std::make_unique<eicrecon::MatchClusters>(GetPrefix());
        m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
        m_algo->init();
    }

    void ChangeRun(int64_t run_number) {
    }

    void Process(const Input& input, const Output& output) const {
        m_algo->process({m_rc_particles_input(), m_rc_clusters_input(), m_rc_particle_assocs_input(), m_rc_cluster_assocs_input()},
                        {m_rc_particles_output().get(), m_rc_particle_assocs_output().get(), m_rc_cluster_assocs_output().get()});
    }
};

} // eicrecon
#endif
