// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <functional>
#include <random>
#include <stdexcept>
#include <vector>

#include <boost/container/flat_map.hpp>
#include <boost/container/flat_set.hpp>

#include "DD4hepBField.h"
#include "ActsExamples/EventData/GeometryContainers.hpp"
#include "ActsExamples/EventData/Index.hpp"
#include "ActsExamples/EventData/IndexSourceLink.hpp"
#include "ActsExamples/EventData/Measurement.hpp"
#include "ActsExamples/EventData/Track.hpp"
#include "ActsExamples/EventData/Trajectories.hpp"

#include <edm4eic/TrackerHitCollection.h>
#include <edm4eic/TrackParameters.h>
#include <edm4eic/Trajectory.h>
#include <spdlog/logger.h>

#include <Acts/Definitions/Common.hpp>
#include <Acts/Geometry/TrackingGeometry.hpp>
#include <Acts/TrackFinding/CombinatorialKalmanFilter.hpp>
#include <Acts/TrackFinding/MeasurementSelector.hpp>
#include "algorithms/interfaces/IObjectProducer.h"
#include <edm4hep/MCParticle.h>
#include <edm4eic/TrackParameters.h>
#include "algorithms/interfaces/WithPodConfig.h"
#include "OrthogonalTrackSeedingConfig.h"



namespace eicrecon {
    class TrackSeeding:
            public eicrecon::WithPodConfig<eicrecon::OrthogonalTrackSeedingConfig> {
    public:
        void init(std::shared_ptr<const ActsGeometryProvider> geo_svc, std::shared_ptr<spdlog::logger> log);
        std::vector<edm4eic::TrackParameters*> produce(std::vector<const edm4eic::TrackerHit*> trk_hits);

    private:
        std::shared_ptr<spdlog::logger> m_log;
        std::shared_ptr<const ActsGeometryProvider> m_geoSvc;

        std::shared_ptr<const eicrecon::BField::DD4hepBField> m_BField = nullptr;
        Acts::GeometryContext m_geoctx;
        Acts::CalibrationContext m_calibctx;
        Acts::MagneticFieldContext m_fieldctx;

	int determineCharge(std::vector<std::pair<float,float>>& positions) const;
	SeedContainer runSeeder(std::vector<const edm4eic::TrackerHit*>& trk_hits);
	std::pair<float,float> findPCA(std::tuple<float,float,float>& circleParams) const;
	std::vector<const eicrecon::SpacePoint*> getSpacePoints(std::vector<const edm4eic::TrackerHit*>& trk_hits);
	std::vector<edm4eic::TrackParameters*> makeTrackParams(SeedContainer& seeds);

	std::tuple<float,float,float> circleFit(std::vector<std::pair<float,float>>& positions) const;
	std::tuple<float,float> lineFit(std::vector<std::pair<float,float>>& positions) const;
    };
}
