// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Minjung Kim, Barak Schmookler
#pragma once

#include <ActsExamples/EventData/Track.hpp>
#include <memory>
#include <tuple>
#include <vector>

#include "Acts/AmbiguityResolution/GreedyAmbiguityResolution.hpp"
#include "AmbiguitySolverConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace ActsExamples {
struct Trajectories;
}
namespace Acts {
class Logger;
}
namespace edm4eic {
class Measurement2DCollection;
}
namespace spdlog {
class logger;
}

namespace eicrecon {

/*Reco Track Filtering Based on Greedy ambiguity resolution solver adopted from ACTS*/
class AmbiguitySolver : public WithPodConfig<eicrecon::AmbiguitySolverConfig> {
public:
  AmbiguitySolver();

  void init(std::shared_ptr<spdlog::logger> log);

  std::tuple<std::vector<ActsExamples::ConstTrackContainer*>,
             std::vector<ActsExamples::Trajectories*>>
  process(std::vector<const ActsExamples::ConstTrackContainer*> input_container,
          const edm4eic::Measurement2DCollection& meas2Ds);

private:
  std::shared_ptr<spdlog::logger> m_log;
  Acts::GreedyAmbiguityResolution::Config m_acts_cfg;
  std::unique_ptr<Acts::GreedyAmbiguityResolution> m_core;
  /// Private access to the logging instance
  std::shared_ptr<const Acts::Logger> m_acts_logger{nullptr};
  const Acts::Logger& logger() const { return *m_acts_logger; }
};

} // namespace eicrecon
