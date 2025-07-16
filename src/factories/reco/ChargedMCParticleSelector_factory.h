// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 - 2025 Wouter Deconinck, Dmitry Kalinkin, Derek Anderson

#pragma once

#include "extensions/jana/JOmniFactory.h"
#include "algorithms/reco/ChargedMCParticleSelector.h"

namespace eicrecon {

class ChargedMCParticleSelector_factory
    : public JOmniFactory<ChargedMCParticleSelector_factory, NoConfig> {
public:
  using AlgoT = eicrecon::ChargedMCParticleSelector;

private:
  // algorithm
  std::unique_ptr<AlgoT> m_algo;

  // input collection
  PodioInput<edm4hep::MCParticle> m_pars_in{this, "GeneratedParticles"};

  // output collection
  PodioOutput<edm4hep::MCParticle> m_pars_out{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, int64_t /* event_number */) {
    m_algo->process({m_pars_in()}, {m_pars_out().get()});
  }
};

} // namespace eicrecon
