// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2025 Dmitry Romanov, Dmitry Kalinkin

#pragma once

#include <ActsPodioEdm/BoundParametersCollection.h>
#include <ActsPodioEdm/JacobianCollection.h>
#include <ActsPodioEdm/TrackCollection.h>
#include <ActsPodioEdm/TrackStateCollection.h>
#include <ActsPodioEdm/TrackStateHitLinkCollection.h>
#include <algorithms/algorithm.h>
#include <edm4eic/TrackCollection.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <memory>
#include <string>
#include <string_view>

#include "ActsGeometryProvider.h"
#include "algorithms/interfaces/ActsSvc.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using TrackProjectorAlgorithm = algorithms::Algorithm<
    algorithms::Input<ActsPodioEdm::TrackStateCollection, ActsPodioEdm::BoundParametersCollection,
                      ActsPodioEdm::JacobianCollection, ActsPodioEdm::TrackStateHitLinkCollection,
                      ActsPodioEdm::TrackCollection, edm4eic::TrackCollection>,
    algorithms::Output<edm4eic::TrackSegmentCollection>>;

class TrackProjector : public TrackProjectorAlgorithm, public WithPodConfig<NoConfig> {
public:
  TrackProjector(std::string_view name)
      : TrackProjectorAlgorithm{name,
                                {"inputActsTrackStates", "inputActsTrackParameters",
                                 "inputActsTrackJacobians", "inputActsTrackStateHitLinks",
                                 "inputActsTracks", "inputTracks"},
                                {"outputTrackSegments"},
                                "Exports track states as segments"} {}

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  const algorithms::ActsSvc& m_actsSvc{algorithms::ActsSvc::instance()};
  std::shared_ptr<const ActsGeometryProvider> m_geo_provider{m_actsSvc.acts_geometry_provider()};
};

} // namespace eicrecon
