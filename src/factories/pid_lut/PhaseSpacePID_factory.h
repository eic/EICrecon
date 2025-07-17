// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025, Simon Gardner

#pragma once

#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/ParticleIDCollection.h>

#include "algorithms/pid_lut/PhaseSpacePID.h"
#include "extensions/jana/JOmniFactory.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"

namespace eicrecon {

class PhaseSpacePID_factory : public JOmniFactory<PhaseSpacePID_factory, PhaseSpacePIDConfig> {
public:
  using AlgoT = eicrecon::PhaseSpacePID;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4eic::ReconstructedParticle> m_recoparticles_input{this};
  PodioInput<edm4eic::MCRecoParticleAssociation> m_recoparticle_assocs_input{this};
  PodioOutput<edm4eic::ReconstructedParticle> m_recoparticles_output{this};
  PodioOutput<edm4eic::MCRecoParticleAssociation> m_recoparticle_assocs_output{this};
  PodioOutput<edm4hep::ParticleID> m_particleids_output{this};

  ParameterRef<std::string> m_system{this, "system", config().system, "For the ParticleID record"};
  ParameterRef<std::vector<float>> m_direction{
      this, "direction", config().direction,
      "Direction vector for the phase space (default is along z-axis)"};
  ParameterRef<double> m_opening_angle{this, "openingAngle", config().opening_angle,
                                       "Opening angle for the phase space in radians"};
  ParameterRef<int> m_pdg_value{
      this, "pdgValue", config().pdg_value,
      "PDG value for the particle type to identify (default is electron)"};

  Service<AlgorithmsInit_service> m_algorithmsInit{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(this->GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void ChangeRun(int32_t /* run_number */) {}

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_recoparticles_input(), m_recoparticle_assocs_input()},
                    {m_recoparticles_output().get(), m_recoparticle_assocs_output().get(),
                     m_particleids_output().get()});
  }
};

} // namespace eicrecon
