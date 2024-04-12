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

namespace eicrecon {

template <typename AlgoT>
class InclusiveKinematicsReconstructed_factory
    : public JOmniFactory<InclusiveKinematicsReconstructed_factory<AlgoT>> {

public:
  using FactoryT = JOmniFactory<InclusiveKinematicsReconstructed_factory<AlgoT>>;

private:
  std::unique_ptr<AlgoT> m_algo;

  typename FactoryT::template PodioInput<edm4hep::MCParticle> m_mc_particles_input{this};
  typename FactoryT::template PodioInput<edm4eic::ReconstructedParticle> m_rc_particles_input{this};
  typename FactoryT::template PodioInput<edm4eic::MCRecoParticleAssociation>
      m_rc_particles_assoc_input{this};
  typename FactoryT::template PodioOutput<edm4eic::InclusiveKinematics>
      m_inclusive_kinematics_output{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(this->GetPrefix());
    m_algo->init(this->logger());
  }

  void ChangeRun(int64_t run_number) {}

  void Process(int64_t run_number, uint64_t event_number) {
    m_algo->process({m_mc_particles_input(), m_rc_particles_input(), m_rc_particles_assoc_input()},
                    {m_inclusive_kinematics_output().get()});
  }
};

} // namespace eicrecon
