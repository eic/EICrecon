// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Tristan Protzman

#pragma once

#include <algorithms/calorimetry/CrossCaloClusterMerger.h>
#include <algorithms/calorimetry/CrossCaloClusterMergerConfig.h>

#include <edm4eic/Cluster.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/ProtoClusterCollection.h>

#include <extensions/jana/JOmniFactory.h>





namespace eicrecon {
    class CrossCaloClusterMerger_factory : public JOmniFactory<CrossCaloClusterMerger_factory, CrossCaloClusterMergerConfig> {
    public:
        using AlgoT = eicrecon::CrossCaloClusterMerger;
    
        void Configure() {
            m_algo = std::make_unique<AlgoT>();
            m_algo->init(logger());
            m_algo->applyConfig(config());
        }

        void ChangeRun(int64_t run_number) {}

        void Process(int64_t run_number, uint64_t event_number) {
            auto output = m_algo->execute(m_collection_a(), m_collection_b());
            m_output() = std::move(output);
        }

    private:
        std::unique_ptr<AlgoT> m_algo;

        PodioInput<edm4eic::Cluster> m_collection_a {this};
        PodioInput<edm4eic::Cluster> m_collection_b {this};

        PodioOutput<edm4eic::Cluster> m_output {this};
    };

}   // eicrecon