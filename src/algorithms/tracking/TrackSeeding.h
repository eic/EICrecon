// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 - 2026, Joe Osborn, Dmitry Romanov, Wouter Deconinck

#pragma once

#include <Acts/Utilities/Logger.hpp>
#include <algorithms/algorithm.h>
#include <edm4eic/TrackParametersCollection.h>
#include <edm4eic/TrackSeedCollection.h>
#include <edm4eic/TrackerHitCollection.h>
#include <array>
#include <cmath>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>

#include "ActsGeometryProvider.h"
#include "TrackSeedingConfig.h"
#include "algorithms/interfaces/ActsSvc.h"
#include "algorithms/interfaces/WithPodConfig.h"

// Define version availability macros for each seeding method
// Seeding2 API available in Acts >= 45
#define TRACKSEEDING_HAS_SEEDING2 (Acts_VERSION_MAJOR >= 45)
// Orthogonal API available up to Acts 45 (may be deprecated later)
#define TRACKSEEDING_HAS_ORTHOGONAL (Acts_VERSION_MAJOR <= 45)

// Acts version-specific includes
#if TRACKSEEDING_HAS_SEEDING2
// Modern Seeding2 API (Acts >= 45)
#include <Acts/EventData/SeedContainer2.hpp>
#include <Acts/EventData/SpacePointContainer2.hpp>
#include <Acts/Seeding/SeedConfirmationRangeConfig.hpp>
#include <Acts/Seeding2/BroadTripletSeedFilter.hpp>
#include <Acts/Seeding2/DoubletSeedFinder.hpp>
#include <Acts/Seeding2/TripletSeedFinder.hpp>
#include <Acts/Seeding2/TripletSeeder.hpp>
#endif

#if TRACKSEEDING_HAS_ORTHOGONAL
// Orthogonal Seeding API (Acts <= 45)
#include <Acts/EventData/Seed.hpp>
#include <Acts/EventData/SpacePointContainer.hpp>
#include <Acts/Seeding/SeedFilterConfig.hpp>
#include <Acts/Seeding/SeedFinderConfig.hpp>
#include <Acts/Seeding/SeedFinderOrthogonalConfig.hpp>
#include <Acts/Utilities/HashedString.hpp>
#include <Acts/Utilities/Holders.hpp>
#if __has_include(<ActsExamples/EventData/SpacePointContainer.hpp>)
#include <ActsExamples/EventData/SpacePointContainer.hpp>
#else
#include <any>
#include <stdexcept>
#endif
#include "SpacePoint.h"
#endif

namespace eicrecon {

#if TRACKSEEDING_HAS_ORTHOGONAL
// SpacePointContainerAdapter only needed for Orthogonal API
#if !__has_include(<ActsExamples/EventData/SpacePointContainer.hpp>)
/// Adapter to wrap a collection of space points for use with Acts::SpacePointContainer.
/// This replaces ActsExamples::SpacePointContainer<T>, which was removed in Acts >= 46
/// (see https://github.com/acts-project/acts/pull/5088).
template <typename collection_t> class SpacePointContainerAdapter {
public:
  using CollectionType = collection_t;
  using ValueType      = typename CollectionType::value_type;

  friend class Acts::SpacePointContainer<SpacePointContainerAdapter<collection_t>,
                                         Acts::detail::RefHolder>;

  SpacePointContainerAdapter() = delete;
  explicit SpacePointContainerAdapter(CollectionType& container) : m_storage(container) {}

private:
  std::size_t size_impl() const { return storage().size(); }
  float x_impl(std::size_t idx) const { return storage()[idx]->x(); }
  float y_impl(std::size_t idx) const { return storage()[idx]->y(); }
  float z_impl(std::size_t idx) const { return storage()[idx]->z(); }
  float varianceR_impl(std::size_t idx) const { return storage()[idx]->varianceR(); }
  float varianceZ_impl(std::size_t idx) const { return storage()[idx]->varianceZ(); }
  const ValueType& get_impl(std::size_t idx) const { return storage()[idx]; }
  std::any component_impl(Acts::HashedString /*key*/, std::size_t /*n*/) const {
    throw std::runtime_error(
        "SpacePointContainerAdapter does not support detailed double measurement "
        "information (useDetailedDoubleMeasurementInfo). Use Acts::SpacePointContainer2 "
        "if this feature is required.");
  }

  const CollectionType& storage() const { return *m_storage; }

  Acts::detail::RefHolder<CollectionType> m_storage;
};
#endif
#endif // Acts_VERSION_MAJOR < 45

using TrackSeedingAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::TrackerHitCollection>,
    algorithms::Output<edm4eic::TrackSeedCollection, edm4eic::TrackParametersCollection>>;

/// Track seeding algorithm with automatic implementation selection based on Acts version.
/// - Acts >= 45: Uses modern Seeding2 API (DoubletSeedFinder + TripletSeedFinder)
/// - Acts < 45: Uses legacy SeedFinderOrthogonal API
class TrackSeeding : public TrackSeedingAlgorithm, public WithPodConfig<TrackSeedingConfig> {
public:
#if TRACKSEEDING_HAS_ORTHOGONAL
  // Orthogonal API types (Acts <= 45)
#if __has_include(<ActsExamples/EventData/SpacePointContainer.hpp>)
  using SpacePointContainerType = ActsExamples::SpacePointContainer<std::vector<const SpacePoint*>>;
#else
  using SpacePointContainerType = SpacePointContainerAdapter<std::vector<const SpacePoint*>>;
#endif
  using proxy_type =
      typename Acts::SpacePointContainer<SpacePointContainerType,
                                         Acts::detail::RefHolder>::SpacePointProxyType;
#endif

