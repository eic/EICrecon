// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Wouter Deconinck

#pragma once

#include <Acts/EventData/VectorMultiTrajectory.hpp>
#include <Acts/EventData/VectorTrackContainer.hpp>
#include <ActsPodioEdm/BoundParametersCollection.h>
#include <ActsPodioEdm/JacobianCollection.h>
#include <ActsPodioEdm/TrackCollection.h>
#include <ActsPodioEdm/TrackStateCollection.h>
#include <algorithms/algorithm.h>
#include <gsl/pointers>
#include <string>
#include <string_view>

#include "algorithms/interfaces/ActsSvc.h"
#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/tracking/ActsGeometryProvider.h"

namespace eicrecon {

using ActsTrackMergerAlgorithm = algorithms::Algorithm<
    algorithms::Input<ActsPodioEdm::TrackStateCollection, ActsPodioEdm::BoundParametersCollection,
                      ActsPodioEdm::JacobianCollection, ActsPodioEdm::TrackCollection,
                      ActsPodioEdm::TrackStateCollection, ActsPodioEdm::BoundParametersCollection,
                      ActsPodioEdm::JacobianCollection, ActsPodioEdm::TrackCollection>,
    algorithms::Output<ActsPodioEdm::TrackStateCollection, ActsPodioEdm::BoundParametersCollection,
                       ActsPodioEdm::JacobianCollection, ActsPodioEdm::TrackCollection>>;

class ActsTrackMerger : public ActsTrackMergerAlgorithm, public WithPodConfig<NoConfig> {
public:
  ActsTrackMerger(std::string_view name)
      : ActsTrackMergerAlgorithm{name,
                                 {
                                     "inputActsTrackStates1",
                                     "inputActsTrackParameters1",
                                     "inputActsTrackJacobians1",
                                     "inputActsTracks1",
                                     "inputActsTrackStates2",
                                     "inputActsTrackParameters2",
                                     "inputActsTrackJacobians2",
                                     "inputActsTracks2",
                                 },
                                 {
                                     "outputActsTrackStates",
                                     "outputActsTrackParameters",
                                     "outputActsTrackJacobians",
                                     "outputActsTracks",
                                 },
                                 "Merges two Acts track container inputs into one"} {};

  void init() final {};
  void process(const Input&, const Output&) const final;

private:
  const algorithms::ActsSvc& m_actsSvc{algorithms::ActsSvc::instance()};
  std::shared_ptr<const ActsGeometryProvider> m_geoSvc{m_actsSvc.acts_geometry_provider()};
};

} // namespace eicrecon
