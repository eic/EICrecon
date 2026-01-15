// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2024, Dmitry Romanov, Wouter Deconinck

#pragma once

#include <Acts/EventData/TrackContainer.hpp>
#include <Acts/EventData/TrackParameters.hpp>
#include <Acts/EventData/TrackProxy.hpp>
#include <Acts/Geometry/GeometryContext.hpp>
#include <Acts/Geometry/GeometryIdentifier.hpp>
#include <Acts/MagneticField/MagneticFieldContext.hpp>
#include <Acts/Surfaces/Surface.hpp>
#include <Acts/Utilities/Result.hpp>
#include <ActsExamples/EventData/Track.hpp>
#include <ActsPlugins/EDM4hep/PodioTrackContainer.hpp>
#include <ActsPlugins/EDM4hep/PodioTrackStateContainer.hpp>
#include <ActsPodioEdm/BoundParametersCollection.h>
#include <ActsPodioEdm/JacobianCollection.h>
#include <ActsPodioEdm/TrackCollection.h>
#include <ActsPodioEdm/TrackStateCollection.h>
#include <DD4hep/Detector.h>
#include <algorithms/algorithm.h>
#include <edm4eic/TrackCollection.h>
#include <edm4eic/TrackPoint.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <memory>
#include <string_view>
#include <vector>

#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/interfaces/ActsSvc.h"
#include "algorithms/tracking/ActsGeometryProvider.h"
#include "algorithms/tracking/PodioGeometryIdConversionHelper.h"
#include "algorithms/tracking/TrackPropagationConfig.h"

namespace eicrecon {

using ActsTrackPropagationResult = Acts::Result<std::unique_ptr<const Acts::BoundTrackParameters>>;

using TrackPropagationAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::TrackCollection, ActsPodioEdm::TrackStateCollection,
                      ActsPodioEdm::BoundParametersCollection, ActsPodioEdm::JacobianCollection,
                      ActsPodioEdm::TrackCollection>,
    algorithms::Output<edm4eic::TrackSegmentCollection>>;

// Define Podio track container types
using PodioTrackContainer =
    Acts::TrackContainer<ActsPlugins::ConstPodioTrackContainer<>,
                         ActsPlugins::ConstPodioTrackStateContainer<>, Acts::RefHolder>;
using PodioTrackProxy =
    Acts::TrackProxy<ActsPlugins::ConstPodioTrackContainer<>,
                     ActsPlugins::ConstPodioTrackStateContainer<>, Acts::RefHolder>;

/** Extract the particles from fit tracks.
     *
     * \ingroup tracking
     */
class TrackPropagation : public TrackPropagationAlgorithm,
                         public WithPodConfig<TrackPropagationConfig> {

public:
  TrackPropagation(std::string_view name)
      : TrackPropagationAlgorithm{name,
                                  {"inputTracks", "inputActsTrackStates",
                                   "inputActsTrackParameters", "inputActsTrackJacobians",
                                   "inputActsTracks"},
                                  {"outputTrackSegments"},
                                  "Track propagation to surfaces"} {}

  /** Initialize algorithm */
  void init() final;

  void setDetector(const dd4hep::Detector* detector) { m_detector = detector; }

  void process(const Input& input, const Output& output) const final {
    const auto [tracks, track_states, track_parameters, track_jacobians, tracks_acts] = input;
    auto [propagated_tracks]                                                          = output;

    // Create conversion helper for Podio backend
    PodioGeometryIdConversionHelper helper;
    helper.geoCtx           = Acts::GeometryContext{};
    helper.trackingGeometry = m_geoSvc->trackingGeometry();

    // Construct ConstPodioTrackContainer from Podio collections
    ActsPlugins::ConstPodioTrackStateContainer<> trackStateContainer(
        helper, Acts::ConstRefHolder<const ActsPodioEdm::TrackStateCollection>{*track_states},
        Acts::ConstRefHolder<const ActsPodioEdm::BoundParametersCollection>{*track_parameters},
        Acts::ConstRefHolder<const ActsPodioEdm::JacobianCollection>{*track_jacobians});
    ActsPlugins::ConstPodioTrackContainer<> trackContainer(
        helper, Acts::ConstRefHolder<const ActsPodioEdm::TrackCollection>{*tracks_acts});
    PodioTrackContainer constTracks(trackContainer, trackStateContainer);

    std::size_t i = 0;
    for (const auto& track : constTracks) {
      auto this_propagated_track = propagated_tracks->create();
      if (tracks->size() == constTracks.size()) {
        trace("track segment connected to track {}", i);
        this_propagated_track.setTrack((*tracks)[i]);
      }
      for (auto& surf : m_target_surfaces) {
        auto prop_point =
            propagate(tracks->size() == constTracks.size() ? (*tracks)[i] : edm4eic::Track{}, track,
                      constTracks, surf);
        if (!prop_point)
          continue;
        prop_point->surface = surf->geometryId().layer();
        prop_point->system  = surf->geometryId().extra();
        this_propagated_track.addToPoints(*prop_point);
      }
      ++i;
    }
  }

  /** Propagates a single track to a given surface */
  std::unique_ptr<edm4eic::TrackPoint>
  propagate(const edm4eic::Track&, const PodioTrackProxy&, const PodioTrackContainer&,
            const std::shared_ptr<const Acts::Surface>& targetSurf) const;

  /** Propagates a collection of tracks to a list of surfaces, and returns the full `TrackSegment`;
         * @param tracks the input collection of tracks
         * @return the resulting collection of propagated tracks
         */
  void propagateToSurfaceList(const Input& input, const Output& output) const;

private:
  Acts::GeometryContext m_geoContext;
  Acts::MagneticFieldContext m_fieldContext;
  const algorithms::ActsSvc& m_actsSvc{algorithms::ActsSvc::instance()};
  std::shared_ptr<const ActsGeometryProvider> m_geoSvc{m_actsSvc.acts_geometry_provider()};
  const dd4hep::Detector* m_detector = nullptr;

  std::vector<std::shared_ptr<Acts::Surface>> m_filter_surfaces;
  std::vector<std::shared_ptr<Acts::Surface>> m_target_surfaces;
};
} // namespace eicrecon