  TrackSeeding(std::string_view name)
      : TrackSeedingAlgorithm{name,
                              {"inputTrackerHits"},
                              {"outputTrackSeeds", "outputTrackParameters"},
                              "create track seeds from tracker hits"}
#if TRACKSEEDING_HAS_SEEDING2 && TRACKSEEDING_HAS_ORTHOGONAL
      , m_seedingData(std::in_place_type<Seeding2Data>) // Default to Seeding2 when both available
#endif
  {
  }

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  const algorithms::ActsSvc& m_actsSvc{algorithms::ActsSvc::instance()};
  const std::shared_ptr<const ActsGeometryProvider> m_geoSvc{m_actsSvc.acts_geometry_provider()};

#if TRACKSEEDING_HAS_SEEDING2
  // Seeding2-specific data
  struct Seeding2Data {
    std::shared_ptr<const Acts::Logger> actsLogger{nullptr};
    Acts::BroadTripletSeedFilter::Config filterConfig;
    std::optional<Acts::TripletSeeder> seedFinder;
  };
#endif

#if TRACKSEEDING_HAS_ORTHOGONAL
  // Orthogonal-specific data
  struct OrthogonalData {
    Acts::SeedFilterConfig seedFilterConfig;
    Acts::SeedFinderOptions seedFinderOptions;
    Acts::SeedFinderOrthogonalConfig<proxy_type> seedFinderConfig;
  };
#endif

#if TRACKSEEDING_HAS_SEEDING2 && TRACKSEEDING_HAS_ORTHOGONAL
  // Both methods available: Use runtime dispatch with variant
  std::variant<Seeding2Data, OrthogonalData> m_seedingData;

  // Helper to access Seeding2 logger
  const Acts::Logger& actsLogger() const {
    return *std::get<Seeding2Data>(m_seedingData).actsLogger;
  }
#elif TRACKSEEDING_HAS_SEEDING2
  // Only Seeding2 available (future Acts versions)
  Seeding2Data m_seedingData;

  const Acts::Logger& actsLogger() const { return *m_seedingData.actsLogger; }
#elif TRACKSEEDING_HAS_ORTHOGONAL
  // Only Orthogonal available (Acts < 45)
  OrthogonalData m_seedingData;
#else
#error "No seeding method available - check Acts version compatibility"
#endif

  // Resolved seeding method (after resolving Auto)
  SeedingMethod m_resolvedMethod;

  // Shared helper functions (used by both implementations)
  static int determineCharge(std::vector<std::pair<float, float>>& positions,
                             const std::pair<float, float>& PCA,
                             std::tuple<float, float, float>& RX0Y0);
  static std::pair<float, float> findPCA(std::tuple<float, float, float>& circleParams);
  static std::tuple<float, float, float> circleFit(std::vector<std::pair<float, float>>& positions);
  static std::tuple<float, float> lineFit(std::vector<std::pair<float, float>>& positions);

  // Shared core physics calculation for track parameter estimation
  static std::optional<edm4eic::MutableTrackParameters> computeTrackParametersFromFit(
      const std::vector<std::pair<float, float>>& xyPositions,
      const std::vector<std::pair<float, float>>& rzPositions, float vertexZ, float bFieldInZ,
      const std::shared_ptr<const ActsGeometryProvider>& geoSvc, const TrackSeedingConfig& cfg);

#if TRACKSEEDING_HAS_SEEDING2
  // Seeding2-specific: track parameter estimation from space point positions
  static std::optional<edm4eic::MutableTrackParameters>
  estimateTrackParamsFromSeed(const std::array<std::array<float, 3>, 3>& spPositions, float vertexZ,
                              float beamPosX, float beamPosY, float bFieldInZ,
                              const std::shared_ptr<const ActsGeometryProvider>& geoSvc,
                              const TrackSeedingConfig& cfg);
#endif

#if TRACKSEEDING_HAS_ORTHOGONAL
  // Orthogonal-specific: track parameter estimation from Acts::Seed
  std::optional<edm4eic::MutableTrackParameters>
  estimateTrackParamsFromSeed(const Acts::Seed<SpacePoint>& seed) const;
  static std::vector<const eicrecon::SpacePoint*>
  getSpacePoints(const edm4eic::TrackerHitCollection& trk_hits);
#endif
};
} // namespace eicrecon
