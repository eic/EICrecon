// Copyright 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

// General algorithm to link ParticleID objects with ReconstructedParticles

#pragma once

// general
#include <algorithm>

// data model
#include <edm4eic/CherenkovParticleIDCollection.h>
#include <edm4hep/ParticleIDCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/utils/vector_utils.h>

// DD4hep
#include <Evaluator/DD4hepUnits.h>

// EICrecon
#include "LinkParticleIDConfig.h"
#include "ConvertParticleID.h"
#include "Tools.h"
#include <algorithms/interfaces/WithPodConfig.h>
#include <spdlog/spdlog.h>

namespace eicrecon {

  // output collections
  struct LinkParticleIDResult {
    std::vector<edm4eic::ReconstructedParticle*> particles;
    std::vector<edm4hep::ParticleID*>            pids;
  };

  // algorithm
  class LinkParticleID : public WithPodConfig<LinkParticleIDConfig> {

    public:
      LinkParticleID() = default;
      ~LinkParticleID() {}

      void AlgorithmInit(std::shared_ptr<spdlog::logger>& logger);
      void AlgorithmChangeRun();

      // AlgorithmProcess
      // - overload this function to support different collections from other PID subsystems
      LinkParticleIDResult AlgorithmProcess(
          std::vector<const edm4eic::ReconstructedParticle*>& in_particles,
          std::vector<const edm4eic::CherenkovParticleID*>& in_pids
          );

    private:
      std::shared_ptr<spdlog::logger> m_log;

  };
}
