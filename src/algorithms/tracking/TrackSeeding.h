// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023  - 2025 Joe Osborn, Dmitry Romanov, Wouter Deconinck// Created by Dmitry Romanov

#pragma once

#include <Acts/EventData/SpacePointContainer.hpp>
#include <Acts/EventData/Seed.hpp>
#include <Acts/Seeding/SeedFilterConfig.hpp>
#include <Acts/Seeding/SeedFinderConfig.hpp>
#include <Acts/Seeding/SeedFinderOrthogonalConfig.hpp>
#include <Acts/Utilities/Holders.hpp>
#if __has_include(<ActsExamples/EventData/SpacePointContainer.hpp>)
#  include <ActsExamples/EventData/SpacePointContainer.hpp>
#else
#  include <any>
#  include <stdexcept>
#endif
#include <algorithms/algorithm.h>
#include <edm4eic/TrackParametersCollection.h>
#include <edm4eic/TrackSeedCollection.h>
#include <edm4eic/TrackerHitCollection.h>
#include <cmath>
#include <iterator>
#include <memory>
#include <optional>
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

#if !__has_include(<ActsExamples/EventData/SpacePointContainer.hpp>)
/// Adapter to wrap a collection of space points for use with Acts::SpacePointContainer.
/// This replaces ActsExamples::SpacePointContainer<T>, which was removed in Acts >= 46
/// (see https://github.com/acts-project/acts/pull/5088).
template <typename collection_t>
class SpacePointContainerAdapter {
 public:
  using CollectionType = collection_t;
  using ValueType = typename CollectionType::value_type;

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

using TrackSeedingAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4eic::TrackerHitCollection>,
    algorithms::Output<edm4eic::TrackSeedCollection, edm4eic::TrackParametersCollection>>;

class TrackSeeding : public TrackSeedingAlgorithm,
                     public WithPodConfig<OrthogonalTrackSeedingConfig> {
public:
#if __has_include(<ActsExamples/EventData/SpacePointContainer.hpp>)
  using SpacePointContainerType =
      ActsExamples::SpacePointContainer<std::vector<const SpacePoint*>>;
#else
  using SpacePointContainerType =
      SpacePointContainerAdapter<std::vector<const SpacePoint*>>;
#endif
  using proxy_type = typename Acts::SpacePointContainer<SpacePointContainerType,
                                                        Acts::detail::RefHolder>::SpacePointProxyType;

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
  Acts::SeedFinderOrthogonalConfig<proxy_type> m_seedFinderConfig;

  static int determineCharge(std::vector<std::pair<float, float>>& positions,
                             const std::pair<float, float>& PCA,
                             std::tuple<float, float, float>& RX0Y0);
  static std::pair<float, float> findPCA(std::tuple<float, float, float>& circleParams);
  static std::vector<const eicrecon::SpacePoint*>
  getSpacePoints(const edm4eic::TrackerHitCollection& trk_hits);
  std::optional<edm4eic::MutableTrackParameters>
  estimateTrackParamsFromSeed(const Acts::Seed<SpacePoint>& seed) const;

  static std::tuple<float, float, float> circleFit(std::vector<std::pair<float, float>>& positions);
  static std::tuple<float, float> lineFit(std::vector<std::pair<float, float>>& positions);
};
} // namespace eicrecon
