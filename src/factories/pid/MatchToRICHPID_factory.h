// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright 2024, Dmitry Kalinkin

#pragma once

#ifndef EICRECON_FACTORY_PRECOMPILE
// Preprocessor-based precompilation pattern:
// When EICRECON_FACTORY_PRECOMPILE is not defined, plugin code sees only
// forward declarations and extern templates for fast compilation.
// The full definition is compiled once into a precompile library.

namespace eicrecon {
class MatchToRICHPID_factory;
}

extern template class JOmniFactory<eicrecon::MatchToRICHPID_factory, NoConfig>;

#else
// Full factory definition: compiled into precompile library

#include "algorithms/pid/MatchToRICHPID.h"
#include "extensions/jana/JOmniFactory.h"
#include <edm4eic/CherenkovParticleID.h>
#include <edm4eic/EDM4eicVersion.h>
#include <edm4eic/ReconstructedParticle.h>
#include <edm4eic/MCRecoParticleAssociation.h>
#include <edm4hep/ParticleID.h>
#include <memory>

namespace eicrecon {

class MatchToRICHPID_factory : public JOmniFactory<MatchToRICHPID_factory, MatchToRICHPIDConfig> {
public:
  using AlgoT = eicrecon::MatchToRICHPID;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4eic::ReconstructedParticle> m_recoparticles_input{this};
  PodioInput<edm4eic::MCRecoParticleAssociation> m_assocs_input{this};
  PodioInput<edm4eic::CherenkovParticleID> m_cherenkov_particle_ids_input{this};
  PodioOutput<edm4eic::ReconstructedParticle> m_recoparticles_output{this};
  PodioOutput<edm4eic::MCRecoParticleLink> m_links_output{this};
  PodioOutput<edm4eic::MCRecoParticleAssociation> m_assocs_output{this};
  PodioOutput<edm4hep::ParticleID> m_pids_output{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(this->GetPrefix());
    m_algo->level((algorithms::LogLevel)logger()->level());
    m_algo->applyConfig(config());
    m_algo->init();
  };

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_recoparticles_input(), m_assocs_input(), m_cherenkov_particle_ids_input()},
                    {m_recoparticles_output().get(), m_links_output().get(),
                     m_assocs_output().get(), m_pids_output().get()});
  }
};

} // namespace eicrecon

#endif // EICRECON_FACTORY_PRECOMPILE
