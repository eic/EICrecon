// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023  - 2025 Joe Osborn, Dmitry Romanov, Wouter Deconinck

#include "TrackSeeding.h"

#include <Acts/Definitions/Algebra.hpp>
#include <Acts/Definitions/Units.hpp>
#if Acts_VERSION_MAJOR >= 37
#include <Acts/EventData/SpacePointProxy.hpp>
#endif
#include <Acts/Seeding/SeedFinderUtils.hpp>
#if Acts_VERSION_MAJOR >= 37
#include <Acts/EventData/Seed.hpp>
#else
#include <Acts/Seeding/Seed.hpp>
#endif
#include <Acts/Seeding/SeedConfirmationRangeConfig.hpp>
#include <Acts/Seeding/SeedFilter.hpp>
#include <Acts/Seeding/SeedFilterConfig.hpp>
#include <Acts/Seeding/SeedFinderConfig.hpp>
#include <Acts/Seeding/SeedFinderOrthogonal.hpp>
#include <Acts/Seeding/SeedFinderOrthogonalConfig.hpp>
#include <Acts/Surfaces/PerigeeSurface.hpp>
#include <Acts/Surfaces/Surface.hpp>
#include <Acts/Utilities/KDTree.hpp> // IWYU pragma: keep FIXME KDTree missing in SeedFinderOrthogonal.hpp until Acts v23.0.0
#include <Acts/Utilities/Result.hpp>
#include <edm4eic/EDM4eicVersion.h>
#include <edm4eic/Cov6f.h>
#include <edm4hep/Vector2f.h>
#include <edm4hep/Vector3f.h>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <array>
#include <cmath>
#include <gsl/pointers>
#include <limits>
#include <tuple>

