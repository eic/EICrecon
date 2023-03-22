// Copyright 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

// data model
#include <edm4eic/CherenkovParticleIDCollection.h>
#include <edm4hep/ParticleIDCollection.h>

// EICrecon
#include "ParticleIDConfig.h"
#include <algorithms/interfaces/WithPodConfig.h>
#include <spdlog/spdlog.h>

namespace eicrecon {

  class ParticleID : public WithPodConfig<ParticleIDConfig> {

    public:
      ParticleID() = default;
      ~ParticleID() {}

      void AlgorithmInit(std::shared_ptr<spdlog::logger>& logger);
      void AlgorithmChangeRun();

      /* AlgorithmProcess
       * - produce a `ParticleID` collection, given an input collection
       * - use function overloading to support different input collections from other PID subsystems
       */
      std::vector<edm4hep::ParticleID*> AlgorithmProcess(std::vector<const edm4eic::CherenkovParticleID*>& in_pids);

    private:
      std::shared_ptr<spdlog::logger> m_log;

  };
}
