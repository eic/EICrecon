// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifndef EICRECON_TRACKPROPAGATION_H
#define EICRECON_TRACKPROPAGATION_H


#include <memory>
#include <spdlog/logger.h>

#include <Acts/Geometry/TrackingGeometry.hpp>
#include <Acts/EventData/TrackParameters.hpp>
#include <Acts/EventData/MultiTrajectory.hpp>
#include <Acts/EventData/MultiTrajectoryHelpers.hpp>


#include <algorithms/tracking/JugTrack/Trajectories.hpp>

#include <edm4eic/TrackSegment.h>
#include <Acts/Surfaces/DiscSurface.hpp>

#include "ActsGeometryProvider.h"
#include "TrackProjector.h"



namespace eicrecon {

    using ActsTrackPropagationResult = Acts::Result<std::unique_ptr<const Acts::BoundTrackParameters>>;

    /** Extrac the particles form fit trajectories.
     *
     * \ingroup tracking
     */
    class TrackPropagation {


    public:

        /** Initialize algorithm */
        void init(std::shared_ptr<const ActsGeometryProvider> geo_svc, std::shared_ptr<spdlog::logger> logger);

        /** Execute track propagation */
        std::vector<edm4eic::TrackSegment *> execute(std::vector<const Jug::Trajectories *> trajectories);

    private:

        /** Does ACTS track propagation **/
        ActsTrackPropagationResult propagateTrack(const Acts::BoundTrackParameters& params, const std::shared_ptr<const Acts::Surface>& targetSurf);

        Acts::GeometryContext m_geoContext;
        Acts::MagneticFieldContext m_fieldContext;

        std::shared_ptr<Acts::DiscSurface> hcalEndcapNSurf;

        std::shared_ptr<const ActsGeometryProvider> m_geoSvc;
        std::shared_ptr<spdlog::logger> m_log;
    };
} // namespace eicrecon


#endif //EICRECON_TRACKPROPAGATION_H
