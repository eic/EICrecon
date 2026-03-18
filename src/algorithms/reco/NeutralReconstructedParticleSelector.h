// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/ReconstructedParticleCollection.h>

#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using NeutralReconstructedParticleSelectorAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4eic::ReconstructedParticleCollection>,
                          algorithms::Output<edm4eic::ReconstructedParticleCollection>>;

class NeutralReconstructedParticleSelector : public NeutralReconstructedParticleSelectorAlgorithm,
                                             public WithPodConfig<NoConfig> {

public:
  NeutralReconstructedParticleSelector(std::string_view name)
      : NeutralReconstructedParticleSelectorAlgorithm{name,
                                                      {"inputReconstructedParticles"},
                                                      {"outputReconstructedParticles"},
                                                      "Select neutral reconstructed particles."} {}

  void init() final {}

  void process(const Input& input, const Output& output) const final {
    const auto [rc_particles_in] = input;
    auto [rc_particles_out]      = output;

    for (const auto& particle : *rc_particles_in) {
      if (particle.getCharge() == 0.) {
        rc_particles_out->push_back(particle);
      }
    }
  }
};

} // namespace eicrecon
