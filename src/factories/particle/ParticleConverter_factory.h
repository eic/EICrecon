// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Esteban Molina, Derek Anderson

#pragma once

#include <string>
#include "extensions/jana/JOmniFactory.h"

#include <edm4eic/Track.h>
#include <edm4eic/Cluster.h>
#include <edm4eic/TrackClusterMatch.h>
#include <edm4eic/ReconstructedParticle.h>

#include "algorithms/particle/ParticleConverter.h"
#include "algorithms/particle/ParticleConverterConfig.h"

namespace eicrecon {
class ParticleConverter_factory
    : public JOmniFactory<ParticleConverter_factory, ParticleConverterConfig> {
public:
  // Class associated to the algorithm
  using Algo = eicrecon::ParticleConverter;

private:
  std::unique_ptr<Algo> m_algo;

  // Input collections
  PodioInput<edm4eic::ReconstructedParticle> m_recoparticles_input{this};

  // Output collection
  // - Reconstructed particles
  PodioOutput<edm4eic::ReconstructedParticle> m_recoparticles_output{this};

  // Parameters
  ParameterRef<double> m_tracking_resolution{this, "tracking_resolution",
                                             config().tracking_resolution};
  ParameterRef<double> m_ecal_resolution{this, "ecal_resolution", config().ecal_resolution};
  ParameterRef<double> m_hcal_resolution{this, "hcal_resolution", config().hcal_resolution};
  ParameterRef<double> m_calo_hadronic_scale{this, "calo_hadron_scale", config().calo_hadron_scale);
  ParameterRef<double> m_calo_energy_norm{this, "calo_energy_norm", config().calo_energy_norm};

  public:
    void Configure() {
      m_algo = std::make_unique<Algo>(this->GetPrefix());
      m_algo->level((algorithms::LogLevel)this->logger()->level());
      m_algo->applyConfig(config());
      m_algo->init();
    };

    void Process(int32_t /* run_number */, uint64_t /* event_number */) {
      m_algo->process({m_recoparticles_input()}, {m_recoparticles_output().get()});
    }
  };
} // namespace eicrecon
