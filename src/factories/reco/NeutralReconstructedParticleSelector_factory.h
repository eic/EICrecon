// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024

#pragma once

#include "algorithms/reco/NeutralReconstructedParticleSelector.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class NeutralReconstructedParticleSelector_factory
    : public JOmniFactory<NeutralReconstructedParticleSelector_factory, NeutralReconstructedParticleSelector> {

public:
  using AlgoT = eicrecon::NeutralReconstructedParticleSelector;
  using FactoryT = JOmniFactory<NeutralReconstructedParticleSelector_factory, AlgoT>;

  NeutralReconstructedParticleSelector_factory()
      : FactoryT("NeutralReconstructedParticleSelector", this) {}

  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->init();
  }

  void ChangeRun(int64_t run_number) {}

  void Process(int64_t run_number, uint64_t event_number) {
    m_algo->process({GetCollection<edm4eic::ReconstructedParticleCollection>(0)},
                    {GetCollection<edm4eic::ReconstructedParticleCollection>(0)});
  }
};

} // namespace eicrecon
