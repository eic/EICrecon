// Created by Joe Osborn
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <functional>
#include <random>
#include <stdexcept>
#include <vector>
#include "IVFConfig.h"
#include "ActsGeometryProvider.h"

#include "JugBase/BField/DD4hepBField.h"
#include "JugTrack/GeometryContainers.hpp"
#include "JugTrack/Track.hpp"
#include "JugTrack/TrackingResultTrajectory.hpp"

#include <edm4eic/TrackParameters.h>
#include <edm4eic/Trajectory.h>
#include <edm4eic/Vertex.h>
#include <spdlog/logger.h>

#include "Acts/Definitions/Common.hpp"
#include <algorithms/interfaces/IObjectProducer.h>
#include <edm4hep/MCParticle.h>
#include <edm4eic/TrackParameters.h>
#include <algorithms/interfaces/WithPodConfig.h>



namespace eicrecon {
    class ActsIVF:
            public eicrecon::WithPodConfig<eicrecon::IVFConfig> {
    public:
        void init(std::shared_ptr<const ActsGeometryProvider> geo_svc, std::shared_ptr<spdlog::logger> log);
        std::vector<edm4eic::Vertex*> produce(std::vector<const eicrecon::TrackingResultTrajectory*> trajectories);

    private:
        std::shared_ptr<spdlog::logger> m_log;
        std::shared_ptr<const ActsGeometryProvider> m_geoSvc;

        std::shared_ptr<const eicrecon::BField::DD4hepBField> m_BField = nullptr;
        Acts::GeometryContext m_geoctx;
        Acts::MagneticFieldContext m_fieldctx;
        IVFConfig m_cfg;
};
}
