// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <edm4eic/TrackSegment.h>

#include "algorithms/interfaces/WithPodConfig.h"
#include <spdlog/logger.h>
#include <spdlog/fmt/ostr.h>
#include "TrackProjectorConfig.h"
#include "ActsGeometryProvider.h"
#include "JugTrack/TrackingResultTrajectory.hpp"


namespace eicrecon {

        /** Extract the particles form fit trajectories.
         *
         * \ingroup tracking
         */
        class TrackProjector:
                public eicrecon::WithPodConfig<TrackProjectorConfig> {

        public:

            void init(std::shared_ptr<const ActsGeometryProvider> geo_svc, std::shared_ptr<spdlog::logger> logger);

            std::vector<edm4eic::TrackSegment*> execute(std::vector<const eicrecon::TrackingResultTrajectory*> trajectories);

        private:
            std::shared_ptr<const ActsGeometryProvider> m_geo_provider;
            std::shared_ptr<spdlog::logger> m_log;

        };



} // eicrecon
