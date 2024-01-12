// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <ActsExamples/EventData/Trajectories.hpp>
#include <edm4eic/TrackSegmentCollection.h>
#include <spdlog/logger.h>
#include <memory>
#include <vector>

#include "ActsGeometryProvider.h"
#include "TrackProjectorConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"


namespace eicrecon {

        /** Extract the particles form fit trajectories.
         *
         * \ingroup tracking
         */
        class TrackProjector:
                public eicrecon::WithPodConfig<TrackProjectorConfig> {

        public:

            void init(std::shared_ptr<const ActsGeometryProvider> geo_svc, std::shared_ptr<spdlog::logger> logger);

            std::unique_ptr<edm4eic::TrackSegmentCollection> execute(std::vector<const ActsExamples::Trajectories*> trajectories);

        private:
            std::shared_ptr<const ActsGeometryProvider> m_geo_provider;
            std::shared_ptr<spdlog::logger> m_log;

        };



} // eicrecon
