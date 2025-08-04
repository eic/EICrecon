// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 - 2025 Dmitry Kalinkin, Derek Anderson, Wouter Deconinck

#pragma once

#include <memory>
#include <utility>

#include <algorithms/algorithm.h>
#include <edm4hep/MCParticleCollection.h>
#include <spdlog/logger.h>

#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using ChargedMCParticleSelectorAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4hep::MCParticleCollection>,
                          algorithms::Output<edm4hep::MCParticleCollection>>;

class ChargedMCParticleSelector : public ChargedMCParticleSelectorAlgorithm,
                                  public WithPodConfig<NoConfig> {

public:
  ChargedMCParticleSelector(std::string_view name)
      : ChargedMCParticleSelectorAlgorithm{
            name, {"inputParticles"}, {"outputParticles"}, "select charged particles"} {}

  // algorithm initialization
  void init() final {}

  // primary algorithm call
  void process(const Input& input, const Output& output) const final {
    const auto [mc_particles_in] = input;
    auto [mc_particles_out]      = output;

    mc_particles_out->setSubsetCollection();

    for (const auto& particle : *mc_particles_in) {
      if (particle.getCharge() != 0.) {
        mc_particles_out->push_back(particle);
      }
    }
  }
};

} // namespace eicrecon