namespace eicrecon {

void TrackSeeding::init() {

  // Filter parameters
  m_seedFilterConfig.maxSeedsPerSpM        = m_cfg.maxSeedsPerSpM_filter;
  m_seedFilterConfig.deltaRMin             = m_cfg.deltaRMin;
  m_seedFilterConfig.seedConfirmation      = m_cfg.seedConfirmation;
  m_seedFilterConfig.deltaInvHelixDiameter = m_cfg.deltaInvHelixDiameter;
  m_seedFilterConfig.impactWeightFactor    = m_cfg.impactWeightFactor;
  m_seedFilterConfig.zOriginWeightFactor   = m_cfg.zOriginWeightFactor;
  m_seedFilterConfig.compatSeedWeight      = m_cfg.compatSeedWeight;
  m_seedFilterConfig.compatSeedLimit       = m_cfg.compatSeedLimit;
  m_seedFilterConfig.seedWeightIncrement   = m_cfg.seedWeightIncrement;

  m_seedFilterConfig.centralSeedConfirmationRange = Acts::SeedConfirmationRangeConfig{
      .zMinSeedConf            = m_cfg.zMinSeedConfCentral,
      .zMaxSeedConf            = m_cfg.zMaxSeedConfCentral,
      .rMaxSeedConf            = m_cfg.rMaxSeedConfCentral,
      .nTopForLargeR           = m_cfg.nTopForLargeRCentral,
      .nTopForSmallR           = m_cfg.nTopForSmallRCentral,
      .seedConfMinBottomRadius = m_cfg.seedConfMinBottomRadiusCentral,
      .seedConfMaxZOrigin      = m_cfg.seedConfMaxZOriginCentral,
      .minImpactSeedConf       = m_cfg.minImpactSeedConfCentral};

  m_seedFilterConfig.forwardSeedConfirmationRange = Acts::SeedConfirmationRangeConfig{
      .zMinSeedConf            = m_cfg.zMinSeedConfForward,
      .zMaxSeedConf            = m_cfg.zMaxSeedConfForward,
      .rMaxSeedConf            = m_cfg.rMaxSeedConfForward,
      .nTopForLargeR           = m_cfg.nTopForLargeRForward,
      .nTopForSmallR           = m_cfg.nTopForSmallRForward,
      .seedConfMinBottomRadius = m_cfg.seedConfMinBottomRadiusForward,
      .seedConfMaxZOrigin      = m_cfg.seedConfMaxZOriginForward,
      .minImpactSeedConf       = m_cfg.minImpactSeedConfForward};

#if Acts_VERSION_MAJOR < 42
  m_seedFilterConfig = m_seedFilterConfig.toInternalUnits();
#endif

  // Finder parameters
#if Acts_VERSION_MAJOR >= 37
  m_seedFinderConfig.seedFilter =
      std::make_unique<Acts::SeedFilter<proxy_type>>(m_seedFilterConfig);
#else
  m_seedFinderConfig.seedFilter = std::make_unique<Acts::SeedFilter<eicrecon::SpacePoint>>(
      Acts::SeedFilter<eicrecon::SpacePoint>(m_seedFilterConfig));
#endif
  m_seedFinderConfig.rMax               = m_cfg.rMax;
  m_seedFinderConfig.rMin               = m_cfg.rMin;
  m_seedFinderConfig.deltaRMinTopSP     = m_cfg.deltaRMinTopSP;
  m_seedFinderConfig.deltaRMaxTopSP     = m_cfg.deltaRMaxTopSP;
  m_seedFinderConfig.deltaRMinBottomSP  = m_cfg.deltaRMinBottomSP;
  m_seedFinderConfig.deltaRMaxBottomSP  = m_cfg.deltaRMaxBottomSP;
  m_seedFinderConfig.collisionRegionMin = m_cfg.collisionRegionMin;
  m_seedFinderConfig.collisionRegionMax = m_cfg.collisionRegionMax;
  m_seedFinderConfig.zMin               = m_cfg.zMin;
  m_seedFinderConfig.zMax               = m_cfg.zMax;
  m_seedFinderConfig.maxSeedsPerSpM     = m_cfg.maxSeedsPerSpM;
  m_seedFinderConfig.cotThetaMax        = m_cfg.cotThetaMax;
  m_seedFinderConfig.sigmaScattering    = m_cfg.sigmaScattering;
  m_seedFinderConfig.radLengthPerSeed   = m_cfg.radLengthPerSeed;
  m_seedFinderConfig.minPt              = m_cfg.minPt;
  m_seedFinderConfig.impactMax          = m_cfg.impactMax;
  m_seedFinderConfig.rMinMiddle         = m_cfg.rMinMiddle;
  m_seedFinderConfig.rMaxMiddle         = m_cfg.rMaxMiddle;
  m_seedFinderConfig.deltaPhiMax        = m_cfg.deltaPhiMax;

  m_seedFinderOptions.beamPos   = Acts::Vector2(m_cfg.beamPosX, m_cfg.beamPosY);
  m_seedFinderOptions.bFieldInZ = m_cfg.bFieldInZ;

  m_seedFinderConfig = m_seedFinderConfig
#if Acts_VERSION_MAJOR < 42
                           .toInternalUnits()
#endif
                           .calculateDerivedQuantities();
  m_seedFinderOptions = m_seedFinderOptions
#if Acts_VERSION_MAJOR < 42
                            .toInternalUnits()
#endif
                            .calculateDerivedQuantities(m_seedFinderConfig);
}

void TrackSeeding::process(const Input& input, const Output& output) const {

  const auto [trk_hits]        = input;
  auto [trk_seeds, trk_params] = output;

  std::vector<const eicrecon::SpacePoint*> spacePoints = getSpacePoints(*trk_hits);

#if Acts_VERSION_MAJOR >= 37
  Acts::SeedFinderOrthogonal<proxy_type> finder(m_seedFinderConfig); // FIXME move into class scope
#else
  Acts::SeedFinderOrthogonal<eicrecon::SpacePoint> finder(
      m_seedFinderConfig); // FIXME move into class scope
#endif

#if Acts_VERSION_MAJOR >= 37
  // Config
  Acts::SpacePointContainerConfig spConfig;

  // Options
  Acts::SpacePointContainerOptions spOptions;
  spOptions.beamPos = {0., 0.};

  ActsExamples::SpacePointContainer container(spacePoints);
  Acts::SpacePointContainer<decltype(container), Acts::detail::RefHolder> spContainer(
      spConfig, spOptions, container);

  std::vector<Acts::Seed<proxy_type>> seeds = finder.createSeeds(m_seedFinderOptions, spContainer);

  // need to convert here from seed of proxies to seed of sps
  for (const auto& seed : seeds) {
    const auto& sps = seed.sp();

    auto seedToAdd = Acts::Seed<eicrecon::SpacePoint>(*sps[0]->externalSpacePoint(),
                                                      *sps[1]->externalSpacePoint(),
                                                      *sps[2]->externalSpacePoint());
    seedToAdd.setVertexZ(seed.z());
    seedToAdd.setQuality(seed.seedQuality());

    // Estimate track parameters
    auto trackParams = estimateTrackParamsFromSeed(seedToAdd);
    if (!trackParams.has_value()) {
      debug("Failed to estimate track parameters from seed");
      continue;
    }
    trk_params->push_back(trackParams.value());

    // Add seed to collection
    auto trk_seed = trk_seeds->create();
    trk_seed.setPerigee({0.f, 0.f, 0.f});
#if EDM4EIC_VERSION_MAJOR > 8 || (EDM4EIC_VERSION_MAJOR == 8 && EDM4EIC_VERSION_MINOR > 5)
    trk_seed.setQuality(seedToAdd.getQuality());
#endif
    trk_seed.setParams(trackParams.value());
    trk_seed.addToHits(*sps[0]->externalSpacePoint());
    trk_seed.addToHits(*sps[1]->externalSpacePoint());
    trk_seed.addToHits(*sps[2]->externalSpacePoint());
  }

#else

  std::function<std::tuple<Acts::Vector3, Acts::Vector2, std::optional<Acts::ActsScalar>>(
      const eicrecon::SpacePoint* sp)>
      create_coordinates = [](const eicrecon::SpacePoint* sp) {
        Acts::Vector3 position(sp->x(), sp->y(), sp->z());
        Acts::Vector2 variance(sp->varianceR(), sp->varianceZ());
        return std::make_tuple(position, variance, sp->t());
      };

  eicrecon::SeedContainer seeds =
      finder.createSeeds(m_seedFinderOptions, spacePoints, create_coordinates);

  for (const auto& seed : seeds) {
    // Estimate track parameters
    auto trackParams = estimateTrackParamsFromSeed(seed);
    if (!trackParams.has_value()) {
      debug("Failed to estimate track parameters from seed");
      continue;
    }
    trk_params->push_back(trackParams.value());

    // Add seed to collection
    auto trk_seed = trk_seeds->create();
    trk_seed.setPerigee({0.f, 0.f, 0.f});
    trk_seed.setParams(trackParams.value());
    // hits are not stored for older Acts versions
  }

#endif

  for (auto& sp : spacePoints) {
    delete sp;
  }
}

std::vector<const eicrecon::SpacePoint*>
TrackSeeding::getSpacePoints(const edm4eic::TrackerHitCollection& trk_hits) {
  std::vector<const eicrecon::SpacePoint*> spacepoints;

  for (const auto hit : trk_hits) {
    const eicrecon::SpacePoint* sp = new SpacePoint(hit);
    spacepoints.push_back(sp);
  }

  return spacepoints;
}

std::optional<edm4eic::MutableTrackParameters>
TrackSeeding::estimateTrackParamsFromSeed(const Acts::Seed<SpacePoint>& seed) const {
  std::vector<std::pair<float, float>> xyHitPositions;
  std::vector<std::pair<float, float>> rzHitPositions;
  for (const auto& spptr : seed.sp()) {
    xyHitPositions.emplace_back(spptr->x(), spptr->y());
    rzHitPositions.emplace_back(spptr->r(), spptr->z());
  }

  auto RX0Y0 = circleFit(xyHitPositions);
  float R    = std::get<0>(RX0Y0);
  float X0   = std::get<1>(RX0Y0);
  float Y0   = std::get<2>(RX0Y0);
  if (!(std::isfinite(R) && std::isfinite(std::abs(X0)) && std::isfinite(std::abs(Y0)))) {
    // avoid float overflow for hits on a line
    return {};
  }
  if (std::hypot(X0, Y0) < std::numeric_limits<decltype(std::hypot(X0, Y0))>::epsilon() ||
      !std::isfinite(std::hypot(X0, Y0))) {
    //Avoid center of circle at origin, where there is no point-of-closest approach
    //Also, avoid float overflow on circle center
    return {};
  }

  auto slopeZ0     = lineFit(rzHitPositions);
  const auto xypos = findPCA(RX0Y0);

  //Determine charge
  int charge = determineCharge(xyHitPositions, xypos, RX0Y0);

  float theta = atan(1. / std::get<0>(slopeZ0));
  // normalize to 0<theta<pi
  if (theta < 0) {
    theta += M_PI;
  }
  float eta    = -log(tan(theta / 2.));
  float pt     = R * m_cfg.bFieldInZ; // pt[GeV] = R[mm] * B[GeV/mm]
  float p      = pt * cosh(eta);
  float qOverP = charge / p;

  //Calculate phi at xypos
  auto xpos = xypos.first;
  auto ypos = xypos.second;

  auto vxpos = -1. * charge * (ypos - Y0);
  auto vypos = charge * (xpos - X0);

  auto phi = atan2(vypos, vxpos);

  const float z0 = seed.z();
  auto perigee   = Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3(0, 0, 0));
  Acts::Vector3 global(xypos.first, xypos.second, z0);

