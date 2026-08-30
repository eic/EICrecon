// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Dmitry Romanov

#pragma once

#include "algorithms/reco/ChargedLambdaReconstruction.h"
#include "algorithms/reco/ChargedLambdaReconstructionConfig.h"
#include "extensions/jana/JOmniFactory.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"

namespace eicrecon {

class ChargedLambdaReconstruction_factory
    : public JOmniFactory<ChargedLambdaReconstruction_factory, ChargedLambdaReconstructionConfig> {

public:
  using AlgoT = eicrecon::ChargedLambdaReconstruction;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4eic::ReconstructedParticle> m_charged_input{this};
  PodioInput<edm4eic::ReconstructedParticle> m_roman_pot_input{this};
  PodioInput<edm4eic::ReconstructedParticle> m_off_momentum_input{this};

  PodioOutput<edm4eic::ReconstructedParticle> m_lambda_output{this};

  ParameterRef<double> m_opening_angle_max{this, "openingAngleMax", config().openingAngleMax};
  ParameterRef<double> m_pair_theta_max{this, "pairThetaMax", config().pairThetaMax};
  ParameterRef<double> m_dca_max{this, "dcaMax", config().dcaMax};
  ParameterRef<double> m_vertex_z_min{this, "vertexZMin", config().vertexZMin};
  ParameterRef<double> m_vertex_z_max{this, "vertexZMax", config().vertexZMax};
  ParameterRef<double> m_pointing_max{this, "pointingMax", config().pointingMax};
  ParameterRef<double> m_mom_asym_min{this, "momAsymMin", config().momAsymMin};
  ParameterRef<double> m_mass_window{this, "massWindow", config().massWindow};

  Service<AlgorithmsInit_service> m_algorithmsInit{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level((algorithms::LogLevel)logger()->level());

    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_charged_input(), m_roman_pot_input(), m_off_momentum_input()},
                    {m_lambda_output().get()});
  }
};

} // namespace eicrecon
