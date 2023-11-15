// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Wouter Deconinck

#pragma once

#include "algorithms/calorimetry/TruthEnergyPositionClusterMerger.h"
#include "extensions/jana/JOmniFactory.h"


namespace eicrecon {

class TruthEnergyPositionClusterMerger_factory : public JOmniFactory<TruthEnergyPositionClusterMerger_factory> {
    eicrecon::TruthEnergyPositionClusterMerger m_algo;

    PodioInput<edm4hep::MCParticle> m_mcparticles_input {this};
    PodioInput<edm4eic::Cluster> m_energy_clusters_input {this};
    PodioInput<edm4eic::MCRecoClusterParticleAssociation> m_energy_assocs_input {this};
    PodioInput<edm4eic::Cluster> m_position_clusters_input {this};
    PodioInput<edm4eic::MCRecoClusterParticleAssociation> m_position_assocs_input {this};

    PodioOutput<edm4eic::Cluster> m_clusters_output {this};
    PodioOutput<edm4eic::MCRecoClusterParticleAssociation> m_assocs_output {this};

public:
    void Configure() {
        m_algo.applyConfig(config());
        m_algo.init(logger());
    }

    void ChangeRun(int64_t run_number) {
    }

    void Process(int64_t run_number, uint64_t event_number) {

        std::tie(m_clusters_output(),
                 m_assocs_output()) = m_algo.process(m_mcparticles_input(),
                                                     m_energy_clusters_input(),
                                                     m_energy_assocs_input(),
                                                     m_position_clusters_input(),
                                                     m_position_assocs_input());
    }
};

} // eicrecon
