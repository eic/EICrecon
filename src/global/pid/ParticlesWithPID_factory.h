// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright 2024, Dmitry Kalinkin

#pragma once

#include <memory>
#include <edm4eic/ReconstructedParticle.h>
#include <edm4eic/CherenkovParticleID.h>
#include <edm4hep/ParticleID.h>
#include "algorithms/pid/ParticlesWithPID.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class ParticlesWithPID_factory : public JOmniFactory<ParticlesWithPID_factory, ParticlesWithPIDConfig> {
public:
using AlgoT = eicrecon::ParticlesWithPID;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4eic::ReconstructedParticle> m_recoparticles_input {this};
  PodioInput<edm4eic::CherenkovParticleID> m_cherenkov_particle_ids_input {this};
  PodioOutput<edm4eic::ReconstructedParticle> m_recoparticles_output {this};
  PodioOutput<edm4hep::ParticleID> m_pids_output {this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(this->GetPrefix());
    m_algo->level((algorithms::LogLevel)logger()->level());
    m_algo->applyConfig(config());
    m_algo->init();
  };

  void Process(int64_t run_number, uint64_t event_number) {
    m_algo->process(
      {m_recoparticles_input(), m_cherenkov_particle_ids_input()},
      {m_recoparticles_output().get(), m_pids_output().get()}
    );
  }
};

}
