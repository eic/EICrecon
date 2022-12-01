// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifndef EICRECON_TRACK_SEEDING_H
#define EICRECON_TRACK_SEEDING_H

#include <functional>
#include <random>
#include <stdexcept>
#include <vector>

#include <boost/container/flat_map.hpp>
#include <boost/container/flat_set.hpp>

#include "JugBase/BField/DD4hepBField.h"
#include "JugTrack/GeometryContainers.hpp"
#include "JugTrack/Index.hpp"
#include "JugTrack/IndexSourceLink.hpp"
#include "JugTrack/Measurement.hpp"
#include "JugTrack/Track.hpp"
#include "JugTrack/TrackingResultTrajectory.hpp"

#include "edm4eic/TrackerHitCollection.h"
#include <edm4eic/TrackParameters.h>
#include <edm4eic/Trajectory.h>
#include <spdlog/logger.h>

#include "Acts/Definitions/Common.hpp"
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/TrackFinding/CombinatorialKalmanFilter.hpp"
#include "Acts/TrackFinding/MeasurementSelector.hpp"
#include <algorithms/interfaces/IObjectProducer.h>
#include <edm4hep/MCParticle.h>
#include <edm4eic/TrackParameters.h>
#include <algorithms/interfaces/WithPodConfig.h>
#include "TrackSeedingConfig.h"
#include "ActsGeometryProvider.h"


namespace eicrecon {
    class TrackSeeding:
            public eicrecon::WithPodConfig<eicrecon::TrackSeedingConfig> {
    public:
        void init(std::shared_ptr<const ActsGeometryProvider> geo_svc, std::shared_ptr<spdlog::logger> log);
        std::vector<edm4eic::TrackParameters*> produce(std::vector<const edm4eic::TrackerHit*> trk_hits) override;

    private:
        std::shared_ptr<spdlog::logger> m_log;
        std::shared_ptr<const ActsGeometryProvider> m_geoSvc;

        std::shared_ptr<const eicrecon::BField::DD4hepBField> m_BField = nullptr;
        Acts::GeometryContext m_geoctx;
        Acts::CalibrationContext m_calibctx;
        Acts::MagneticFieldContext m_fieldctx;

        Acts::MeasurementSelector::Config m_sourcelinkSelectorCfg;

    };
}


#endif //EICRECON_TRUTHTRACKSEEDING_H
