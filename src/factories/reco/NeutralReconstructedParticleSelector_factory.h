// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024

#pragma once

#include "extensions/jana/JOmniFactory.h"
#include "algorithms/reco/NeutralReconstructedParticleSelector.h"

namespace eicrecon {

class NeutralReconstructedParticleSelector_factory
    : public JOmniFactory<NeutralReconstructedParticleSelector_factory, NoConfig> {
public:
  using AlgoT = eicrecon::NeutralReconstructedParticleSelector;

private:
  // algorithm
  std::unique_ptr<AlgoT> m_algo;

  // input collection
  PodioInput<edm4eic::ReconstructedParticle> m_pars_in{this, "GeneratedParticles"};

  // output collection
  PodioOutput<edm4eic::ReconstructedParticle> m_pars_out{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_pars_in()}, {m_pars_out().get()});
  }
};

} // namespace eicrecon
