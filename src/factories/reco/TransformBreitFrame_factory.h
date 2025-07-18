// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 John Lajoie

#pragma once

#include <JANA/JEvent.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4eic/InclusiveKinematicsCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "algorithms/reco/TransformBreitFrame.h"
#include "extensions/jana/JOmniFactory.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"

namespace eicrecon {

class TransformBreitFrame_factory : public JOmniFactory<TransformBreitFrame_factory> {

public:
  // algorithm to run
  using Algo = eicrecon::TransformBreitFrame;

private:
  std::unique_ptr<Algo> m_algo;

  // input collection
  PodioInput<edm4hep::MCParticle> m_in_mcpart{this};
  PodioInput<edm4eic::InclusiveKinematics> m_in_kine{this};
  PodioInput<edm4eic::ReconstructedParticle> m_in_part{this};

  // output collection
  PodioOutput<edm4eic::ReconstructedParticle> m_out_part{this};

  Service<AlgorithmsInit_service> m_algorithmsInit{this};

public:
  void Configure() {
    m_algo = std::make_unique<Algo>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->init();
  }

  void ChangeRun(int32_t /* run_number */) { /* nothing to do */ }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_in_mcpart(), m_in_kine(), m_in_part()}, {m_out_part().get()});
  }

}; // end TransfromBreitFrame_factory definition

} // namespace eicrecon
