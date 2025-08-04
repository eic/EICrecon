// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 John Lajoie

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/InclusiveKinematicsCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <string>
#include <string_view>

#include "algorithms/interfaces/ParticleSvc.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using TransformBreitFrameAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4hep::MCParticleCollection, edm4eic::InclusiveKinematicsCollection,
                      edm4eic::ReconstructedParticleCollection>,
    algorithms::Output<edm4eic::ReconstructedParticleCollection>>;

class TransformBreitFrame : public TransformBreitFrameAlgorithm, public WithPodConfig<NoConfig> {

public:
  TransformBreitFrame(std::string_view name)
      : TransformBreitFrameAlgorithm{
            name,
            {"inputMCParticles", "inputInclusiveKinematics", "inputReconstructedParticles"},
            {"outputReconstructedParticles"},
            "Transforms a set of particles from the lab frame to the Breit frame"} {}

  // algorithm initialization
  void init() final {};

  // run algorithm
  void process(const Input&, const Output&) const final;

private:
  const algorithms::ParticleSvc& m_particleSvc = algorithms::ParticleSvc::instance();
  double m_crossingAngle{-0.025};

}; // end TransformBreitFrame definition

} // namespace eicrecon
