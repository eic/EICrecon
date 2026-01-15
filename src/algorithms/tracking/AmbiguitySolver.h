// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Minjung Kim, Barak Schmookler
#pragma once

#include <Acts/AmbiguityResolution/GreedyAmbiguityResolution.hpp>
#include <Acts/EventData/VectorMultiTrajectory.hpp>
#include <Acts/EventData/VectorTrackContainer.hpp>
#include <Acts/Utilities/Logger.hpp>
#include <ActsExamples/EventData/Track.hpp>
#include <ActsPodioEdm/TrackCollection.h>
#include <ActsPodioEdm/TrackStateCollection.h>
#include <ActsPodioEdm/BoundParametersCollection.h>
#include <ActsPodioEdm/JacobianCollection.h>
#include <algorithms/algorithm.h>
#include <memory>
#include <string>
#include <string_view>

#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/interfaces/ActsSvc.h"
#include "algorithms/tracking/AmbiguitySolverConfig.h"

class ActsGeometryProvider;

namespace eicrecon {

using AmbiguitySolverAlgorithm = algorithms::Algorithm<
    algorithms::Input<ActsPodioEdm::TrackStateCollection, ActsPodioEdm::BoundParametersCollection,
                      ActsPodioEdm::JacobianCollection, ActsPodioEdm::TrackCollection>,
    algorithms::Output<ActsPodioEdm::TrackStateCollection, ActsPodioEdm::BoundParametersCollection,
                       ActsPodioEdm::JacobianCollection, ActsPodioEdm::TrackCollection>>;

/*Reco Track Filtering Based on Greedy ambiguity resolution solver adopted from ACTS*/
class AmbiguitySolver : public AmbiguitySolverAlgorithm,
                        public WithPodConfig<eicrecon::AmbiguitySolverConfig> {
public:
  AmbiguitySolver(std::string_view name)
      : AmbiguitySolverAlgorithm{name,
                                 {"inputActsTrackStates", "inputActsTrackParameters",
                                  "inputActsTrackJacobians", "inputActsTracks"},
                                 {"outputActsTrackStates", "outputActsTrackParameters",
                                  "outputActsTrackJacobians", "outputActsTracks"},
                                 "Greedy ambiguity resolution for tracks"} {}

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  const algorithms::ActsSvc& m_actsSvc{algorithms::ActsSvc::instance()};
  std::shared_ptr<const ActsGeometryProvider> m_geoSvc{m_actsSvc.acts_geometry_provider()};
  Acts::GreedyAmbiguityResolution::Config m_acts_cfg;
  std::unique_ptr<Acts::GreedyAmbiguityResolution> m_core;
  /// Private access to the logging instance
  std::shared_ptr<const Acts::Logger> m_acts_logger{nullptr};
  const Acts::Logger& acts_logger() const { return *m_acts_logger; }
};

} // namespace eicrecon
