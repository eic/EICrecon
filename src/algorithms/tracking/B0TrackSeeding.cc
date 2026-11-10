// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Tom Bleher

#include "B0TrackSeeding.h"

#include <Acts/Definitions/Algebra.hpp>
#include <Acts/Definitions/TrackParametrization.hpp>
#include <Acts/Definitions/Units.hpp>
#include <Acts/Seeding/EstimateTrackParamsFromSeed.hpp>
#include <Acts/Utilities/Logger.hpp>
#include <Acts/Utilities/MathHelpers.hpp>
#include <Acts/Utilities/Result.hpp>
#include <Acts/Utilities/UnitVectors.hpp>
#include <Acts/Utilities/VectorHelpers.hpp>
#include <edm4eic/Cov6f.h>
#include <edm4eic/CovDiag3f.h>
#include <edm4eic/unit_system.h>
#include <edm4hep/Vector2f.h>
#include <edm4hep/Vector3f.h>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <span>
#include <tuple>
#include <utility>
#include <vector>

#include "ActsGeometryProvider.h"
#include "B0TrackSeedingConfig.h"
#include "TrackSeeding.h"

// Acts version-specific includes
#if TRACKSEEDING_HAS_SEEDING
#include <Acts/Definitions/Direction.hpp>
#include <Acts/EventData/SeedContainer.hpp>
#include <Acts/EventData/SeedProxy.hpp>
#include <Acts/EventData/SpacePointColumnProxy.hpp>
#include <Acts/EventData/SpacePointColumns.hpp>
#include <Acts/EventData/SpacePointContainer.hpp>
#include <Acts/EventData/SpacePointProxy.hpp>
#include <Acts/Seeding/BroadTripletSeedFilter.hpp>
#include <Acts/Seeding/DoubletSeedFinder.hpp>
#include <Acts/Seeding/TripletSeedFinder.hpp>
#include <Acts/Seeding/TripletSeeder.hpp>

namespace Acts {
using SeedContainer2       = Acts::SeedContainer;
using SpacePointContainer2 = Acts::SpacePointContainer;
using ConstSeedProxy2      = Acts::ConstSeedProxy;
} // namespace Acts
#endif

#if TRACKSEEDING_HAS_SEEDING2
#include <Acts/Definitions/Direction.hpp>
#include <Acts/EventData/SeedContainer2.hpp>
#include <Acts/EventData/SeedProxy2.hpp>
#include <Acts/EventData/SpacePointColumns.hpp>
#include <Acts/EventData/SpacePointContainer2.hpp>
#include <Acts/EventData/SpacePointProxy2.hpp>
#include <Acts/Seeding2/BroadTripletSeedFilter.hpp>
#include <Acts/Seeding2/DoubletSeedFinder.hpp>
#include <Acts/Seeding2/TripletSeedFinder.hpp>
#include <Acts/Seeding2/TripletSeeder.hpp>
#endif

#if TRACKSEEDING_HAS_SEEDING2 || TRACKSEEDING_HAS_SEEDING

namespace eicrecon {

namespace {

  /// Opens a cut of the Acts finders, in Acts units
  constexpr float noLimit = 1.e5 * Acts::UnitConstants::mm;

