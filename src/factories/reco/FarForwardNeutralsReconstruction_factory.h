// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Sebouh Paul

#pragma once

#include "algorithms/reco/FarForwardNeutralsReconstruction.h"
#include "algorithms/reco/FarForwardNeutralsReconstructionConfig.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class FarForwardNeutralsReconstruction_factory
    : public JOmniFactory<FarForwardNeutralsReconstruction_factory,
                          FarForwardNeutralsReconstructionConfig> {

public:
  using AlgoT = eicrecon::FarForwardNeutralsReconstruction;

private:
  std::unique_ptr<AlgoT> m_algo;
  PodioInput<edm4eic::Cluster> m_clusters_hcal_input{this};
  PodioOutput<edm4eic::ReconstructedParticle> m_neutrals_output{this};

  ParameterRef<std::string> m_offset_position_name{this, "offsetPositionName",
                                                   config().offsetPositionName};
  ParameterRef<std::vector<double>> m_n_scale_corr_coeff_hcal{this, "neutronScaleCorrCoeffHcal",
                                                              config().neutronScaleCorrCoeffHcal};
  ParameterRef<std::vector<double>> m_gamma_scale_corr_coeff_hcal{this, "gammaScaleCorrCoeffHcal",
                                                                  config().gammaScaleCorrCoeffHcal};
  ParameterRef<double> m_global_to_proton_rotation{this, "globalToProtonRotation",
                                                   config().globalToProtonRotation};
  ParameterRef<double> m_gamma_zmax_offset{this, "gammaZMaxOffset", config().gammaZMaxOffset};

  ParameterRef<double> m_gamma_max_length{this, "gammaMaxLength", config().gammaMaxLength};
  ParameterRef<double> m_gamma_max_width{this, "gammaMaxWidth", config().gammaMaxWidth};

  Service<AlgorithmsInit_service> m_algorithmsInit{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level((algorithms::LogLevel)logger()->level());

    m_algo->applyConfig(config());
    m_algo->init();
  }

  void ChangeRun(int32_t /* run_number */) {}

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process(
        {
            m_clusters_hcal_input(),
        },
        {
            m_neutrals_output().get(),
        });
  }
};

} // namespace eicrecon
