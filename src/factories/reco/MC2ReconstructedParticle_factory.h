// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <JANA/JEvent.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "algorithms/reco/MC2ReconstructedParticle.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class MC2ReconstructedParticle_factory
    : public JOmniFactory<MC2ReconstructedParticle_factory, NoConfig> {

private:
  using AlgoT = eicrecon::MC2ReconstructedParticle;
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4hep::MCParticle> m_mc_particles_input{this};
  PodioOutput<edm4eic::ReconstructedParticle> m_rc_particles_output{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void ChangeRun(int32_t /* run_number */) {}

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_mc_particles_input()}, {m_rc_particles_output().get()});
  }
};

} // namespace eicrecon
