// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck

#pragma once

#include <JANA/JEvent.h>
#include <edm4eic/InclusiveKinematicsCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "algorithms/reco/InclusiveKinematicsTruthDressed.h"
#include "algorithms/reco/InclusiveKinematicsTruthDressedConfig.h"
#include "extensions/jana/JOmniFactory.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"

namespace eicrecon {

class InclusiveKinematicsTruthDressed_factory
    : public JOmniFactory<InclusiveKinematicsTruthDressed_factory,
                          InclusiveKinematicsTruthDressedConfig> {

public:
  using AlgoT = eicrecon::InclusiveKinematicsTruthDressed;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4hep::MCParticle> m_mc_particles_input{this};
  PodioOutput<edm4eic::InclusiveKinematics> m_inclusive_kinematics_output{this};
  PodioOutput<edm4eic::ReconstructedParticle> m_scattered_lepton_output{this};
  PodioOutput<edm4hep::MCParticle> m_scattered_lepton_constituents_output{this};

  ParameterRef<double> m_coneDeltaR{this, "coneDeltaR", config().coneDeltaR};

  Service<AlgorithmsInit_service> m_algorithmsInit{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_mc_particles_input()},
                    {m_inclusive_kinematics_output().get(), m_scattered_lepton_output().get(),
                     m_scattered_lepton_constituents_output().get()});
  }
};

} // namespace eicrecon
