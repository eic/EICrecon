// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, Nathan Brei

#pragma once

#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/EDM4eicVersion.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/ParticleIDCollection.h>

#include "algorithms/pid_lut/PIDLookup.h"
#include "extensions/jana/JOmniFactory.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"

namespace eicrecon {

class PIDLookup_factory : public JOmniFactory<PIDLookup_factory, PIDLookupConfig> {
public:
  using AlgoT = eicrecon::PIDLookup;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4hep::EventHeader> m_event_headers_input{this};
  PodioInput<edm4eic::ReconstructedParticle> m_recoparticles_input{this};
  PodioInput<edm4eic::MCRecoParticleAssociation> m_recoparticle_assocs_input{this};
  PodioOutput<edm4eic::ReconstructedParticle> m_recoparticles_output{this};
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
  PodioOutput<edm4eic::MCRecoParticleLink> m_recoparticle_links_output{this};
#endif
  PodioOutput<edm4eic::MCRecoParticleAssociation> m_recoparticle_assocs_output{this};
  PodioOutput<edm4hep::ParticleID> m_particleids_output{this};

  ParameterRef<std::string> m_filename{this, "filename", config().filename,
                                       "Relative to current working directory"};
  ParameterRef<std::string> m_system{this, "system", config().system, "For the ParticleID record"};

  Service<AlgorithmsInit_service> m_algorithmsInit{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(this->GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process(
        {m_event_headers_input(), m_recoparticles_input(), m_recoparticle_assocs_input()},
        {m_recoparticles_output().get(),
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
         m_recoparticle_links_output().get(),
#endif
         m_recoparticle_assocs_output().get(), m_particleids_output().get()});
  }
};

} // namespace eicrecon
