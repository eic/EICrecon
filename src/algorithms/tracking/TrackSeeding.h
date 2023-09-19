// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <Acts/MagneticField/MagneticFieldContext.hpp>
#include <Acts/Seeding/SeedFilterConfig.hpp>
#include <Acts/Seeding/SeedFinderOrthogonalConfig.hpp>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>

#include "OrthogonalTrackSeedingConfig.h"
#include "SpacePoint.h"
#include "algorithms/interfaces/WithPodConfig.h"

class ActsGeometryProvider;
namespace edm4eic { class TrackParameters; }
namespace edm4eic { class TrackerHit; }
namespace eicrecon::BField { class DD4hepBField; }
namespace spdlog { class logger; }


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
