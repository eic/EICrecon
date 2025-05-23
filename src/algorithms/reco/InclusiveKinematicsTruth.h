// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022, 2023 Sylvester Joosten, Dmitry Romanov, Wouter Deconinck

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/InclusiveKinematicsCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <string>
#include <string_view>

#include "algorithms/interfaces/ParticleSvc.h"

namespace eicrecon {

using InclusiveKinematicsTruthAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4hep::MCParticleCollection>,
                          algorithms::Output<edm4eic::InclusiveKinematicsCollection>>;

class InclusiveKinematicsTruth : public InclusiveKinematicsTruthAlgorithm {

public:
  InclusiveKinematicsTruth(std::string_view name)
      : InclusiveKinematicsTruthAlgorithm{
            name,
            {"MCParticles"},
            {"inclusiveKinematics"},
            "Determine inclusive kinematics from truth information."} {}

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  const algorithms::ParticleSvc& m_particleSvc = algorithms::ParticleSvc::instance();
};

} // namespace eicrecon
