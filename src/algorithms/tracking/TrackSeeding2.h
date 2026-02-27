// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025, EICrecon Authors

#pragma once

#include <Acts/Definitions/Units.hpp>
#include <algorithms/algorithm.h>
#include <edm4eic/TrackParametersCollection.h>
#include <edm4eic/TrackSeedCollection.h>
#include <edm4eic/TrackerHitCollection.h>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <Acts/Utilities/Logger.hpp>

#include "ActsGeometryProvider.h"
#include "TripletTrackSeedingConfig.h"
#include "algorithms/interfaces/ActsSvc.h"
#include "algorithms/interfaces/WithPodConfig.h"

#if Acts_VERSION_MAJOR >= 45
#include <Acts/EventData/SeedContainer2.hpp>
#include <Acts/EventData/SpacePointContainer2.hpp>
#include <Acts/Seeding/SeedConfirmationRangeConfig.hpp>
#include <Acts/Seeding2/BroadTripletSeedFilter.hpp>
#include <Acts/Seeding2/DoubletSeedFinder.hpp>
#include <Acts/Seeding2/TripletSeedFinder.hpp>
#include <Acts/Seeding2/TripletSeeder.hpp>
#endif

namespace eicrecon {

using TrackSeeding2Algorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::TrackerHitCollection>,
    algorithms::Output<edm4eic::TrackSeedCollection, edm4eic::TrackParametersCollection>>;

/// Track seeding using the Acts Seeding2 API (CylindricalSpacePointKDTree +
/// DoubletSeedFinder + TripletSeedFinder + BroadTripletSeedFilter).
/// Requires Acts >= 45.
class TrackSeeding2 : public TrackSeeding2Algorithm,
                      public WithPodConfig<TripletTrackSeedingConfig> {
public:
  TrackSeeding2(std::string_view name)
      : TrackSeeding2Algorithm{name,
                               {"inputTrackerHits"},
                               {"outputTrackSeeds", "outputTrackParameters"},
                               "create track seeds using Acts Seeding2 API"} {}

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  const algorithms::ActsSvc& m_actsSvc{algorithms::ActsSvc::instance()};
  const std::shared_ptr<const ActsGeometryProvider> m_geoSvc{m_actsSvc.acts_geometry_provider()};

  std::shared_ptr<const Acts::Logger> m_actsLogger{nullptr};
  const Acts::Logger& actsLogger() const { return *m_actsLogger; }

#if Acts_VERSION_MAJOR >= 45
  Acts::BroadTripletSeedFilter::Config m_filterConfig;
  std::optional<Acts::TripletSeeder> m_seedFinder;
#endif

  static std::optional<edm4eic::MutableTrackParameters>
  estimateTrackParamsFromSeed(const std::array<std::array<float, 3>, 3>& spPositions, float vertexZ,
                              float bFieldInZ,
                              const std::shared_ptr<const ActsGeometryProvider>& geoSvc,
                              const TripletTrackSeedingConfig& cfg);

  static int determineCharge(std::vector<std::pair<float, float>>& positions,
                             const std::pair<float, float>& PCA,
                             std::tuple<float, float, float>& RX0Y0);
  static std::pair<float, float> findPCA(std::tuple<float, float, float>& circleParams);
  static std::tuple<float, float, float> circleFit(std::vector<std::pair<float, float>>& positions);
  static std::tuple<float, float> lineFit(std::vector<std::pair<float, float>>& positions);
};

} // namespace eicrecon