  /// Adds the seed of an Acts triplet on a helix in the field at its middle hit, unless it has no
  /// finite non-zero q/p. Space point i is hit i, at positions[i] in Acts units.
  void addSeed(const B0TrackSeedingConfig& cfg, const Acts::ConstSeedProxy2& actsSeed,
               const edm4eic::TrackerHitCollection& hits, std::span<const Acts::Vector3> positions,
               const Acts::Vector3& field, edm4eic::TrackSeedCollection& seeds,
               edm4eic::TrackParametersCollection& parameters) {
    const auto triplet           = actsSeed.spacePointIndices();
    const Acts::Vector3& first   = positions[triplet[0]];
    const Acts::FreeVector helix = Acts::estimateTrackParamsFromSeed(
        first, 0., positions[triplet[1]], positions[triplet[2]], field);
    const double qOverP = helix[Acts::eFreeQOverP] * Acts::UnitConstants::GeV / edm4eic::unit::GeV;
    if (!helix.allFinite() || qOverP == 0.) {
      return;
    }

    // CKFTracking rebuilds the perigee along global z from the seed's point, so the seed is at
    // loc0 = loc1 = 0. Forward tracks run nearly parallel to it, so the loc1 and phi errors are
    // scaled up.
    const Acts::Vector3 direction = helix.segment<3>(Acts::eFreeDir0);
    const double theta            = Acts::VectorHelpers::theta(direction);
    const double sinTheta         = std::sin(theta);
    // The perigee is not defined for a track along its axis
    if (!(sinTheta > 0.)) {
      return;
    }

    edm4eic::Cov6f cov;
    cov(0, 0) = static_cast<float>(Acts::square(cfg.positionError));
    cov(1, 1) = static_cast<float>(Acts::square(cfg.positionError / sinTheta));
    cov(2, 2) = static_cast<float>(Acts::square(cfg.angleError / sinTheta));
    cov(3, 3) = static_cast<float>(Acts::square(cfg.angleError));
    cov(4, 4) = static_cast<float>(Acts::square(cfg.qOverPRelativeError * qOverP));
    cov(5, 5) = static_cast<float>(Acts::square(cfg.timeError));

    auto params = parameters.create();
    params.setType(-1); // seed
    params.setLoc({0.F, 0.F});
    params.setPhi(static_cast<float>(Acts::VectorHelpers::phi(direction)));
    params.setTheta(static_cast<float>(theta));
    params.setQOverP(static_cast<float>(qOverP));
    params.setTime(hits[triplet[0]].getTime());
    params.setCovariance(cov);

    const Acts::Vector3 anchor =
        (first - cfg.anchorDistance * direction) / Acts::UnitConstants::mm * edm4eic::unit::mm;
    auto seed = seeds.create();
    seed.setPerigee({static_cast<float>(anchor.x()), static_cast<float>(anchor.y()),
                     static_cast<float>(anchor.z())});
    seed.setQuality(actsSeed.quality());
    seed.setParams(params);
    for (const auto i : triplet) {
      seed.addToHits(hits[i]);
    }
  }

} // namespace

void B0TrackSeeding::process(const Input& input, const Output& output) const {
  const auto [hits]        = input;
  auto [seeds, parameters] = output;
  findSeeds(*hits, *m_geoSvc->getFieldProvider(), m_geoSvc->getActsMagneticFieldContext(), *seeds,
            *parameters);
  debug("Created {} track seeds from {} hits", seeds->size(), hits->size());
}

void B0TrackSeeding::findSeeds(const edm4eic::TrackerHitCollection& hits,
                               const Acts::MagneticFieldProvider& fieldProvider,
                               const Acts::MagneticFieldContext& fieldContext,
                               edm4eic::TrackSeedCollection& seeds,
                               edm4eic::TrackParametersCollection& parameters) const {
  // The Acts seeder, with only what differs from its defaults. Every hit is tried as a middle hit;
  // in its frame, r' is about the distance along the beam and z' lies along the local field.
  // Doublets span stations. Tracks may come from decays upstream, so the origin is not cut on.
  Acts::DoubletSeedFinder::Config topConfig;
  topConfig.spacePointsSortedByRadius = false;
  topConfig.deltaRMin                 = m_cfg.stationGap;
  topConfig.deltaRMax                 = noLimit;
  topConfig.collisionRegionMin        = -noLimit;
  topConfig.collisionRegionMax        = noLimit;

  auto bottomConfig               = topConfig;
  bottomConfig.candidateDirection = Acts::Direction::Backward();

  Acts::TripletSeedFinder::Config tripletConfig;
  tripletConfig.minPt     = m_cfg.minPt;
  tripletConfig.impactMax = noLimit;

  // Seeds of a middle hit rank by the hits on other stations that confirm them. A confirming hit
  // only has to pass the slope cut: above about 10 GeV the curvature window of the filter
  // (deltaInvHelixDiameter) is wider than the curvature itself.
  // The impact parameter here comes from extrapolating the circle through the field-free beam line,
  // of order 100 mm at 41 GeV, not from where the track came from. The Acts default weight of 1 per
  // mm would make it compete with a confirming hit (+200), so it is lowered to a tie-break between
  // seeds of equal confirmation.
  Acts::BroadTripletSeedFilter::Config filterConfig;
  filterConfig.deltaRMin          = m_cfg.stationGap;
  filterConfig.impactWeightFactor = 1.e-4F;

  // Space point i is hit i, with its position in the frame of the current middle hit. All hits are
  // bottom and top candidates: the doublet finders keep those at least deltaRMin from the middle.
  const double toActs = Acts::UnitConstants::mm / edm4eic::unit::mm;
  std::vector<Acts::Vector3> positions;
  Acts::SpacePointContainer2 spacePoints(
      Acts::SpacePointColumns::PackedXY | Acts::SpacePointColumns::PackedZR |
      Acts::SpacePointColumns::VarianceZ | Acts::SpacePointColumns::VarianceR);
  for (const auto& hit : hits) {
    const auto pos = hit.getPosition();
    const auto cov = hit.getPositionError();
    positions.emplace_back(toActs * Acts::Vector3(pos.x, pos.y, pos.z));
    auto sp = spacePoints.createSpacePoint();
    sp.varianceZ() =
        static_cast<float>(Acts::square(toActs * m_cfg.hitErrorScale) * std::max(cov.xx, cov.yy));
    sp.varianceR() = sp.varianceZ();
  }
  std::array candidates{spacePoints.range({0, spacePoints.size()}).asConst()};

  Acts::BroadTripletSeedFilter::State filterState;
  Acts::BroadTripletSeedFilter::Cache filterCache;
  const Acts::BroadTripletSeedFilter filter(filterConfig, filterState, filterCache,
                                            Acts::getDummyLogger());
  const Acts::TripletSeeder seeder;
  Acts::TripletSeeder::Cache seederCache;
  Acts::SeedContainer2 actsSeeds;

  auto fieldCache = fieldProvider.makeCache(fieldContext);

  for (std::uint32_t m = 0; m < hits.size(); ++m) {
    const auto fieldResult = fieldProvider.getField(positions[m], fieldCache);
    // No field outside the field map
    if (!fieldResult.ok() || !(fieldResult.value().norm() > 0.)) {
      continue;
    }
    const Acts::Vector3 field = fieldResult.value();
    const auto bField         = static_cast<float>(field.norm());

    const Acts::Vector3 fieldDirection = field.normalized();
    const auto [u, v]                  = Acts::createCurvilinearUnitVectors(fieldDirection);
    for (std::uint32_t i = 0; i < hits.size(); ++i) {
      const auto x = static_cast<float>(u.dot(positions[i]));
      const auto y = static_cast<float>(v.dot(positions[i]));
      auto sp      = spacePoints[i];
      sp.xy()      = {x, y};
      sp.zr()      = {static_cast<float>(fieldDirection.dot(positions[i])), std::hypot(x, y)};
    }

    // The Acts finders take |B| when they are created. Unlike in TrackSeeding, it cannot be fixed
    // once in init(): it varies by about a factor 2 across the B0 tracker and with the beam setup.
    // The middle radius range only applies to hits sorted in radius.
    actsSeeds.clear();
    seeder.createSeedsFromGroups(
        seederCache, *Acts::DoubletSeedFinder::create({bottomConfig, bField}),
        *Acts::DoubletSeedFinder::create({topConfig, bField}),
        *Acts::TripletSeedFinder::create({tripletConfig, bField}), filter, spacePoints, candidates,
        spacePoints.range({m, m + 1}).asConst(), candidates, {0.F, 0.F}, actsSeeds);

    for (const Acts::ConstSeedProxy2 actsSeed : std::as_const(actsSeeds)) {
      addSeed(m_cfg, actsSeed, hits, positions, field, seeds, parameters);
    }
  }
}

} // namespace eicrecon

#else

namespace eicrecon {

// Acts older than 45.3 has no Seeding2 API: no B0 seeds, as with TrackSeeding before
void B0TrackSeeding::process(const Input& /*input*/, const Output& /*output*/) const {}

void B0TrackSeeding::findSeeds(const edm4eic::TrackerHitCollection& /*hits*/,
                               const Acts::MagneticFieldProvider& /*fieldProvider*/,
                               const Acts::MagneticFieldContext& /*fieldContext*/,
                               edm4eic::TrackSeedCollection& /*seeds*/,
                               edm4eic::TrackParametersCollection& /*parameters*/) const {}

} // namespace eicrecon

#endif
