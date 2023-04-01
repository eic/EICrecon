// Copyright 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

// General algorithm to merge together particle ID datatypes

#pragma once

// data model
#include <edm4eic/CherenkovParticleIDCollection.h>

// EICrecon
#include "MergeParticleIDConfig.h"
#include <algorithms/interfaces/WithPodConfig.h>
#include <spdlog/spdlog.h>

namespace eicrecon {

  class MergeParticleID : public WithPodConfig<MergeParticleIDConfig> {

    public:
      MergeParticleID() = default;
      ~MergeParticleID() {}

      void AlgorithmInit(std::shared_ptr<spdlog::logger>& logger);
      void AlgorithmChangeRun();

      // AlgorithmProcess
      // - overload this function to support different collections from other PID subsystems, or to support
      //   merging PID results from overlapping subsystems
      std::vector<edm4eic::CherenkovParticleID*> AlgorithmProcess(std::vector<const edm4eic::CherenkovParticleID*>& in_pids);

    private:
      std::shared_ptr<spdlog::logger> m_log;

  };
}
