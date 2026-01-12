// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Minjung Kim, Barak Schmookler
#pragma once

#include <Acts/Utilities/Logger.hpp>
#include <ActsExamples/EventData/Track.hpp>
#include <algorithms/algorithm.h>
#include <edm4eic/Measurement2D.h>
#include <memory>
#include <string_view>

#include "Acts/AmbiguityResolution/GreedyAmbiguityResolution.hpp"
#include "AmbiguitySolverConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using AmbiguitySolverAlgorithm = algorithms::Algorithm<
    algorithms::Input<Acts::ConstVectorMultiTrajectory, Acts::ConstVectorTrackContainer,
                      edm4eic::Measurement2DCollection>,
    algorithms::Output<Acts::ConstVectorMultiTrajectory*, Acts::ConstVectorTrackContainer*>>;

/*Reco Track Filtering Based on Greedy ambiguity resolution solver adopted from ACTS*/
class AmbiguitySolver : public AmbiguitySolverAlgorithm,
                        public WithPodConfig<eicrecon::AmbiguitySolverConfig> {
public:
  AmbiguitySolver(std::string_view name)
      : AmbiguitySolverAlgorithm{name,
                                 {"inputActsTrackStates", "inputActsTracks", "inputMeasurements"},
                                 {"outputActsTrackStates", "outputActsTracks"},
                                 "Greedy ambiguity resolution for tracks"} {}

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  Acts::GreedyAmbiguityResolution::Config m_acts_cfg;
  std::unique_ptr<Acts::GreedyAmbiguityResolution> m_core;
  /// Private access to the logging instance
  std::shared_ptr<const Acts::Logger> m_acts_logger{nullptr};
  const Acts::Logger& acts_logger() const { return *m_acts_logger; }
};

} // namespace eicrecon
