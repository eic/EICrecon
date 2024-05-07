// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Sebouh Paul

#pragma once

#include "algorithms/reco/FarForwardNeutronReconstruction.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "extensions/jana/JOmniFactory.h"


namespace eicrecon {

class FarForwardNeutronReconstruction_factory : public JOmniFactory<FarForwardNeutronReconstruction_factory,NoConfig> {

   using AlgoT = eicrecon::FarForwardNeutronReconstruction;
     private:
         std::unique_ptr<AlgoT> m_algo;
    PodioInput<edm4eic::Cluster> m_clusters_input {this};
    PodioOutput<edm4eic::ReconstructedParticle> m_neutrons_output {this};

    Service<AlgorithmsInit_service> m_algorithmsInit {this};

public:
    void Configure() {
        m_algo = std::make_unique<AlgoT>(GetPrefix());
        m_algo->level((algorithms::LogLevel)logger()->level());
        m_algo->applyConfig(config());
        m_algo->init();
    }

    void ChangeRun(int64_t run_number) {
    }

    void Process(int64_t run_number, uint64_t event_number) {
      m_algo->process({m_clusters_input()},{m_neutrons_output().get()});
    }
};

} // eicrecon
