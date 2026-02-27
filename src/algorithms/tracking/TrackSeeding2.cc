// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025, EICrecon Authors

#include "TrackSeeding2.h"

#include <Acts/Definitions/Algebra.hpp>
#include <Acts/Definitions/Units.hpp>
#include <Acts/Surfaces/PerigeeSurface.hpp>
#include <Acts/Surfaces/Surface.hpp>
#include <Acts/Utilities/Logger.hpp>
#include <Acts/Utilities/Result.hpp>
#include <edm4eic/EDM4eicVersion.h>
#include <edm4eic/Cov6f.h>
#include <edm4hep/Vector2f.h>
#include <edm4hep/Vector3f.h>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <array>
#include <cmath>
#include <limits>
#include <tuple>

#include "extensions/spdlog/SpdlogToActs.h"

#if Acts_VERSION_MAJOR >= 45
#include <Acts/EventData/SpacePointColumns.hpp>
#include <Acts/EventData/Types.hpp>
#include <Acts/Geometry/Extent.hpp>
#include <Acts/Seeding/SeedConfirmationRangeConfig.hpp>
#include <Acts/Seeding2/CylindricalSpacePointKDTree.hpp>
#include <Acts/Utilities/AxisDefinitions.hpp>
#endif

namespace eicrecon {

void TrackSeeding2::init() {
  m_actsLogger = Acts::getDefaultLogger(
      "TrackSeeding2",
      eicrecon::SpdlogToActsLevel(static_cast<spdlog::level::level_enum>(this->level())));

#if Acts_VERSION_MAJOR >= 45
  m_filterConfig.deltaInvHelixDiameter = m_cfg.deltaInvHelixDiameter;
  m_filterConfig.deltaRMin             = m_cfg.deltaRMin;
  m_filterConfig.compatSeedWeight      = m_cfg.compatSeedWeight;
  m_filterConfig.impactWeightFactor    = m_cfg.impactWeightFactor;
  m_filterConfig.zOriginWeightFactor   = m_cfg.zOriginWeightFactor;
  m_filterConfig.maxSeedsPerSpM        = m_cfg.maxSeedsPerSpM;
  m_filterConfig.compatSeedLimit       = m_cfg.compatSeedLimit;
  m_filterConfig.seedWeightIncrement   = m_cfg.seedWeightIncrement;
  m_filterConfig.seedConfirmation      = m_cfg.seedConfirmation;

  m_filterConfig.centralSeedConfirmationRange = Acts::SeedConfirmationRangeConfig{
      .zMinSeedConf            = m_cfg.zMinSeedConfCentral,
      .zMaxSeedConf            = m_cfg.zMaxSeedConfCentral,
      .rMaxSeedConf            = m_cfg.rMaxSeedConfCentral,
      .nTopForLargeR           = m_cfg.nTopForLargeRCentral,
      .nTopForSmallR           = m_cfg.nTopForSmallRCentral,
      .seedConfMinBottomRadius = m_cfg.seedConfMinBottomRadiusCentral,
      .seedConfMaxZOrigin      = m_cfg.seedConfMaxZOriginCentral,
      .minImpactSeedConf       = m_cfg.minImpactSeedConfCentral};

  m_filterConfig.forwardSeedConfirmationRange = Acts::SeedConfirmationRangeConfig{
      .zMinSeedConf            = m_cfg.zMinSeedConfForward,
      .zMaxSeedConf            = m_cfg.zMaxSeedConfForward,
      .rMaxSeedConf            = m_cfg.rMaxSeedConfForward,
      .nTopForLargeR           = m_cfg.nTopForLargeRForward,
      .nTopForSmallR           = m_cfg.nTopForSmallRForward,
      .seedConfMinBottomRadius = m_cfg.seedConfMinBottomRadiusForward,
      .seedConfMaxZOrigin      = m_cfg.seedConfMaxZOriginForward,
      .minImpactSeedConf       = m_cfg.minImpactSeedConfForward};

  m_seedFinder.emplace();
#endif
}

void TrackSeeding2::process(const Input& input, const Output& output) const {
  const auto [trk_hits]        = input;
  auto [trk_seeds, trk_params] = output;

#if Acts_VERSION_MAJOR >= 45
  // Build SpacePointContainer2 from tracker hits
  Acts::SpacePointContainer2 spacePoints(
      Acts::SpacePointColumns::PackedXY | Acts::SpacePointColumns::PackedZR |
      Acts::SpacePointColumns::Phi | Acts::SpacePointColumns::VarianceZ |
      Acts::SpacePointColumns::VarianceR | Acts::SpacePointColumns::CopyFromIndex);
  spacePoints.reserve(trk_hits->size());

  Acts::Experimental::CylindricalSpacePointKDTreeBuilder kdTreeBuilder;
  kdTreeBuilder.reserve(trk_hits->size());

  Acts::Extent rRangeSPExtent;

  for (std::uint32_t i = 0; i < trk_hits->size(); ++i) {
    const auto hit = (*trk_hits)[i];
    const float hx = static_cast<float>(hit.getPosition()[0]);
    const float hy = static_cast<float>(hit.getPosition()[1]);
    const float hz = static_cast<float>(hit.getPosition()[2]);
    const float hr = std::hypot(hx, hy);

    // variance in r and z from position error matrix
    const float varR = (hx * hx * hit.getPositionError().xx + hy * hy * hit.getPositionError().yy) /
                       (hx * hx + hy * hy + std::numeric_limits<float>::epsilon());
    const float varZ = static_cast<float>(hit.getPositionError().zz);

    Acts::SpacePointIndex2 spIdx = spacePoints.size();
    auto sp                      = spacePoints.createSpacePoint();
    sp.xy()                      = {hx - m_cfg.beamPosX, hy - m_cfg.beamPosY};
    sp.zr()                      = {hz, hr};
    sp.phi()                     = std::atan2(hy, hx);
    sp.varianceZ()               = varZ;
    sp.varianceR()               = varR;
    sp.copyFromIndex()           = i;

    kdTreeBuilder.insert(spIdx, sp.phi(), hr, hz);
    rRangeSPExtent.extend({hx, hy, hz});
  }

  if (kdTreeBuilder.size() == 0) {
    return;
  }

  Acts::Experimental::CylindricalSpacePointKDTree kdTree = kdTreeBuilder.build();

  // Configure KDTree search options for bottom doublets (low→high z, i.e. bottom candidates)
  const float deltaRMinBottom =
      std::isnan(m_cfg.deltaRMinBottom) ? m_cfg.deltaRMin : m_cfg.deltaRMinBottom;
  const float deltaRMaxBottom =
      std::isnan(m_cfg.deltaRMaxBottom) ? m_cfg.deltaRMax : m_cfg.deltaRMaxBottom;
  const float deltaRMinTop = std::isnan(m_cfg.deltaRMinTop) ? m_cfg.deltaRMin : m_cfg.deltaRMinTop;
  const float deltaRMaxTop = std::isnan(m_cfg.deltaRMaxTop) ? m_cfg.deltaRMax : m_cfg.deltaRMaxTop;

  Acts::Experimental::CylindricalSpacePointKDTree::Options lhOptions;
  lhOptions.rMax               = m_cfg.rMax;
  lhOptions.zMin               = m_cfg.zMin;
  lhOptions.zMax               = m_cfg.zMax;
  lhOptions.phiMin             = m_cfg.phiMin;
  lhOptions.phiMax             = m_cfg.phiMax;
  lhOptions.deltaRMin          = deltaRMinBottom;
  lhOptions.deltaRMax          = deltaRMaxBottom;
  lhOptions.collisionRegionMin = m_cfg.collisionRegionMin;
  lhOptions.collisionRegionMax = m_cfg.collisionRegionMax;
  lhOptions.cotThetaMax        = m_cfg.cotThetaMax;
  lhOptions.deltaPhiMax        = m_cfg.deltaPhiMax;

  Acts::Experimental::CylindricalSpacePointKDTree::Options hlOptions = lhOptions;
  hlOptions.deltaRMin                                                = deltaRMinTop;
  hlOptions.deltaRMax                                                = deltaRMaxTop;

  // Bottom doublet finder configuration
  Acts::DoubletSeedFinder::Config bottomCfg;
  bottomCfg.spacePointsSortedByRadius = false;
  bottomCfg.candidateDirection        = Acts::Direction::Backward();
  bottomCfg.deltaRMin                 = deltaRMinBottom;
  bottomCfg.deltaRMax                 = deltaRMaxBottom;
  bottomCfg.deltaZMin                 = m_cfg.deltaZMin;
  bottomCfg.deltaZMax                 = m_cfg.deltaZMax;
  bottomCfg.impactMax                 = m_cfg.impactMax;
  bottomCfg.interactionPointCut       = m_cfg.interactionPointCut;
  bottomCfg.collisionRegionMin        = m_cfg.collisionRegionMin;
  bottomCfg.collisionRegionMax        = m_cfg.collisionRegionMax;
  bottomCfg.cotThetaMax               = m_cfg.cotThetaMax;
  bottomCfg.minPt                     = m_cfg.minPt;
  bottomCfg.helixCutTolerance         = m_cfg.helixCutTolerance;
  auto bottomDoubletFinder            = Acts::DoubletSeedFinder::create(
      Acts::DoubletSeedFinder::DerivedConfig(bottomCfg, m_cfg.bFieldInZ));

  // Top doublet finder configuration
  Acts::DoubletSeedFinder::Config topCfg = bottomCfg;
  topCfg.candidateDirection              = Acts::Direction::Forward();
  topCfg.deltaRMin                       = deltaRMinTop;
  topCfg.deltaRMax                       = deltaRMaxTop;
  auto topDoubletFinder                  = Acts::DoubletSeedFinder::create(
      Acts::DoubletSeedFinder::DerivedConfig(topCfg, m_cfg.bFieldInZ));

  // Triplet finder configuration
  Acts::TripletSeedFinder::Config tripletCfg;
  tripletCfg.useStripInfo      = false;
  tripletCfg.sortedByCotTheta  = true;
  tripletCfg.minPt             = m_cfg.minPt;
  tripletCfg.sigmaScattering   = m_cfg.sigmaScattering;
  tripletCfg.radLengthPerSeed  = m_cfg.radLengthPerSeed;
  tripletCfg.impactMax         = m_cfg.impactMax;
  tripletCfg.helixCutTolerance = m_cfg.helixCutTolerance;
  tripletCfg.toleranceParam    = m_cfg.toleranceParam;
  auto tripletFinder           = Acts::TripletSeedFinder::create(
      Acts::TripletSeedFinder::DerivedConfig(tripletCfg, m_cfg.bFieldInZ));

  // Seed filter
  Acts::BroadTripletSeedFilter::State filterState;
  Acts::BroadTripletSeedFilter::Cache filterCache;
  Acts::BroadTripletSeedFilter seedFilter(m_filterConfig, filterState, filterCache, actsLogger());

  // Determine middle SP r range
  const Acts::Range1D<float> rMiddleSPRange(
      std::floor(rRangeSPExtent.min(Acts::AxisDirection::AxisR) / 2) * 2 +
          m_cfg.deltaRMiddleMinSPRange,
      std::floor(rRangeSPExtent.max(Acts::AxisDirection::AxisR) / 2) * 2 -
          m_cfg.deltaRMiddleMaxSPRange);

  // Run seeding
  static thread_local Acts::TripletSeeder::Cache cache;
  static thread_local Acts::Experimental::CylindricalSpacePointKDTree::Candidates candidates;

  Acts::SeedContainer2 seeds;
  seeds.assignSpacePointContainer(spacePoints);

  for (const auto& middle : kdTree) {
    const auto spM = spacePoints.at(middle.second).asConst();

    const float rM = spM.zr()[1];
    if (m_cfg.useVariableMiddleSPRange) {
      if (rM < rMiddleSPRange.min() || rM > rMiddleSPRange.max()) {
        continue;
      }
    } else {
      if (rM < m_cfg.rMinMiddle || rM > m_cfg.rMaxMiddle) {
        continue;
      }
    }

    const float zM = spM.zr()[0];
    if (zM < m_cfg.zOutermostLayers.first || zM > m_cfg.zOutermostLayers.second) {
      continue;
    }
    if (const float phiM = spM.phi(); phiM > m_cfg.phiMax || phiM < m_cfg.phiMin) {
      continue;
    }

    std::size_t nTopSeedConf = 0;
    if (m_cfg.seedConfirmation) {
      Acts::SeedConfirmationRangeConfig seedConfRange =
          (zM > m_filterConfig.centralSeedConfirmationRange.zMaxSeedConf ||
           zM < m_filterConfig.centralSeedConfirmationRange.zMinSeedConf)
              ? m_filterConfig.forwardSeedConfirmationRange
              : m_filterConfig.centralSeedConfirmationRange;
      nTopSeedConf = rM > seedConfRange.rMaxSeedConf ? seedConfRange.nTopForLargeR
                                                     : seedConfRange.nTopForSmallR;
    }

    candidates.clear();
    kdTree.validTuples(lhOptions, hlOptions, spM, nTopSeedConf, candidates);

    Acts::SpacePointContainer2::ConstSubset bottomSps =
        spacePoints.subset(candidates.bottom_lh_v).asConst();
    Acts::SpacePointContainer2::ConstSubset topSps =
        spacePoints.subset(candidates.top_lh_v).asConst();
    m_seedFinder->createSeedsFromGroup(cache, *bottomDoubletFinder, *topDoubletFinder,
                                       *tripletFinder, seedFilter, spacePoints, bottomSps, spM,
                                       topSps, seeds);

    bottomSps = spacePoints.subset(candidates.bottom_hl_v).asConst();
    topSps    = spacePoints.subset(candidates.top_hl_v).asConst();
    m_seedFinder->createSeedsFromGroup(cache, *bottomDoubletFinder, *topDoubletFinder,
                                       *tripletFinder, seedFilter, spacePoints, bottomSps, spM,
                                       topSps, seeds);
  }

  debug("Created {} track seeds from {} space points", seeds.size(), trk_hits->size());

  // Convert seeds to output collections
  for (auto seed : seeds) {
    auto spIndices = seed.spacePointIndices();
    if (spIndices.size() < 3) {
      continue;
    }

    // Build array of hit positions for track param estimation
    std::array<std::array<float, 3>, 3> positions{};
    bool valid = true;
    for (std::size_t k = 0; k < 3; ++k) {
      const std::uint32_t hitIdx = spacePoints.at(spIndices[k]).copyFromIndex();
      if (hitIdx >= trk_hits->size()) {
        valid = false;
        break;
      }
      const auto hit = (*trk_hits)[hitIdx];
      positions[k]   = {static_cast<float>(hit.getPosition()[0]),
                        static_cast<float>(hit.getPosition()[1]),
                        static_cast<float>(hit.getPosition()[2])};
    }
    if (!valid) {
      continue;
    }

    auto trackParams =
        estimateTrackParamsFromSeed(positions, seed.vertexZ(), m_cfg.bFieldInZ, m_geoSvc, m_cfg);
    if (!trackParams.has_value()) {
      debug("Failed to estimate track parameters from seed");
      continue;
    }
    trk_params->push_back(trackParams.value());

    // Build seed output: look up TrackerHit objects by index
    auto trk_seed = trk_seeds->create();
    trk_seed.setPerigee({0.f, 0.f, 0.f});
#if EDM4EIC_VERSION_MAJOR > 8 || (EDM4EIC_VERSION_MAJOR == 8 && EDM4EIC_VERSION_MINOR > 5)
    trk_seed.setQuality(seed.quality());
#endif
    trk_seed.setParams(trackParams.value());
    for (std::size_t k = 0; k < 3; ++k) {
      const std::uint32_t hitIdx = spacePoints.at(spIndices[k]).copyFromIndex();
      trk_seed.addToHits((*trk_hits)[hitIdx]);
    }
  }
#else
  (void)trk_hits;
  warning("TrackSeeding2 requires Acts >= 45, which is not available. No seeds produced.");
#endif
}

// Track parameter estimation (shared logic with TrackSeeding)
std::optional<edm4eic::MutableTrackParameters> TrackSeeding2::estimateTrackParamsFromSeed(
    const std::array<std::array<float, 3>, 3>& spPositions, float vertexZ, float bFieldInZ,
    const std::shared_ptr<const ActsGeometryProvider>& geoSvc,
    const TripletTrackSeedingConfig& cfg) {

  std::vector<std::pair<float, float>> xyPositions;
  std::vector<std::pair<float, float>> rzPositions;
  xyPositions.reserve(3);
  rzPositions.reserve(3);
  for (const auto& pos : spPositions) {
    xyPositions.emplace_back(pos[0], pos[1]);
    rzPositions.emplace_back(std::hypot(pos[0], pos[1]), pos[2]);
  }

  auto RX0Y0 = circleFit(xyPositions);
  float R    = std::get<0>(RX0Y0);
  float X0   = std::get<1>(RX0Y0);
  float Y0   = std::get<2>(RX0Y0);
  if (!(std::isfinite(R) && std::isfinite(std::abs(X0)) && std::isfinite(std::abs(Y0)))) {
    return {};
  }
  if (std::hypot(X0, Y0) < std::numeric_limits<decltype(std::hypot(X0, Y0))>::epsilon() ||
      !std::isfinite(std::hypot(X0, Y0))) {
    return {};
  }

  auto slopeZ0     = lineFit(rzPositions);
  const auto xypos = findPCA(RX0Y0);
  int charge       = determineCharge(xyPositions, xypos, RX0Y0);

  float theta = std::atan(1.f / std::get<0>(slopeZ0));
  if (theta < 0) {
    theta += static_cast<float>(M_PI);
  }
  float eta    = -std::log(std::tan(theta / 2.f));
  float pt     = R * bFieldInZ;
  float p      = pt * std::cosh(eta);
  float qOverP = static_cast<float>(charge) / p;

  auto xpos  = xypos.first;
  auto ypos  = xypos.second;
  auto vxpos = -1. * charge * (ypos - Y0);
  auto vypos = charge * (xpos - X0);
  auto phi   = std::atan2(vypos, vxpos);

  auto perigee = Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3(0, 0, 0));
  Acts::Vector3 global(xypos.first, xypos.second, vertexZ);
  Acts::Vector3 direction(std::sin(theta) * std::cos(phi), std::sin(theta) * std::sin(phi),
                          std::cos(theta));
  auto local = perigee->globalToLocal(geoSvc->getActsGeometryContext(), global, direction);
  if (!local.ok()) {
    return {};
  }
  Acts::Vector2 localpos = local.value();

