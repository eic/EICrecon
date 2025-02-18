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
    ParameterRef<std::vector<double>> m_n_scale_corr_coeff_hcal     {this, "neutronScaleCorrCoeffHcal",          config().neutronScaleCorrCoeffHcal};
    ParameterRef<std::vector<double>> m_gamma_scale_corr_coeff_hcal     {this, "gammaScaleCorrCoeffHcal",          config().gammaScaleCorrCoeffHcal};
    ParameterRef<double> m_global_to_proton_rotation     {this, "globalToProtonRotation",          config().globalToProtonRotation};
    ParameterRef<double> m_gamma_zmax     {this, "gammaZMax",          config().gammaZMax};

    ParameterRef<double> m_gamma_max_length     {this, "gammaMaxLength",          config().gammaMaxLength};
    ParameterRef<double> m_gamma_max_width     {this, "gammaMaxWidth",          config().gammaMaxWidth};

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
