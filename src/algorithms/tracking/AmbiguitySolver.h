// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Minjung Kim, Barak Schmookler
#pragma once

#include <Acts/Utilities/Logger.hpp>
#include <ActsExamples/EventData/Track.hpp>
#include <ActsExamples/EventData/Trajectories.hpp>
#include <edm4eic/Measurement2D.h>
#include <spdlog/logger.h>
#include <memory>
#include <tuple>
#include <variant>
#include <vector>

#include "Acts/AmbiguityResolution/GreedyAmbiguityResolution.hpp"
#include "AmbiguitySolverConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/tracking/ActsExamplesEdm.h"

namespace eicrecon {

/*Reco Track Filtering Based on Greedy ambiguity resolution solver adopted from ACTS*/
template <typename edm_t = eicrecon::ActsExamplesEdm>
class AmbiguitySolver : public WithPodConfig<eicrecon::AmbiguitySolverConfig> {
private:
  static std::size_t sourceLinkHash(const Acts::SourceLink& a);
  static bool sourceLinkEquality(const Acts::SourceLink& a, const Acts::SourceLink& b);

public:
  AmbiguitySolver() = default;

  void init(std::shared_ptr<spdlog::logger> log);

  std::tuple<std::vector<typename edm_t::ConstTrackContainer*>,
             std::vector<typename edm_t::Trajectories*>>
  process(std::vector<const typename edm_t::ConstTrackContainer*> input_container,
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
