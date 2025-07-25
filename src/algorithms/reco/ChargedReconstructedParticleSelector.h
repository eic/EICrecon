// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 - 2025 Dmitry Kalinkin, Derek Anderson, Wouter Deconinck

#pragma once

#include <memory>
#include <utility>

#include <algorithms/algorithm.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <spdlog/logger.h>

#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using ChargedReconstructedParticleSelectorAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4eic::ReconstructedParticleCollection>,
                          algorithms::Output<edm4eic::ReconstructedParticleCollection>>;

class ChargedReconstructedParticleSelector : public ChargedReconstructedParticleSelectorAlgorithm,
                                             public WithPodConfig<NoConfig> {

public:
  ChargedReconstructedParticleSelector(std::string_view name)
      : ChargedReconstructedParticleSelectorAlgorithm{
            name, {"inputParticles"}, {"outputParticles"}, "select charged particles"} {}

  // algorithm initialization
  void init() final {}

  // primary algorithm call
  void process(const Input& input, const Output& output) const final {
    const auto [rc_particles_in] = input;
    auto [rc_particles_out]      = output;
    rc_particles_out->setSubsetCollection();

    for (const auto& particle : *rc_particles_in) {
      if (particle.getCharge() != 0.) {
        rc_particles_out->push_back(particle);
      }
    }
  }
};

} // namespace eicrecon
