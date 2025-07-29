// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023  - 2025 Joe Osborn, Dmitry Romanov, Wouter Deconinck// Created by Dmitry Romanov

#pragma once

#if Acts_VERSION_MAJOR >= 37
#include <Acts/EventData/SpacePointContainer.hpp>
#endif
#include <Acts/Seeding/SeedFilterConfig.hpp>
#include <Acts/Seeding/SeedFinderConfig.hpp>
#include <Acts/Seeding/SeedFinderOrthogonalConfig.hpp>
#include <Acts/Utilities/Holders.hpp>
#if Acts_VERSION_MAJOR >= 37
#include <ActsExamples/EventData/SpacePointContainer.hpp>
#endif
#include <algorithms/algorithm.h>
#include <edm4eic/TrackParametersCollection.h>
#include <edm4eic/TrackerHitCollection.h>
#include <cmath>
#include <iterator>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>

#include "ActsGeometryProvider.h"
#include "OrthogonalTrackSeedingConfig.h"
#include "SpacePoint.h"
#include "algorithms/interfaces/ActsSvc.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using TrackSeedingAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4eic::TrackerHitCollection>,
                          algorithms::Output<edm4eic::TrackParametersCollection>>;

class TrackSeeding : public TrackSeedingAlgorithm,
                     public WithPodConfig<OrthogonalTrackSeedingConfig> {
public:
#if Acts_VERSION_MAJOR >= 37
  using proxy_type = typename Acts::SpacePointContainer<
      ActsExamples::SpacePointContainer<std::vector<const SpacePoint*>>,
      Acts::detail::RefHolder>::SpacePointProxyType;
#endif

  TrackSeeding(std::string_view name)
      : TrackSeedingAlgorithm{name,
                              {"inputTrackerHits"},
                              {"outputTrackParameters"},
                              "create track seeds from tracker hits"} {}

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  const algorithms::ActsSvc& m_actsSvc{algorithms::ActsSvc::instance()};
  const std::shared_ptr<const ActsGeometryProvider> m_geoSvc{m_actsSvc.acts_geometry_provider()};

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
  void addToTrackParams(edm4eic::TrackParametersCollection& trackparams,
                        SeedContainer& seeds) const;

  static std::tuple<float, float, float> circleFit(std::vector<std::pair<float, float>>& positions);
  static std::tuple<float, float> lineFit(std::vector<std::pair<float, float>>& positions);
};
} // namespace eicrecon
