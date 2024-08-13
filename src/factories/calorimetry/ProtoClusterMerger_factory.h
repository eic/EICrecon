// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Tristan Protzman

#pragma once

#include <algorithms/calorimetry/ProtoClusterMerger.h>
#include <algorithms/calorimetry/ProtoClusterMergerConfig.h>

#include <edm4eic/ProtoClusterCollection.h>

#include <extensions/jana/JOmniFactory.h>





namespace eicrecon {
    class ProtoClusterMerger_factory : public JOmniFactory<ProtoClusterMerger_factory, ProtoClusterMergerConfig> {
    public:
        using AlgoT = eicrecon::ProtoClusterMerger;
    
        void Configure() {
            m_algo = std::make_unique<AlgoT>(GetPrefix());
            m_algo->level(static_cast<algorithms::LogLevel(logger()->level()));
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

        PodioInput<edm4eic::ProtoClusterCollection> m_collection_a {this};
        PodioInput<edm4eic::ProtoClusterCollection> m_collection_b {this};

        PodioOutput<edm4eic::ProtoClusterCollection> m_output {this};
    };

}   // eicrecon