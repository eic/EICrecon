// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#if Acts_VERSION_MAJOR >= 37
#include <Acts/EventData/SpacePointContainer.hpp>
#endif
#include <Acts/MagneticField/MagneticFieldContext.hpp>
#include <Acts/Seeding/SeedFilterConfig.hpp>
#include <Acts/Seeding/SeedFinderConfig.hpp>
#include <Acts/Seeding/SeedFinderOrthogonalConfig.hpp>
#if Acts_VERSION_MAJOR >= 37
#include <ActsExamples/EventData/SpacePointContainer.hpp>
#endif
#include <edm4eic/TrackParametersCollection.h>
#include <edm4eic/TrackerHitCollection.h>
#include <spdlog/logger.h>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>

#include "ActsGeometryProvider.h"
#include "DD4hepBField.h"
#include "OrthogonalTrackSeedingConfig.h"
#include "SpacePoint.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {
class TrackSeeding : public eicrecon::WithPodConfig<eicrecon::OrthogonalTrackSeedingConfig> {
public:
#if Acts_VERSION_MAJOR >= 37
  using proxy_type = typename Acts::SpacePointContainer<
      ActsExamples::SpacePointContainer<std::vector<const SpacePoint*>>,
      Acts::detail::RefHolder>::SpacePointProxyType;
#endif

  void init(std::shared_ptr<const ActsGeometryProvider> geo_svc,
            std::shared_ptr<spdlog::logger> log);
  std::unique_ptr<edm4eic::TrackParametersCollection>
  produce(const edm4eic::TrackerHitCollection& trk_hits);

private:
  void configure();

  std::shared_ptr<spdlog::logger> m_log;
  std::shared_ptr<const ActsGeometryProvider> m_geoSvc;

  std::shared_ptr<const eicrecon::BField::DD4hepBField> m_BField = nullptr;
  Acts::MagneticFieldContext m_fieldctx;

  Acts::SeedFilterConfig m_seedFilterConfig;
  Acts::SeedFinderOptions m_seedFinderOptions;
#if Acts_VERSION_MAJOR >= 37
  Acts::SeedFinderOrthogonalConfig<proxy_type> m_seedFinderConfig;
#else
  Acts::SeedFinderOrthogonalConfig<SpacePoint> m_seedFinderConfig;
#endif

  static int determineCharge(std::vector<std::pair<float, float>>& positions,
                             const std::pair<float, float>& PCA,
                             std::tuple<float, float, float>& RX0Y0);
  static std::pair<float, float> findPCA(std::tuple<float, float, float>& circleParams);
  static std::vector<const eicrecon::SpacePoint*>
  getSpacePoints(const edm4eic::TrackerHitCollection& trk_hits);
  std::unique_ptr<edm4eic::TrackParametersCollection> makeTrackParams(SeedContainer& seeds);

  static std::tuple<float, float, float> circleFit(std::vector<std::pair<float, float>>& positions);
  static std::tuple<float, float> lineFit(std::vector<std::pair<float, float>>& positions);
};
} // namespace eicrecon
