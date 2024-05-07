// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright 2024, Dmitry Kalinkin

#pragma once

#include "algorithms/tracking/TracksToParticles.h"
#include "algorithms/tracking/TracksToParticlesConfig.h"
#include <edm4eic/MCRecoParticleAssociation.h>
#include <edm4eic/ReconstructedParticle.h>
#include <edm4eic/Track.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/ParticleID.h>
#include <memory>

namespace eicrecon {

class TracksToParticles_factory
    : public JOmniFactory<TracksToParticles_factory, TracksToParticlesConfig> {
public:
  using AlgoT = eicrecon::TracksToParticles;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4hep::MCParticle> m_particles_input{this};
  PodioInput<edm4eic::Track> m_tracks_input{this};
  PodioOutput<edm4eic::ReconstructedParticle> m_recoparticles_output{this};
  PodioOutput<edm4eic::MCRecoParticleAssociation> m_recoassocs_output{this};

  ParameterRef<double> m_momentum_relative_tolerance{this, "momentumRelativeTolerance",
                                                     config().momentumRelativeTolerance};
  ParameterRef<double> m_phi_tolerance{this, "phiTolerance", config().phiTolerance};
  ParameterRef<double> m_eta_tolerance{this, "etaTolerance", config().etaTolerance};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(this->GetPrefix());
    m_algo->level((algorithms::LogLevel)logger()->level());
    m_algo->applyConfig(config());
    m_algo->init();
  };

  void ChangeRun(int64_t run_number) {};

  void Process(int64_t run_number, uint64_t event_number) {
    m_algo->process({m_particles_input(), m_tracks_input()},
                    {m_recoparticles_output().get(), m_recoassocs_output().get()});
  }
};

} // namespace eicrecon
