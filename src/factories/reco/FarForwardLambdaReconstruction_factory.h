// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Sebouh Paul
// Update/modification 2026 by Baptiste Fraisse

#pragma once

#include "algorithms/reco/FarForwardLambdaReconstruction.h"
#include "algorithms/reco/FarForwardLambdaReconstructionConfig.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class FarForwardLambdaReconstruction_factory
    : public JOmniFactory<FarForwardLambdaReconstruction_factory,
                          FarForwardLambdaReconstructionConfig> {

public:
  using AlgoT = eicrecon::FarForwardLambdaReconstruction;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4eic::ReconstructedParticle> m_hcal_neutrals_input{this};
  PodioInput<edm4eic::ReconstructedParticle> m_b0_neutrals_input{this};
  PodioInput<edm4eic::ReconstructedParticle> m_ecalendcapp_neutrals_input{this};
  PodioInput<edm4eic::ReconstructedParticle> m_lfhcal_neutrals_input{this};

  PodioOutput<edm4eic::ReconstructedParticle> m_lambda_output{this};
  PodioOutput<edm4eic::ReconstructedParticle> m_lambda_decay_products_cm_output{this};

  ParameterRef<std::string> m_offset_position_name{this, "offsetPositionName",
                                                   config().offsetPositionName};
  ParameterRef<double> m_rot_y{this, "globalToProtonRotation", config().globalToProtonRotation};
  ParameterRef<double> m_lambda_max_mass_dev{this, "lambdaMassWindow", config().lambdaMassWindow};
  ParameterRef<double> m_pi0_max_mass_dev{this, "pi0Window", config().pi0Window};
  ParameterRef<int> m_iterations{this, "iterations", config().iterations};
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

        {m_hcal_neutrals_input(), m_b0_neutrals_input(), m_ecalendcapp_neutrals_input(),
         m_lfhcal_neutrals_input()},

        {m_lambda_output().get(), m_lambda_decay_products_cm_output().get()});
  }
};

} // namespace eicrecon
