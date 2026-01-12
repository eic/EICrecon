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
#include <DD4hep/Detector.h>
#include <algorithms/algorithm.h>
#include <edm4eic/TrackCollection.h>
#include <edm4eic/TrackPoint.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <memory>
#include <string_view>
#include <vector>

#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/tracking/ActsGeometryProvider.h"
#include "algorithms/tracking/TrackPropagationConfig.h"

namespace eicrecon {

using ActsTrackPropagationResult = Acts::Result<std::unique_ptr<const Acts::BoundTrackParameters>>;

using TrackPropagationAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::TrackCollection, Acts::ConstVectorMultiTrajectory,
                      Acts::ConstVectorTrackContainer>,
    algorithms::Output<edm4eic::TrackSegmentCollection>>;

/** Extract the particles from fit tracks.
     *
     * \ingroup tracking
     */
class TrackPropagation : public TrackPropagationAlgorithm,
                         public WithPodConfig<TrackPropagationConfig> {

public:
  TrackPropagation(std::string_view name)
      : TrackPropagationAlgorithm{name,
                                  {"inputTracks", "inputActsTrackStates", "inputActsTracks"},
                                  {"outputTrackSegments"},
                                  "Track propagation to surfaces"} {}

  /** Initialize algorithm */
  void init() final;

  void setGeometryService(std::shared_ptr<const ActsGeometryProvider> geo_svc) {
    m_geoSvc = geo_svc;
  }
  void setDetector(const dd4hep::Detector* detector) { m_detector = detector; }

  void process(const Input& input, const Output& output) const final {
    const auto [tracks, track_states, tracks_acts] = input;
    auto [propagated_tracks]                       = output;

    // Construct ConstTrackContainer from underlying containers
    auto trackStateContainer = std::make_shared<Acts::ConstVectorMultiTrajectory>(*track_states);
    auto trackContainer      = std::make_shared<Acts::ConstVectorTrackContainer>(*tracks_acts);
    ActsExamples::ConstTrackContainer constTracks(trackContainer, trackStateContainer);

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
  propagate(const edm4eic::Track&, const ActsExamples::ConstTrackProxy&,
            const ActsExamples::ConstTrackContainer&,
            const std::shared_ptr<const Acts::Surface>& targetSurf) const;

  /** Propagates a collection of tracks to a list of surfaces, and returns the full `TrackSegment`;
         * @param tracks the input collection of tracks
         * @return the resulting collection of propagated tracks
         */
  void propagateToSurfaceList(const Input& input, const Output& output) const;

private:
  Acts::GeometryContext m_geoContext;
  Acts::MagneticFieldContext m_fieldContext;
  std::shared_ptr<const ActsGeometryProvider> m_geoSvc;
  const dd4hep::Detector* m_detector = nullptr;

  std::vector<std::shared_ptr<Acts::Surface>> m_filter_surfaces;
  std::vector<std::shared_ptr<Acts::Surface>> m_target_surfaces;
};
} // namespace eicrecon