  auto trackparam = edm4eic::MutableTrackParameters();
  trackparam.setType(-1);
  trackparam.setLoc({static_cast<float>(localpos(0)), static_cast<float>(localpos(1))});
  trackparam.setPhi(static_cast<float>(phi));
  trackparam.setTheta(theta);
  trackparam.setQOverP(qOverP);
  trackparam.setTime(10);
  edm4eic::Cov6f cov;
  cov(0, 0) = cfg.locaError / Acts::UnitConstants::mm;
  cov(1, 1) = cfg.locbError / Acts::UnitConstants::mm;
  cov(2, 2) = cfg.phiError / Acts::UnitConstants::rad;
  cov(3, 3) = cfg.thetaError / Acts::UnitConstants::rad;
  cov(4, 4) = cfg.qOverPError * Acts::UnitConstants::GeV;
  cov(5, 5) = cfg.timeError / Acts::UnitConstants::ns;
  trackparam.setCovariance(cov);
  return trackparam;
}

std::pair<float, float> TrackSeeding2::findPCA(std::tuple<float, float, float>& circleParams) {
  const float R   = std::get<0>(circleParams);
  const float X0  = std::get<1>(circleParams);
  const float Y0  = std::get<2>(circleParams);
  const double R0 = std::hypot(X0, Y0);
  return {static_cast<float>(X0 * (1. - R / R0)), static_cast<float>(Y0 * (1. - R / R0))};
}

