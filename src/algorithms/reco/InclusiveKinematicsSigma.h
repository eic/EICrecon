// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022, 2023 Sylvester Joosten, Dmitry Romanov, Wouter Deconinck

#pragma once

#include <algorithms/algorithm.h>
#include <string>
#include <string_view>

#include "algorithms/interfaces/ParticleSvc.h"

namespace edm4eic {
class HadronicFinalStateCollection;
}
namespace edm4eic {
class InclusiveKinematicsCollection;
}
namespace edm4eic {
class ReconstructedParticleCollection;
}
namespace edm4hep {
class MCParticleCollection;
}

namespace eicrecon {

using InclusiveKinematicsSigmaAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4hep::MCParticleCollection, edm4eic::ReconstructedParticleCollection,
                      edm4eic::HadronicFinalStateCollection>,
    algorithms::Output<edm4eic::InclusiveKinematicsCollection>>;

class InclusiveKinematicsSigma : public InclusiveKinematicsSigmaAlgorithm {

public:
  InclusiveKinematicsSigma(std::string_view name)
      : InclusiveKinematicsSigmaAlgorithm{
            name,
            {"MCParticles", "scatteredElectron", "hadronicFinalState"},
            {"inclusiveKinematics"},
            "Determine inclusive kinematics using Sigma method."} {}

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  const algorithms::ParticleSvc& m_particleSvc = algorithms::ParticleSvc::instance();
  double m_crossingAngle{-0.025};
};

} // namespace eicrecon
