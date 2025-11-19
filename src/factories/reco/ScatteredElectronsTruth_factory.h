// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Daniel Brandenburg

#pragma once

#include <JANA/JEvent.h>
#include <edm4eic/InclusiveKinematicsCollection.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "algorithms/reco/ScatteredElectronsTruth.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class ScatteredElectronsTruth_factory : public JOmniFactory<ScatteredElectronsTruth_factory> {

public:
  using AlgoT = eicrecon::ScatteredElectronsTruth;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4hep::MCParticle> m_mc_particles_input{this};
  PodioInput<edm4eic::ReconstructedParticle> m_rc_particles_input{this};
  PodioInput<edm4eic::MCRecoParticleAssociation> m_rc_particles_assoc_input{this};

  // Declare outputs
  PodioOutput<edm4eic::ReconstructedParticle> m_out_reco_particles{this};

  Service<AlgorithmsInit_service> m_algorithmsInit{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_mc_particles_input(), m_rc_particles_input(), m_rc_particles_assoc_input()},
                    {m_out_reco_particles().get()});
  }
};

} // namespace eicrecon