  //Compute local position at PCA
  Acts::Vector2 localpos;
  Acts::Vector3 direction(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));

  auto local = perigee->globalToLocal(m_geoSvc->getActsGeometryContext(), global, direction);

  if (!local.ok()) {
    return {};
  }

  localpos = local.value();

  auto trackparam = edm4eic::MutableTrackParameters();
  trackparam.setType(-1); // type --> seed(-1)
  trackparam.setLoc(
      {static_cast<float>(localpos(0)), static_cast<float>(localpos(1))}); // 2d location on surface
  trackparam.setPhi(static_cast<float>(phi));                              // phi [rad]
  trackparam.setTheta(theta);                                              //theta [rad]
  trackparam.setQOverP(qOverP);                                            // Q/p [e/GeV]
  trackparam.setTime(10);                                                  // time in ns
  edm4eic::Cov6f cov;
  cov(0, 0) = m_cfg.locaError / Acts::UnitConstants::mm;    // loc0
  cov(1, 1) = m_cfg.locbError / Acts::UnitConstants::mm;    // loc1
  cov(2, 2) = m_cfg.phiError / Acts::UnitConstants::rad;    // phi
  cov(3, 3) = m_cfg.thetaError / Acts::UnitConstants::rad;  // theta
  cov(4, 4) = m_cfg.qOverPError * Acts::UnitConstants::GeV; // qOverP
  cov(5, 5) = m_cfg.timeError / Acts::UnitConstants::ns;    // time
  trackparam.setCovariance(cov);

  return trackparam;
}

