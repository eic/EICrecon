// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Daniel Brandenburg

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <string>
#include <string_view>

#include "services/particle/ParticleSvc.h"

namespace eicrecon {

using ScatteredElectronsTruthAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4hep::MCParticleCollection, edm4eic::ReconstructedParticleCollection,
                      edm4eic::MCRecoParticleAssociationCollection>,
    algorithms::Output<edm4eic::ReconstructedParticleCollection>>;

class ScatteredElectronsTruth : public ScatteredElectronsTruthAlgorithm {

public:
  ScatteredElectronsTruth(std::string_view name)
      : ScatteredElectronsTruthAlgorithm{name,
                                         {"MCParticles", "inputParticles", "inputAssociations"},
                                         {"ReconstructedParticles"},
                                         "Output a list of possible scattered electrons using "
                                         "truth MC Particle associations."} {}

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  const algorithms::ParticleSvc& m_particleSvc = algorithms::ParticleSvc::instance();
};

} // namespace eicrecon
