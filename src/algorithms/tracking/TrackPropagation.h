// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <Acts/EventData/TrackParameters.hpp>
#include <Acts/Geometry/GeometryContext.hpp>
#include <Acts/MagneticField/MagneticFieldContext.hpp>
#include <Acts/Surfaces/Surface.hpp>
#include <Acts/Utilities/Result.hpp>
#include <ActsExamples/EventData/Trajectories.hpp>
#include <edm4eic/TrackPoint.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <spdlog/logger.h>
#include <functional>
#include <memory>
#include <vector>

#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/tracking/TrackPropagationConfig.h"
#include "algorithms/tracking/ActsGeometryProvider.h"

namespace eicrecon {

    using ActsTrackPropagationResult = Acts::Result<std::unique_ptr<const Acts::BoundTrackParameters>>;

    /** Extract the particles form fit trajectories.
     *
     * \ingroup tracking
     */
    class TrackPropagation: public eicrecon::WithPodConfig<TrackPropagationConfig> {


    public:

        /** Initialize algorithm */
        void init(const dd4hep::Detector* detector, std::shared_ptr<const ActsGeometryProvider> geo_svc, std::shared_ptr<spdlog::logger> logger);

        void process(
                const std::tuple<const std::vector<const ActsExamples::Trajectories*>, const std::vector<const ActsExamples::ConstTrackContainer*>> input,
                const std::tuple<edm4eic::TrackSegmentCollection*> output) const {

            const auto [acts_trajectories, acts_tracks] = input;
            auto [propagated_tracks] = output;

            for (auto traj: acts_trajectories) {
                edm4eic::MutableTrackSegment this_propagated_track;
                for(auto& surf : m_target_surface_list) {
                    auto prop_point = propagate(traj, surf);
                    if(!prop_point) continue;
                    prop_point->surface = surf->geometryId().layer();
                    prop_point->system  = surf->geometryId().extra();
                    this_propagated_track.addToPoints(*prop_point);
                }
                propagated_tracks->push_back(this_propagated_track);
            }
        }

        /** Propagates a single trajectory to a given surface */
        std::unique_ptr<edm4eic::TrackPoint> propagate(const ActsExamples::Trajectories *, const std::shared_ptr<const Acts::Surface>& targetSurf) const;

        /** Propagates a collection of trajectories to a given surface
         * @remark: being a simple wrapper of propagate(...) this method is more suitable for factories */
        std::vector<std::unique_ptr<edm4eic::TrackPoint>> propagateMany(std::vector<const ActsExamples::Trajectories *> trajectories,
                                                         const std::shared_ptr<const Acts::Surface> &targetSurf) const;

        /** Propagates a collection of trajectories to a list of surfaces, and returns the full `TrackSegment`;
         * @param trajectories the input collection of trajectories
         * @param targetSurfaces the list of surfaces to propagate to
         * @param filterSurface if defined, do not propagate to any surfaces unless successful propagation to this filterSurface
         * @param trackPointCut an optional cut to omit specific track points
         * @param stopIfTrackPointCutFailed if true, stop propagating a trajectory when trackPointCut returns false
         * @return the resulting collection of propagated tracks
         */
        std::unique_ptr<edm4eic::TrackSegmentCollection> propagateToSurfaceList(
            std::vector<const ActsExamples::Trajectories*> trajectories,
            std::vector<std::shared_ptr<Acts::Surface>> targetSurfaces,
            std::shared_ptr<Acts::Surface> filterSurface = nullptr,
            std::function<bool(edm4eic::TrackPoint)> trackPointCut = [] (edm4eic::TrackPoint p) { return true; },
            bool stopIfTrackPointCutFailed = false
            ) const;

    private:

        Acts::GeometryContext m_geoContext;
        Acts::MagneticFieldContext m_fieldContext;
        std::shared_ptr<const ActsGeometryProvider> m_geoSvc;
        std::shared_ptr<spdlog::logger> m_log;

        std::vector<std::shared_ptr<Acts::Surface>> m_target_surface_list;
    };
} // namespace eicrecon
