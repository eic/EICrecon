// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 John Lajoie

#pragma once

#include <edm4hep/MCParticleCollection.h>
#include <edm4eic/InclusiveKinematicsCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <spdlog/logger.h>
#include <map>
#include <memory>
#include <string>

namespace eicrecon {

  class TransformBreitFrame {

    public:

      // algorithm initialization
      void init(std::shared_ptr<spdlog::logger> logger);

      // run algorithm
      std::unique_ptr<edm4eic::ReconstructedParticleCollection> process(const edm4hep::MCParticleCollection *mcpart,
									const edm4eic::InclusiveKinematicsCollection *kine,
									const edm4eic::ReconstructedParticleCollection *lab_collection);

    private:

      std::shared_ptr<spdlog::logger> m_log;
      double m_proton{0.93827}, m_neutron{0.93957}, m_electron{0.000510998928}, m_crossingAngle{-0.025};

  };  // end TransformBreitFrame definition

}  // end eicrecon namespace
