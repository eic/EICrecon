// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 John Lajoie

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/InclusiveKinematicsCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <spdlog/logger.h>
#include <memory>
#include <string>
#include <string_view>

#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

   using TransformBreitFrameAlgorithm = algorithms::Algorithm<
     algorithms::Input<
       edm4hep::MCParticleCollection,
       edm4eic::InclusiveKinematicsCollection,
       edm4eic::ReconstructedParticleCollection
       >,
     algorithms::Output<
       edm4eic::ReconstructedParticleCollection
       >
   >;

  class TransformBreitFrame
    : public TransformBreitFrameAlgorithm,
      public WithPodConfig<NoConfig> {

    public:

    TransformBreitFrame(std::string_view name) :
      TransformBreitFrameAlgorithm {
        name,
        {"inputMCParticles", "inputInclusiveKinematics", "inputReconstructedParticles"},
        {"outputReconstructedParticles"},
        "Transforms a set of particles from the lab frame to the Breit frame"
      } {}

      // algorithm initialization
      void init(std::shared_ptr<spdlog::logger> logger);

      // run algorithm
      void process(const Input&, const Output&) const final;

    private:

      std::shared_ptr<spdlog::logger> m_log;
      double m_proton{0.93827}, m_neutron{0.93957}, m_electron{0.000510998928}, m_crossingAngle{-0.025};

  };  // end TransformBreitFrame definition

}  // end eicrecon namespace
