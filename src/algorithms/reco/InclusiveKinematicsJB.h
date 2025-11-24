// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022, 2023 Sylvester Joosten, Dmitry Romanov, Wouter Deconinck

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/HadronicFinalStateCollection.h>
#include <edm4eic/InclusiveKinematicsCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <string>
#include <string_view>

#include "algorithms/interfaces/WithPodConfig.h"
#include "services/particle/ParticleSvc.h"

namespace eicrecon {

using InclusiveKinematicsJBAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4hep::MCParticleCollection, edm4eic::ReconstructedParticleCollection,
                      edm4eic::HadronicFinalStateCollection>,
    algorithms::Output<edm4eic::InclusiveKinematicsCollection>>;

class InclusiveKinematicsJB : public InclusiveKinematicsJBAlgorithm, public WithPodConfig<NoConfig> {

public:
  InclusiveKinematicsJB(std::string_view name)
      : InclusiveKinematicsJBAlgorithm{
            name,
            {"MCParticles", "scatteredElectron", "hadronicFinalState"},
            {"inclusiveKinematics"},
            "Determine inclusive kinematics using Jacquet-Blondel method."} {}

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  const algorithms::ParticleSvc& m_particleSvc = algorithms::ParticleSvc::instance();
  double m_crossingAngle{-0.025};
};

} // namespace eicrecon
