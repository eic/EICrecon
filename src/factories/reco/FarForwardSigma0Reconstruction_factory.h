// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Sebouh Paul

#pragma once

#include "algorithms/reco/FarForwardSigma0Reconstruction.h"
#include "algorithms/reco/FarForwardSigma0ReconstructionConfig.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class FarForwardSigma0Reconstruction_factory
    : public JOmniFactory<FarForwardSigma0Reconstruction_factory,
                          FarForwardSigma0ReconstructionConfig> {

public:
  using AlgoT = eicrecon::FarForwardSigma0Reconstruction;

private:
  std::unique_ptr<AlgoT> m_algo;
  PodioInput<edm4eic::ReconstructedParticle> m_neutrals_input{this};
  PodioInput<edm4eic::ReconstructedParticle> m_lambdas_input{this};
  PodioOutput<edm4eic::ReconstructedParticle> m_sigma0s_output{this};
  PodioOutput<edm4eic::ReconstructedParticle> m_sigma0s_decay_products_cm_output{this};


  ParameterRef<double> m_sigma0_max_mass_dev{this, "sigma0MaxMassDev", config().sigma0MaxMassDev};

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
    m_algo->process({m_neutrals_input(),m_lambdas_input()},
                    {m_sigma0s_output().get(), m_sigma0s_decay_products_cm_output().get()});
  }
};

} // namespace eicrecon
