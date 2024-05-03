// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, Nathan Brei

#pragma once

#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/ParticleIDCollection.h>

#include "algorithms/pid_lut/PIDLookup.h"
#include "extensions/jana/JOmniFactory.h"
#include "services/pid_lut/PIDLookupTable_service.h"

namespace eicrecon {

class PIDLookup_factory : public JOmniFactory<PIDLookup_factory, PIDLookupConfig> {
public:
  using AlgoT = eicrecon::PIDLookup;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4eic::ReconstructedParticle> m_recoparticles_input{this};
  PodioInput<edm4eic::MCRecoParticleAssociation> m_recoparticle_assocs_input{this};
  PodioOutput<edm4eic::ReconstructedParticle> m_recoparticles_output{this};
  PodioOutput<edm4hep::ParticleID> m_particleids_output{this};

  ParameterRef<std::string> m_filename{this, "filename", config().filename,
                                       "Relative to current working directory"};

  Service<PIDLookupTable_service> m_lut_svc{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(this->GetPrefix());
    m_algo->level((algorithms::LogLevel)logger()->level());
    m_algo->applyConfig(config());
    m_algo->init(this->m_lut_svc());
  }

  void ChangeRun(int64_t run_number) {}

  void Process(int64_t run_number, uint64_t event_number) {
    m_algo->process({m_recoparticles_input(), m_recoparticle_assocs_input()},
                    {m_recoparticles_output().get(), m_particleids_output().get()});
  }
};

} // namespace eicrecon