std::pair<float, float> TrackSeeding::findPCA(std::tuple<float, float, float>& circleParams) {
  const float R  = std::get<0>(circleParams);
  const float X0 = std::get<1>(circleParams);
  const float Y0 = std::get<2>(circleParams);

  const double R0 = std::hypot(X0, Y0);

  //Calculate point on circle closest to origin
  const double xmin = X0 * (1. - R / R0);
  const double ymin = Y0 * (1. - R / R0);

  return std::make_pair(xmin, ymin);
}

int TrackSeeding::determineCharge(std::vector<std::pair<float, float>>& positions,
                                  const std::pair<float, float>& PCA,
                                  std::tuple<float, float, float>& RX0Y0) {

  const auto& firstpos = positions.at(0);
  auto hit_x           = firstpos.first;
  auto hit_y           = firstpos.second;

  auto xpos = PCA.first;
  auto ypos = PCA.second;

  float X0 = std::get<1>(RX0Y0);
  float Y0 = std::get<2>(RX0Y0);

  Acts::Vector3 B_z(0, 0, 1);
  Acts::Vector3 radial(X0 - xpos, Y0 - ypos, 0);
  Acts::Vector3 hit(hit_x - xpos, hit_y - ypos, 0);

  auto cross = radial.cross(hit);

  float dot = cross.dot(B_z);

  return copysign(1., -dot);
}

