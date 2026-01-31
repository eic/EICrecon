// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Tyler Kutz

#pragma once

#include <JANA/JEvent.h>
#include <edm4eic/HadronicFinalStateCollection.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "extensions/jana/JOmniFactory.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"

namespace eicrecon {

template <typename AlgoT>
class HadronicFinalState_factory
    : public JOmniFactory<HadronicFinalState_factory<AlgoT>, NoConfig> {

public:
  using FactoryT = JOmniFactory<HadronicFinalState_factory<AlgoT>, NoConfig>;

private:
  std::unique_ptr<AlgoT> m_algo;

  typename FactoryT::template PodioInput<edm4hep::MCParticle, true> m_mc_particles_input{this};
  typename FactoryT::template PodioInput<edm4eic::ReconstructedParticle> m_rc_particles_input{this};
  typename FactoryT::template PodioInput<edm4eic::MCRecoParticleAssociation, true>
      m_rc_particles_assoc_input{this};
  typename FactoryT::template PodioOutput<edm4eic::HadronicFinalState>
      m_hadronic_final_state_output{this};

  typename FactoryT::template Service<AlgorithmsInit_service> m_algorithmsInit{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(this->GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(this->logger()->level()));
    m_algo->applyConfig(this->config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_mc_particles_input(), m_rc_particles_input(), m_rc_particles_assoc_input()},
                    {m_hadronic_final_state_output().get()});
  }
};

} // namespace eicrecon
