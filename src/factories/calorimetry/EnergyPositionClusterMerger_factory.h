// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Wouter Deconinck

#pragma once

#include "algorithms/calorimetry/EnergyPositionClusterMerger.h"
#include "extensions/jana/JOmniFactory.h"


namespace eicrecon {

class EnergyPositionClusterMerger_factory :
    public JOmniFactory<EnergyPositionClusterMerger_factory, EnergyPositionClusterMergerConfig> {

public:
    using AlgoT = eicrecon::EnergyPositionClusterMerger;
private:
    std::unique_ptr<AlgoT> m_algo;

    PodioInput<edm4eic::Cluster> m_energy_cluster_input {this};
    PodioInput<edm4eic::MCRecoClusterParticleAssociation> m_energy_assoc_input {this};
    PodioInput<edm4eic::Cluster> m_position_cluster_input {this};
    PodioInput<edm4eic::MCRecoClusterParticleAssociation> m_position_assoc_input {this};

    PodioOutput<edm4eic::Cluster> m_cluster_output {this};
    PodioOutput<edm4eic::MCRecoClusterParticleAssociation> m_assoc_output {this};

    ParameterRef<double> m_energyRelTolerance {this, "energyRelTolerance", config().energyRelTolerance};
    ParameterRef<double> m_phiTolerance {this, "phiTolerance", config().phiTolerance};
    ParameterRef<double> m_etaTolerance {this, "etaTolerance", config().etaTolerance};

public:
    void Configure() {
        m_algo = std::make_unique<AlgoT>(GetPrefix());
        m_algo->applyConfig(config());
        m_algo->init(logger());
    }

    void ChangeRun(int64_t run_number) {
    }

    void Process(int64_t run_number, uint64_t event_number) {
        m_algo->process({m_energy_cluster_input(), m_energy_assoc_input(),
                         m_position_cluster_input(), m_position_assoc_input()},
                        {m_cluster_output().get(), m_assoc_output().get()});
    }
};

} // eicrecon
