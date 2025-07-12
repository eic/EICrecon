// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2025 Daniel Brandenburg, Wouter Deconinck

#pragma once

#include "extensions/jana/JOmniFactory.h"

#include "algorithms/reco/ElectronReconstruction.h"

namespace eicrecon {

class ReconstructedElectrons_factory
    : public JOmniFactory<ReconstructedElectrons_factory, ElectronReconstructionConfig> {
public:
  using AlgoT = eicrecon::ElectronReconstruction;

private:
  // Underlying algorithm
  std::unique_ptr<AlgoT> m_algo;

  // Declare inputs
  PodioInput<edm4eic::ReconstructedParticle> m_in_rc_particles{this, "ReconstructedParticles"};

  // Declare outputs
  PodioOutput<edm4eic::ReconstructedParticle> m_out_reco_particles{this};

  // Declare parameters
  ParameterRef<double> m_min_energy_over_momentum{this, "minEnergyOverMomentum",
                                                  config().min_energy_over_momentum};
  ParameterRef<double> m_max_energy_over_momentum{this, "maxEnergyOverMomentum",
                                                  config().max_energy_over_momentum};

public:
  void Configure() {
    // This is called when the factory is instantiated.
    // Use this callback to make sure the algorithm is configured.
    // The logger, parameters, and services have all been fetched before this is called
    m_algo = std::make_unique<AlgoT>(GetPrefix());

    // Pass config object to algorithm
    m_algo->applyConfig(config());

    m_algo->init();
  }

  void ChangeRun(int32_t /* run_number */) {
    // This is called whenever the run number is changed.
    // Use this callback to retrieve state that is keyed off of run number.
    // This state should usually be managed by a Service.
    // Note: You usually don't need this, because you can declare a Resource instead.
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    // This is called on every event.
    // Use this callback to call your Algorithm using all inputs and outputs
    // The inputs will have already been fetched for you at this point.
    m_algo->process({m_in_rc_particles()}, {m_out_reco_particles().get()});

    logger()->debug("Found {} reconstructed electron candidates", m_out_reco_particles()->size());
  }
};
} // namespace eicrecon
