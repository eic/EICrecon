// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <cstddef> // FIXME size_t missing in SeedConfirmationRangeConfig.hpp until Acts 27.2.0 (maybe even later)
#include <vector>

#include <Acts/Definitions/Units.hpp>
#include <Acts/MagneticField/MagneticFieldContext.hpp>
#include <Acts/Seeding/SeedConfirmationRangeConfig.hpp>
#include <Acts/Seeding/SeedFilterConfig.hpp>
#include <Acts/Seeding/SeedFinderOrthogonalConfig.hpp>

#include <edm4eic/TrackerHitCollection.h>
#include <edm4eic/TrackParametersCollection.h>

#include <spdlog/logger.h>

#include "algorithms/interfaces/WithPodConfig.h"
#include "OrthogonalTrackSeedingConfig.h"

#include "ActsGeometryProvider.h"
#include "DD4hepBField.h"
#include "SpacePoint.h"


namespace eicrecon {
    class TrackSeeding:
            public eicrecon::WithPodConfig<eicrecon::OrthogonalTrackSeedingConfig> {
    public:
        void init(std::shared_ptr<const ActsGeometryProvider> geo_svc, std::shared_ptr<spdlog::logger> log);
        std::vector<edm4eic::TrackParameters*> produce(std::vector<const edm4eic::TrackerHit*> trk_hits);

    private:
        void configure();

        std::shared_ptr<spdlog::logger> m_log;
        std::shared_ptr<const ActsGeometryProvider> m_geoSvc;

        std::shared_ptr<const eicrecon::BField::DD4hepBField> m_BField = nullptr;
        Acts::MagneticFieldContext m_fieldctx;

        Acts::SeedFilterConfig m_seedFilterConfig;
        Acts::SeedFinderOrthogonalConfig<SpacePoint> m_seedFinderConfig;

        int determineCharge(std::vector<std::pair<float,float>>& positions) const;
        std::pair<float,float> findPCA(std::tuple<float,float,float>& circleParams) const;
        std::vector<const eicrecon::SpacePoint*> getSpacePoints(std::vector<const edm4eic::TrackerHit*>& trk_hits);
        std::vector<edm4eic::TrackParameters*> makeTrackParams(SeedContainer& seeds);

        std::tuple<float,float,float> circleFit(std::vector<std::pair<float,float>>& positions) const;
        std::tuple<float,float> lineFit(std::vector<std::pair<float,float>>& positions) const;
    };
}
