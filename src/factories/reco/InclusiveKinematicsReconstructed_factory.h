// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Wouter Deconinck

#pragma once

#include <JANA/JEvent.h>
#include <edm4eic/InclusiveKinematicsCollection.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "extensions/jana/JOmniFactory.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"

namespace eicrecon {

template <typename AlgoT>
class InclusiveKinematicsReconstructed_factory
    : public JOmniFactory<InclusiveKinematicsReconstructed_factory<AlgoT>> {

public:
  using FactoryT = JOmniFactory<InclusiveKinematicsReconstructed_factory<AlgoT>>;

private:
  std::unique_ptr<AlgoT> m_algo;

  typename FactoryT::template PodioInput<edm4hep::MCParticle> m_mc_particles_input{this};
  typename FactoryT::template PodioInput<edm4eic::ReconstructedParticle> m_scattered_electron_input{
      this};
  typename FactoryT::template PodioInput<edm4eic::HadronicFinalState> m_hadronic_final_state_input{
      this};
  typename FactoryT::template PodioOutput<edm4eic::InclusiveKinematics>
      m_inclusive_kinematics_output{this};

  typename FactoryT::template Service<AlgorithmsInit_service> m_algorithmsInit{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(this->GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(this->logger()->level()));
    m_algo->applyConfig(this->config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process(
        {m_mc_particles_input(), m_scattered_electron_input(), m_hadronic_final_state_input()},
        {m_inclusive_kinematics_output().get()});
  }
};

} // namespace eicrecon
