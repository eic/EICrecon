// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023, Christopher Dilks

// General algorithm to merge together particle ID datatypes

#pragma once

#include <memory>
#include <vector>

// EICrecon
#include "MergeParticleIDConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace edm4eic { class CherenkovParticleIDCollection; }
namespace spdlog { class logger; }

namespace eicrecon {

  class MergeParticleID : public WithPodConfig<MergeParticleIDConfig> {

    public:
      MergeParticleID() = default;
      ~MergeParticleID() {}

      void AlgorithmInit(std::shared_ptr<spdlog::logger>& logger);
      void AlgorithmChangeRun();

      // AlgorithmProcess
      // - input: a list of particle ID collections, which we want to merge together
      // - output: the merged particle ID collection
      // - overload this function to support different collections from other PID subsystems, or to support
      //   merging PID results from overlapping subsystems
      std::unique_ptr<edm4eic::CherenkovParticleIDCollection> AlgorithmProcess(
          std::vector<const edm4eic::CherenkovParticleIDCollection*> in_pid_collections_list
          );

    private:
      std::shared_ptr<spdlog::logger> m_log;

  };
}
