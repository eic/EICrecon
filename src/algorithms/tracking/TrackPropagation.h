// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2024, Dmitry Romanov, Wouter Deconinck

#pragma once

#include <Acts/EventData/TrackParameters.hpp>
#include <Acts/Geometry/GeometryContext.hpp>
#include <Acts/Geometry/GeometryIdentifier.hpp>
#include <Acts/MagneticField/MagneticFieldContext.hpp>
#include <Acts/Surfaces/Surface.hpp>
#include <Acts/Utilities/Result.hpp>
#include <ActsExamples/EventData/Track.hpp>
#include <ActsExamples/EventData/Trajectories.hpp>
#include <DD4hep/Detector.h>
#include <edm4eic/TrackCollection.h>
#include <edm4eic/TrackPoint.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <spdlog/logger.h>
#include <cstddef>
#include <memory>
#include <tuple>
#include <variant>
#include <vector>

#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/tracking/ActsExamplesEdm.h"
#include "algorithms/tracking/ActsGeometryProvider.h"
#include "algorithms/tracking/TrackPropagationConfig.h"

namespace eicrecon {

using ActsTrackPropagationResult = Acts::Result<std::unique_ptr<const Acts::BoundTrackParameters>>;

/** Extract the particles form fit trajectories.
     *
     * \ingroup tracking
     */
template <typename edm_t = eicrecon::ActsExamplesEdm>
class TrackPropagation : public eicrecon::WithPodConfig<TrackPropagationConfig> {

public:
  /** Initialize algorithm */
  void init(const dd4hep::Detector* detector, std::shared_ptr<const ActsGeometryProvider> geo_svc,
            std::shared_ptr<spdlog::logger> logger);

  void process(const std::tuple<const edm4eic::TrackCollection&,
                                const std::vector<const typename edm_t::Trajectories*>,
                                const std::vector<const typename edm_t::ConstTrackContainer*>>
                   input,
               const std::tuple<edm4eic::TrackSegmentCollection*> output) const {

    const auto [tracks, acts_trajectories, acts_tracks] = input;
    auto [propagated_tracks]                            = output;

    for (std::size_t i = 0; auto traj : acts_trajectories) {
      auto this_propagated_track = propagated_tracks->create();
      if (tracks.size() == acts_trajectories.size()) {
        m_log->trace("track segment connected to track {}", i);
        this_propagated_track.setTrack(tracks[i]);
      }
      for (auto& surf : m_target_surfaces) {
        auto prop_point = propagate(
            tracks.size() == acts_trajectories.size() ? tracks[i] : edm4eic::Track{}, traj, surf);
        if (!prop_point)
          continue;
        prop_point->surface = surf->geometryId().layer();
        prop_point->system  = surf->geometryId().extra();
        this_propagated_track.addToPoints(*prop_point);
      }
      ++i;
    }
  }

  /** Propagates a single trajectory to a given surface */
  std::unique_ptr<edm4eic::TrackPoint>
  propagate(const edm4eic::Track&, const typename edm_t::Trajectories*,
            const std::shared_ptr<const Acts::Surface>& targetSurf) const;

  /** Propagates a collection of trajectories to a list of surfaces, and returns the full `TrackSegment`;
         * @param trajectories the input collection of trajectories
         * @return the resulting collection of propagated tracks
         */
  void propagateToSurfaceList(
      const std::tuple<const edm4eic::TrackCollection&,
                       const std::vector<const typename edm_t::Trajectories*>,
                       const std::vector<const typename edm_t::ConstTrackContainer*>>
          input,
      const std::tuple<edm4eic::TrackSegmentCollection*> output) const;

private:
  Acts::GeometryContext m_geoContext;
  Acts::MagneticFieldContext m_fieldContext;
  std::shared_ptr<const ActsGeometryProvider> m_geoSvc;
  std::shared_ptr<spdlog::logger> m_log;

  std::vector<std::shared_ptr<Acts::Surface>> m_filter_surfaces;
  std::vector<std::shared_ptr<Acts::Surface>> m_target_surfaces;
};
} // namespace eicrecon
