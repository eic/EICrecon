// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Sebouh Paul

#pragma once

#include "algorithms/reco/FarForwardNeutronReconstruction.h"
#include "algorithms/reco/FarForwardNeutronReconstructionConfig.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "extensions/jana/JOmniFactory.h"


namespace eicrecon {

  class FarForwardNeutronReconstruction_factory : public JOmniFactory<FarForwardNeutronReconstruction_factory,FarForwardNeutronReconstructionConfig> {

   using AlgoT = eicrecon::FarForwardNeutronReconstruction;
     private:
         std::unique_ptr<AlgoT> m_algo;
    PodioInput<edm4eic::Cluster> m_clusters_hcal_input {this};
    PodioInput<edm4eic::Cluster> m_clusters_ecal_input {this};
    PodioOutput<edm4eic::ReconstructedParticle> m_neutrons_output {this};
    ParameterRef<std::vector<double>> m_scale_corr_coeff_hcal     {this, "scale_corr_coeff_hcal",          config().scale_corr_coeff_hcal};
    ParameterRef<std::vector<double>> m_scale_corr_coeff_ecal     {this, "scale_corr_coeff_ecal",          config().scale_corr_coeff_ecal};
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
      m_algo->process({m_clusters_hcal_input(), m_clusters_ecal_input()},{m_neutrons_output().get()});
    }
};

} // eicrecon