/**
   * Circle fit to a given set of data points (in 2D)
   * This is an algebraic fit, due to Taubin, based on the journal article
   * G. Taubin, "Estimation Of Planar Curves, Surfaces And Nonplanar
   * Space Curves Defined By Implicit Equations, With
   * Applications To Edge And Range Image Segmentation",
   * IEEE Trans. PAMI, Vol. 13, pages 1115-1138, (1991)
   * It works well whether data points are sampled along an entire circle or along a small arc.
   * It still has a small bias and its statistical accuracy is slightly lower than that of the geometric fit (minimizing geometric distances),
   * It provides a very good initial guess for a subsequent geometric fit.
   * Nikolai Chernov  (September 2012)
   */
std::tuple<float, float, float>
TrackSeeding::circleFit(std::vector<std::pair<float, float>>& positions) {
  // Compute x- and y- sample means
  double meanX  = 0;
  double meanY  = 0;
  double weight = 0;

  for (const auto& [x, y] : positions) {
    meanX += x;
    meanY += y;
    ++weight;
  }
  meanX /= weight;
  meanY /= weight;

  //     computing moments

  double Mxx = 0;
  double Myy = 0;
  double Mxy = 0;
  double Mxz = 0;
  double Myz = 0;
  double Mzz = 0;

  for (auto& [x, y] : positions) {
    double Xi = x - meanX; //  centered x-coordinates
    double Yi = y - meanY; //  centered y-coordinates
    double Zi = std::pow(Xi, 2) + std::pow(Yi, 2);

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

  //  computing coefficients of the characteristic polynomial

  const double Mz     = Mxx + Myy;
  const double Cov_xy = Mxx * Myy - Mxy * Mxy;
  const double Var_z  = Mzz - Mz * Mz;
  const double A3     = 4 * Mz;
  const double A2     = -3 * Mz * Mz - Mzz;
  const double A1     = Var_z * Mz + 4 * Cov_xy * Mz - Mxz * Mxz - Myz * Myz;
  const double A0  = Mxz * (Mxz * Myy - Myz * Mxy) + Myz * (Myz * Mxx - Mxz * Mxy) - Var_z * Cov_xy;
  const double A22 = A2 + A2;
  const double A33 = A3 + A3 + A3;

  //    finding the root of the characteristic polynomial
  //    using Newton's method starting at x=0
  //    (it is guaranteed to converge to the right root)
  static constexpr int iter_max = 99;
  double x                      = 0;
  double y                      = A0;

  // usually, 4-6 iterations are enough
  for (int iter = 0; iter < iter_max; ++iter) {
    const double Dy   = A1 + x * (A22 + A33 * x);
    const double xnew = x - y / Dy;
    if ((xnew == x) || (!std::isfinite(xnew))) {
      break;
    }

    const double ynew = A0 + xnew * (A1 + xnew * (A2 + xnew * A3));
    if (std::abs(ynew) >= std::abs(y)) {
      break;
    }

    x = xnew;
    y = ynew;
  }

  //  computing parameters of the fitting circle
  const double DET     = std::pow(x, 2) - x * Mz + Cov_xy;
  const double Xcenter = (Mxz * (Myy - x) - Myz * Mxy) / DET / 2;
  const double Ycenter = (Myz * (Mxx - x) - Mxz * Mxy) / DET / 2;

  //  assembling the output
  float X0 = Xcenter + meanX;
  float Y0 = Ycenter + meanY;
  float R  = std::sqrt(std::pow(Xcenter, 2) + std::pow(Ycenter, 2) + Mz);
  return std::make_tuple(R, X0, Y0);
}

std::tuple<float, float> TrackSeeding::lineFit(std::vector<std::pair<float, float>>& positions) {
  double xsum  = 0;
  double x2sum = 0;
  double ysum  = 0;
  double xysum = 0;
  for (const auto& [r, z] : positions) {
    xsum  = xsum + r;               //calculate sigma(xi)
    ysum  = ysum + z;               //calculate sigma(yi)
    x2sum = x2sum + std::pow(r, 2); //calculate sigma(x^2i)
    xysum = xysum + r * z;          //calculate sigma(xi*yi)
  }

  const auto npts          = positions.size();
  const double denominator = (x2sum * npts - std::pow(xsum, 2));
  const float a            = (xysum * npts - xsum * ysum) / denominator;  //calculate slope
  const float b            = (x2sum * ysum - xsum * xysum) / denominator; //calculate intercept
  return std::make_tuple(a, b);
}

} // namespace eicrecon
