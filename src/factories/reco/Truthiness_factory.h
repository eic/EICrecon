// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Wouter Deconinck

#pragma once

#include <JANA/JEvent.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#if __has_include(<edm4eic/Truthiness.h>)
#include <edm4eic/TruthinessCollection.h>
#endif

#include "algorithms/reco/Truthiness.h"
#include "extensions/jana/JOmniFactory.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"

namespace eicrecon {

class Truthiness_factory : public JOmniFactory<Truthiness_factory> {

public:
  using FactoryT = JOmniFactory<Truthiness_factory>;

private:
  std::unique_ptr<Truthiness> m_algo;

  FactoryT::PodioInput<edm4hep::MCParticle> m_mc_particles_input{this};
  FactoryT::PodioInput<edm4eic::ReconstructedParticle> m_rc_particles_input{this};
  FactoryT::PodioInput<edm4eic::MCRecoParticleAssociation> m_associations_input{this};

#if __has_include(<edm4eic/Truthiness.h>)
  FactoryT::PodioOutput<edm4eic::Truthiness> m_truthiness_output{this};
#endif

  FactoryT::Service<AlgorithmsInit_service> m_algorithmsInit{this};

public:
  void Configure() {
    m_algo = std::make_unique<Truthiness>(this->GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(this->logger()->level()));
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
#if __has_include(<edm4eic/Truthiness.h>)
    m_algo->process({m_mc_particles_input(), m_rc_particles_input(), m_associations_input()},
                    {m_truthiness_output().get()});
#else
    m_algo->process({m_mc_particles_input(), m_rc_particles_input(), m_associations_input()}, {});
#endif
  }
};

} // namespace eicrecon
