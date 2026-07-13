// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Sebouh Paul, Baptiste Fraisse

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

  PodioInput<edm4eic::Cluster> m_clusters_input{this};
  PodioOutput<edm4eic::ReconstructedParticle> m_neutrals_output{this};

  ParameterRef<std::string> m_offset_position_name{this, "offsetPositionName",
                                                   config().offsetPositionName};

  ParameterRef<std::vector<double>> m_neutron_scale_corr_coeff{this, "neutronScaleCorrCoeff",
                                                               config().neutronScaleCorrCoeff};

  ParameterRef<std::vector<double>> m_gamma_scale_corr_coeff{this, "gammaScaleCorrCoeff",
                                                             config().gammaScaleCorrCoeff};

  ParameterRef<bool> m_can_detect_gammas{this, "canDetectGammas", config().canDetectGammas};

  ParameterRef<bool> m_can_detect_neutrons{this, "canDetectNeutrons", config().canDetectNeutrons};

  ParameterRef<std::string> m_gamma_mode{this, "gammaMode", config().gammaMode};

  ParameterRef<std::string> m_neutron_mode{this, "neutronMode", config().neutronMode};

  ParameterRef<double> m_cluster_emin{this, "clusterEmin", config().clusterEmin};

  ParameterRef<bool> m_associate_all_clusters_to_neutron{this, "associateAllClustersToNeutron",
                                                         config().associateAllClustersToNeutron};

  ParameterRef<double> m_global_to_proton_rotation{this, "globalToProtonRotation",
                                                   config().globalToProtonRotation};

  ParameterRef<double> m_gamma_zmax_offset{this, "gammaZMaxOffset", config().gammaZMaxOffset};

  ParameterRef<double> m_gamma_max_length{this, "gammaMaxLength", config().gammaMaxLength};

  ParameterRef<double> m_gamma_max_width{this, "gammaMaxWidth", config().gammaMaxWidth};
  
  ParameterRef<double> m_gamma_max_nhits_coeff_lin{this, "gammaMaxNhitsCoeffLin", config().gammaMaxNhitsCoeffLin};
  
  ParameterRef<double> m_gamma_max_nhits_coeff_sqrt{this, "gammaMaxNhitsCoeffSqrt", config().gammaMaxNhitsCoeffSqrt};

  Service<AlgorithmsInit_service> m_algorithmsInit{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level((algorithms::LogLevel)logger()->level());

    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process(
        {
            m_clusters_input(),

        },
        {
            m_neutrals_output().get(),

        });
  }
};

} // namespace eicrecon