int TrackSeeding2::determineCharge(std::vector<std::pair<float, float>>& positions,
                                   const std::pair<float, float>& PCA,
                                   std::tuple<float, float, float>& RX0Y0) {
  const auto& firstpos = positions.at(0);
  Acts::Vector3 B_z(0, 0, 1);
  Acts::Vector3 radial(std::get<1>(RX0Y0) - PCA.first, std::get<2>(RX0Y0) - PCA.second, 0);
  Acts::Vector3 hit(firstpos.first - PCA.first, firstpos.second - PCA.second, 0);
  return static_cast<int>(std::copysign(1., -radial.cross(hit).dot(B_z)));
}

std::tuple<float, float, float>
TrackSeeding2::circleFit(std::vector<std::pair<float, float>>& positions) {
  double meanX = 0, meanY = 0, weight = 0;
  for (const auto& [x, y] : positions) {
    meanX += x;
    meanY += y;
    ++weight;
  }
  meanX /= weight;
  meanY /= weight;

  double Mxx = 0, Myy = 0, Mxy = 0, Mxz = 0, Myz = 0, Mzz = 0;
  for (auto& [x, y] : positions) {
    double Xi = x - meanX, Yi = y - meanY, Zi = Xi * Xi + Yi * Yi;
    Mxy += Xi * Yi;
    Mxx += Xi * Xi;
    Myy += Yi * Yi;
    Mxz += Xi * Zi;
    Myz += Yi * Zi;
    Mzz += Zi * Zi;
  }
  Mxx /= weight;
  Myy /= weight;
  Mxy /= weight;
  Mxz /= weight;
  Myz /= weight;
  Mzz /= weight;

  const double Mz = Mxx + Myy, Cov_xy = Mxx * Myy - Mxy * Mxy, Var_z = Mzz - Mz * Mz;
  const double A3 = 4 * Mz, A2 = -3 * Mz * Mz - Mzz;
  const double A1  = Var_z * Mz + 4 * Cov_xy * Mz - Mxz * Mxz - Myz * Myz;
  const double A0  = Mxz * (Mxz * Myy - Myz * Mxy) + Myz * (Myz * Mxx - Mxz * Mxy) - Var_z * Cov_xy;
  const double A22 = A2 + A2, A33 = A3 + A3 + A3;

  double x = 0, y = A0;
  for (int iter = 0; iter < 99; ++iter) {
    const double Dy   = A1 + x * (A22 + A33 * x);
    const double xnew = x - y / Dy;
    if (xnew == x || !std::isfinite(xnew))
      break;
    const double ynew = A0 + xnew * (A1 + xnew * (A2 + xnew * A3));
    if (std::abs(ynew) >= std::abs(y))
      break;
    x = xnew;
    y = ynew;
  }

  const double DET     = x * x - x * Mz + Cov_xy;
  const double Xcenter = (Mxz * (Myy - x) - Myz * Mxy) / DET / 2;
  const double Ycenter = (Myz * (Mxx - x) - Mxz * Mxy) / DET / 2;
  return {static_cast<float>(std::sqrt(Xcenter * Xcenter + Ycenter * Ycenter + Mz)),
          static_cast<float>(Xcenter + meanX), static_cast<float>(Ycenter + meanY)};
}

std::tuple<float, float> TrackSeeding2::lineFit(std::vector<std::pair<float, float>>& positions) {
  double xsum = 0, x2sum = 0, ysum = 0, xysum = 0;
  for (const auto& [r, z] : positions) {
    xsum += r;
    ysum += z;
    x2sum += r * r;
    xysum += r * z;
  }
  const auto npts    = static_cast<double>(positions.size());
  const double denom = x2sum * npts - xsum * xsum;
  return {static_cast<float>((xysum * npts - xsum * ysum) / denom),
          static_cast<float>((x2sum * ysum - xsum * xysum) / denom)};
}

} // namespace eicrecon
