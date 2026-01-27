// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Minjung Kim, Barak Schmookler
#pragma once

#include <Acts/EventData/VectorMultiTrajectory.hpp>
#include <Acts/EventData/VectorTrackContainer.hpp>
#include <Acts/Utilities/Logger.hpp>
#include <edm4eic/Measurement2D.h>
#include <spdlog/logger.h>
#include <memory>
#include <tuple>
#include <variant>
#include <vector>

#include "Acts/AmbiguityResolution/GreedyAmbiguityResolution.hpp"
#include "AmbiguitySolverConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

/*Reco Track Filtering Based on Greedy ambiguity resolution solver adopted from ACTS*/
class AmbiguitySolver : public WithPodConfig<eicrecon::AmbiguitySolverConfig> {
public:
  AmbiguitySolver();

  void init(std::shared_ptr<spdlog::logger> log);

  std::tuple<std::vector<Acts::ConstVectorMultiTrajectory*>,
             std::vector<Acts::ConstVectorTrackContainer*>>
  process(std::vector<const Acts::ConstVectorMultiTrajectory*> input_track_states,
          std::vector<const Acts::ConstVectorTrackContainer*> input_tracks,
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
