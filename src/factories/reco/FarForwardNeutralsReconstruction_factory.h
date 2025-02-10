// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Sebouh Paul

#pragma once

#include "algorithms/reco/FarForwardNeutralsReconstruction.h"
#include "algorithms/reco/FarForwardNeutralsReconstructionConfig.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "extensions/jana/JOmniFactory.h"


namespace eicrecon {

  class FarForwardNeutralsReconstruction_factory : public JOmniFactory<FarForwardNeutralsReconstruction_factory,FarForwardNeutralsReconstructionConfig> {

   using AlgoT = eicrecon::FarForwardNeutralsReconstruction;
     private:
         std::unique_ptr<AlgoT> m_algo;
    PodioInput<edm4eic::Cluster> m_clusters_hcal_input {this};
    PodioOutput<edm4eic::ReconstructedParticle> m_neutrons_output {this};
    PodioOutput<edm4eic::ReconstructedParticle> m_gammas_output {this};
    ParameterRef<std::vector<double>> m_n_scale_corr_coeff_hcal     {this, "n_scale_corr_coeff_hcal",          config().n_scale_corr_coeff_hcal};
    ParameterRef<std::vector<double>> m_gamma_scale_corr_coeff_hcal     {this, "gamma_scale_corr_coeff_hcal",          config().gamma_scale_corr_coeff_hcal};
    ParameterRef<double> m_rot_y     {this, "rot_y",          config().rot_y};
    ParameterRef<double> m_gamma_zmax     {this, "gamma_zmax",          config().gamma_zmax};

    ParameterRef<double> m_gamma_max_length     {this, "gamma_max_length",          config().gamma_max_length};
    ParameterRef<double> m_gamma_max_width     {this, "gamma_max_width",          config().gamma_max_width};

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
      m_algo->process({m_clusters_hcal_input(),},{m_neutrons_output().get(), m_gammas_output().get()});
    }
};

} // eicrecon
