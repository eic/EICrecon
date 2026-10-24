// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Dmitry Kalinkin

#pragma once

#include <ActsPodioEdm/BoundParametersCollection.h>
#include <ActsPodioEdm/JacobianCollection.h>
#include <ActsPodioEdm/TrackCollection.h>
#include <ActsPodioEdm/TrackStateCollection.h>
#include <ActsPodioEdm/TrackStateHitLinkCollection.h>
#include <algorithms/algorithm.h>
#include <edm4eic/MCRecoTrackParticleAssociationCollection.h>
#include <edm4eic/MCRecoTrackParticleLinkCollection.h>
#include <edm4eic/MCRecoTrackerHitAssociationCollection.h>
#include <edm4eic/Measurement2DCollection.h>
#include <edm4eic/TrackCollection.h>
#include <edm4eic/TrackParametersCollection.h>
#include <edm4eic/TrackSeedCollection.h>
#include <edm4eic/TrajectoryCollection.h>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include "ActsGeometryProvider.h"
#include "algorithms/interfaces/ActsSvc.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using ActsToTracksAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::Measurement2DCollection, edm4eic::TrackSeedCollection,
                      ActsPodioEdm::TrackStateCollection, ActsPodioEdm::BoundParametersCollection,
                      ActsPodioEdm::JacobianCollection, ActsPodioEdm::TrackStateHitLinkCollection,
                      ActsPodioEdm::TrackCollection,
                      std::optional<edm4eic::MCRecoTrackerHitAssociationCollection>>,
    algorithms::Output<edm4eic::TrajectoryCollection, edm4eic::TrackParametersCollection,
                       edm4eic::TrackCollection,
                       std::optional<edm4eic::MCRecoTrackParticleLinkCollection>,
                       std::optional<edm4eic::MCRecoTrackParticleAssociationCollection>>>;

class ActsToTracks : public ActsToTracksAlgorithm, public WithPodConfig<NoConfig> {
public:
  ActsToTracks(std::string_view name)
      : ActsToTracksAlgorithm{name,
                              {
                                  "inputMeasurements",
                                  "inputTrackSeeds",
                                  "inputActsTrackStates",
                                  "inputActsTrackParameters",
                                  "inputActsTrackJacobians",
                                  "inputActsTrackStateHitLinks",
                                  "inputActsTracks",
                                  "inputRawTrackerHitAssociations",
                              },
                              {
                                  "outputTrajectories",
                                  "outputTrackParameters",
                                  "outputTracks",
                                  "outputTrackLinks",
                                  "outputTrackAssociations",
                              },
                              "Converts ACTS tracks to EDM4eic"} {};

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  const algorithms::ActsSvc& m_actsSvc{algorithms::ActsSvc::instance()};
  std::shared_ptr<const ActsGeometryProvider> m_geoSvc{m_actsSvc.acts_geometry_provider()};
};

} // namespace eicrecon
