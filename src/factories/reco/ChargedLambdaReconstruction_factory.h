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

  ParameterRef<double> m_opening_angle_max{this, "openingAngleMax", config().openingAngleMax,
                                           "maximum opening angle between the daughters [rad]"};
  ParameterRef<double> m_pair_theta_max{this, "pairThetaMax", config().pairThetaMax,
                                        "maximum polar angle of the summed pair momentum [rad]"};
  ParameterRef<double> m_dca_max{this, "dcaMax", config().dcaMax,
                                 "maximum distance of closest approach of the track lines [mm]"};
  ParameterRef<double> m_vertex_z_min{this, "vertexZMin", config().vertexZMin,
                                      "minimum z of the decay-vertex proxy [mm]"};
  ParameterRef<double> m_vertex_z_max{this, "vertexZMax", config().vertexZMax,
                                      "maximum z of the decay-vertex proxy [mm]"};
  ParameterRef<double> m_pointing_max{this, "pointingMax", config().pointingMax,
                                      "maximum pointing angle to the decay-vertex proxy [rad]"};
  ParameterRef<double> m_mom_asym_min{this, "momAsymMin", config().momAsymMin,
                                      "minimum momentum asymmetry (p_p - p_pi)/(p_p + p_pi)"};
  ParameterRef<double> m_mass_window{this, "massWindow", config().massWindow,
                                     "half-width of the invariant-mass window [GeV]"};

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
