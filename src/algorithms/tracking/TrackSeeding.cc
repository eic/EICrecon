// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include "TrackSeeding.h"

#include <Acts/Definitions/Algebra.hpp>
#include <Acts/Definitions/Units.hpp>
#include <Acts/Seeding/Seed.hpp>
#include <Acts/Seeding/SeedConfirmationRangeConfig.hpp>
#include <Acts/Seeding/SeedFilter.hpp>
#include <Acts/Seeding/SeedFilter.ipp>
#include <Acts/Seeding/SeedFilterConfig.hpp>
#include <Acts/Seeding/SeedFinderOrthogonal.hpp>
#include <Acts/Seeding/SeedFinderOrthogonal.ipp>
#include <Acts/Seeding/SeedFinderOrthogonalConfig.hpp>
#include <Acts/Surfaces/PerigeeSurface.hpp>
#include <Acts/Surfaces/Surface.hpp>
#include <Acts/Utilities/Result.hpp>
#include <Eigen/src/Core/DenseCoeffsBase.h>
#include <bits/std_abs.h>
#include <bits/utility.h>
#include <boost/container/small_vector.hpp>
#include <boost/container/vector.hpp>
#include <edm4eic/TrackParameters.h>
#include <cmath>
#include <iterator>
#include <limits>
#include <ostream>
#include <tuple>
#include <type_traits>
#include <variant>

#include "ActsGeometryProvider.h"
#include "DD4hepBField.h"

namespace edm4eic { class TrackerHit; }
namespace spdlog { class logger; }

namespace
{
  //! convenience square method
  template<class T>
    inline constexpr T square( const T& x ) { return x*x; }
}

void eicrecon::TrackSeeding::init(std::shared_ptr<const ActsGeometryProvider> geo_svc, std::shared_ptr<spdlog::logger> log) {

    m_log = log;

    m_geoSvc = geo_svc;

    m_BField = std::dynamic_pointer_cast<const eicrecon::BField::DD4hepBField>(m_geoSvc->getFieldProvider());
    m_fieldctx = eicrecon::BField::BFieldVariant(m_BField);

    configure();
}

void eicrecon::TrackSeeding::configure() {

    // Filter parameters
    m_seedFilterConfig.maxSeedsPerSpM = m_cfg.m_maxSeedsPerSpM_filter;
    m_seedFilterConfig.deltaRMin = m_cfg.m_deltaRMin;
    m_seedFilterConfig.seedConfirmation = m_cfg.m_seedConfirmation;
    m_seedFilterConfig.deltaInvHelixDiameter = m_cfg.m_deltaInvHelixDiameter;
    m_seedFilterConfig.impactWeightFactor = m_cfg.m_impactWeightFactor;
    m_seedFilterConfig.zOriginWeightFactor = m_cfg.m_zOriginWeightFactor;
    m_seedFilterConfig.compatSeedWeight = m_cfg.m_compatSeedWeight;
    m_seedFilterConfig.compatSeedLimit = m_cfg.m_compatSeedLimit;
    m_seedFilterConfig.curvatureSortingInFilter = m_cfg.m_curvatureSortingInFilter;
    m_seedFilterConfig.seedWeightIncrement = m_cfg.m_seedWeightIncrement;

    m_seedFilterConfig.centralSeedConfirmationRange = Acts::SeedConfirmationRangeConfig{
      m_cfg.m_zMinSeedConf_cent,
      m_cfg.m_zMaxSeedConf_cent,
      m_cfg.m_rMaxSeedConf_cent,
      m_cfg.m_nTopForLargeR_cent,
      m_cfg.m_nTopForSmallR_cent,
      m_cfg.m_seedConfMinBottomRadius_cent,
      m_cfg.m_seedConfMaxZOrigin_cent,
      m_cfg.m_minImpactSeedConf_cent
    };

    m_seedFilterConfig.forwardSeedConfirmationRange = Acts::SeedConfirmationRangeConfig{
      m_cfg.m_zMinSeedConf_forw,
      m_cfg.m_zMaxSeedConf_forw,
      m_cfg.m_rMaxSeedConf_forw,
      m_cfg.m_nTopForLargeR_forw,
      m_cfg.m_nTopForSmallR_forw,
      m_cfg.m_seedConfMinBottomRadius_forw,
      m_cfg.m_seedConfMaxZOrigin_forw,
      m_cfg.m_minImpactSeedConf_forw
    };

    // Finder parameters
    m_seedFinderConfig.seedFilter = std::make_unique<Acts::SeedFilter<eicrecon::SpacePoint>>(Acts::SeedFilter<eicrecon::SpacePoint>(m_seedFilterConfig));
    m_seedFinderConfig.rMax = m_cfg.m_rMax;
    m_seedFinderConfig.deltaRMinTopSP = m_cfg.m_deltaRMinTopSP;
    m_seedFinderConfig.deltaRMaxTopSP = m_cfg.m_deltaRMaxTopSP;
    m_seedFinderConfig.deltaRMinBottomSP = m_cfg.m_deltaRMinBottomSP;
    m_seedFinderConfig.deltaRMaxBottomSP = m_cfg.m_deltaRMaxBottomSP;
    m_seedFinderConfig.collisionRegionMin = m_cfg.m_collisionRegionMin;
    m_seedFinderConfig.collisionRegionMax = m_cfg.m_collisionRegionMax;
    m_seedFinderConfig.zMin = m_cfg.m_zMin;
    m_seedFinderConfig.zMax = m_cfg.m_zMax;
    m_seedFinderConfig.maxSeedsPerSpM = m_cfg.m_maxSeedsPerSpM;
    m_seedFinderConfig.cotThetaMax = m_cfg.m_cotThetaMax;
    m_seedFinderConfig.sigmaScattering = m_cfg.m_sigmaScattering;
    m_seedFinderConfig.radLengthPerSeed = m_cfg.m_radLengthPerSeed;
    m_seedFinderConfig.minPt = m_cfg.m_minPt;
    m_seedFinderConfig.bFieldInZ = m_cfg.m_bFieldInZ;
    m_seedFinderConfig.beamPos = Acts::Vector2(m_cfg.m_beamPosX, m_cfg.m_beamPosY);
    m_seedFinderConfig.impactMax = m_cfg.m_impactMax;
    m_seedFinderConfig.rMinMiddle = m_cfg.m_rMinMiddle;
    m_seedFinderConfig.rMaxMiddle = m_cfg.m_rMaxMiddle;

    // Taken from SeedingOrthogonalAlgorithm.cpp, e.g.
    // calculation of scattering using the highland formula
    // convert pT to p once theta angle is known
    m_seedFinderConfig.highland =
      (13.6 * Acts::UnitConstants::MeV) * std::sqrt(m_seedFinderConfig.radLengthPerSeed) *
      (1 + 0.038 * std::log(m_seedFinderConfig.radLengthPerSeed));
    float maxScatteringAngle = m_seedFinderConfig.highland / m_seedFinderConfig.minPt;
    m_seedFinderConfig.maxScatteringAngle2 = maxScatteringAngle * maxScatteringAngle;

    // Helix radius in homogeneous magnetic field
    // in ACTS Units of GeV, mm, and GeV/(e*mm)
    m_seedFinderConfig.pTPerHelixRadius = m_seedFinderConfig.bFieldInZ;

    m_seedFinderConfig.minHelixDiameter2 =
      std::pow(m_seedFinderConfig.minPt * 2 / m_seedFinderConfig.pTPerHelixRadius,2);

    m_seedFinderConfig.pT2perRadius =
      std::pow(m_seedFinderConfig.highland / m_seedFinderConfig.pTPerHelixRadius,2);
}

std::vector<edm4eic::TrackParameters*> eicrecon::TrackSeeding::produce(std::vector<const edm4eic::TrackerHit*> trk_hits) {

  std::vector<const eicrecon::SpacePoint*> spacePoints = getSpacePoints(trk_hits);

  Acts::SeedFinderOrthogonal<eicrecon::SpacePoint> finder(m_seedFinderConfig);
  eicrecon::SeedContainer seeds = finder.createSeeds(spacePoints);

  std::vector<edm4eic::TrackParameters*> result = makeTrackParams(seeds);

  for (auto& sp: spacePoints) {
    delete sp;
  }

  return result;
}

std::vector<const eicrecon::SpacePoint*> eicrecon::TrackSeeding::getSpacePoints(std::vector<const edm4eic::TrackerHit*>& trk_hits)
{
  std::vector<const eicrecon::SpacePoint*> spacepoints;

  for(const auto hit : trk_hits)
    {
      const eicrecon::SpacePoint* sp = new SpacePoint(*hit);
      spacepoints.push_back(sp);
    }

  return spacepoints;
}

std::vector<edm4eic::TrackParameters*> eicrecon::TrackSeeding::makeTrackParams(SeedContainer& seeds)
{
  std::vector<edm4eic::TrackParameters*> trackparams;

  for(auto& seed : seeds)
    {
      std::vector<std::pair<float,float>> xyHitPositions;
      std::vector<std::pair<float,float>> rzHitPositions;
      for(const auto& spptr : seed.sp())
        {
          xyHitPositions.emplace_back(spptr->x(), spptr->y());
          rzHitPositions.emplace_back(spptr->r(), spptr->z());
        }

      auto RX0Y0 = circleFit(xyHitPositions);
      float R = std::get<0>(RX0Y0);
      float X0 = std::get<1>(RX0Y0);
      float Y0 = std::get<2>(RX0Y0);
      if (!(std::isfinite(R) &&
        std::isfinite(std::abs(X0)) &&
        std::isfinite(std::abs(Y0)))) {
        // avoid float overflow for hits on a line
        continue;
      }
      if ( std::hypot(X0,Y0) < std::numeric_limits<decltype(std::hypot(X0,Y0))>::epsilon() ||
        !std::isfinite(std::hypot(X0,Y0)) ) {
        //Avoid center of circle at origin, where there is no point-of-closest approach
        //Also, avoid float overfloat on circle center
        continue;
      }

      auto slopeZ0 = lineFit(rzHitPositions);

      int charge = determineCharge(xyHitPositions);
      float theta = atan(1./std::get<0>(slopeZ0));
      // normalize to 0<theta<pi
      if(theta < 0)
        { theta += M_PI; }
      float eta = -log(tan(theta/2.));
      float pt = R * m_cfg.m_bFieldInZ; // pt[GeV] = R[mm] * B[GeV/mm]
      float p = pt * cosh(eta);
      float qOverP = charge / p;

      const auto xypos = findPCA(RX0Y0);

      //Calculate phi at xypos
      auto xpos = xypos.first;
      auto ypos = xypos.second;

      auto vxpos = -1.*charge*(ypos-Y0);
      auto vypos = charge*(xpos-X0);

      auto phi = atan2(vypos,vxpos);

      const float z0 = seed.z();
      auto perigee = Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3(0,0,0));
      Acts::Vector3 global(xypos.first, xypos.second, z0);

      auto local = perigee->globalToLocal(m_geoSvc->getActsGeometryContext(),
                                          global, Acts::Vector3(1,1,1));

      Acts::Vector2 localpos(sqrt(square(xypos.first) + square(xypos.second)), z0);
      if(local.ok())
        {
          localpos = local.value();
        }

      auto *params = new edm4eic::TrackParameters{
        -1, // type --> seed(-1)
        {(float)localpos(0), (float)localpos(1)}, // 2d location on surface
        {0.1,0.1}, //covariance of location
        theta, //theta [rad]
        (float)phi, // phi [rad]
        qOverP, // Q/p [e/GeV]
        {0.05,0.05,0.05}, // covariance on theta/phi/q/p
        10, // time in ns
        0.1, // error on time
        (float)charge // charge
      };

      trackparams.push_back(params);
    }

  return trackparams;
}
std::pair<float, float> eicrecon::TrackSeeding::findPCA(std::tuple<float,float,float>& circleParams) const
{
  const float R = std::get<0>(circleParams);
  const float X0 = std::get<1>(circleParams);
  const float Y0 = std::get<2>(circleParams);

  const double R0 = std::hypot(X0, Y0);

  //Calculate point on circle closest to origin
  const double xmin = X0 * (1. - R/R0);
  const double ymin = Y0 * (1. - R/R0);

  return std::make_pair(xmin,ymin);
}

int eicrecon::TrackSeeding::determineCharge(std::vector<std::pair<float,float>>& positions) const
{
  // determine the charge by the bend angle of the first two hits
  int charge = 1;
  const auto& firstpos = positions.at(0);
  const auto& secondpos = positions.at(1);

  const auto firstphi = atan2(firstpos.second, firstpos.first);
  const auto secondphi = atan2(secondpos.second, secondpos.first);
  auto dphi = secondphi - firstphi;
  if(dphi > M_PI) dphi = 2.*M_PI - dphi;
  if(dphi < -M_PI) dphi = 2*M_PI + dphi;
  if(dphi < 0) charge = -1;

  return charge;
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
std::tuple<float,float,float> eicrecon::TrackSeeding::circleFit(std::vector<std::pair<float,float>>& positions) const
{
  // Compute x- and y- sample means
  double meanX = 0;
  double meanY = 0;
  double weight = 0;

  for( const auto& [x,y] : positions)
  {
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

  for (auto& [x,y] : positions)
  {
    double Xi = x - meanX;   //  centered x-coordinates
    double Yi = y - meanY;   //  centered y-coordinates
double Zi = std::pow(Xi,2) + std::pow(Yi,2);

    Mxy += Xi*Yi;
    Mxx += Xi*Xi;
    Myy += Yi*Yi;
    Mxz += Xi*Zi;
    Myz += Yi*Zi;
    Mzz += Zi*Zi;
  }
  Mxx /= weight;
  Myy /= weight;
  Mxy /= weight;
  Mxz /= weight;
  Myz /= weight;
  Mzz /= weight;

  //  computing coefficients of the characteristic polynomial

  const double Mz = Mxx + Myy;
  const double Cov_xy = Mxx*Myy - Mxy*Mxy;
  const double Var_z = Mzz - Mz*Mz;
  const double A3 = 4*Mz;
  const double A2 = -3*Mz*Mz - Mzz;
  const double A1 = Var_z*Mz + 4*Cov_xy*Mz - Mxz*Mxz - Myz*Myz;
  const double A0 = Mxz*(Mxz*Myy - Myz*Mxy) + Myz*(Myz*Mxx - Mxz*Mxy) - Var_z*Cov_xy;
  const double A22 = A2 + A2;
  const double A33 = A3 + A3 + A3;

  //    finding the root of the characteristic polynomial
  //    using Newton's method starting at x=0
  //    (it is guaranteed to converge to the right root)
  static constexpr int iter_max = 99;
  double x = 0;
  double y = A0;

  // usually, 4-6 iterations are enough
  for( int iter=0; iter<iter_max; ++iter)
  {
    const double Dy = A1 + x*(A22 + A33*x);
    const double xnew = x - y/Dy;
    if ((xnew == x)||(!std::isfinite(xnew))) break;

    const double ynew = A0 + xnew*(A1 + xnew*(A2 + xnew*A3));
    if (std::abs(ynew)>=std::abs(y))  break;

    x = xnew;  y = ynew;

  }

  //  computing parameters of the fitting circle
  const double DET = std::pow(x,2) - x*Mz + Cov_xy;
  const double Xcenter = (Mxz*(Myy - x) - Myz*Mxy)/DET/2;
  const double Ycenter = (Myz*(Mxx - x) - Mxz*Mxy)/DET/2;

  //  assembling the output
  float X0 = Xcenter + meanX;
  float Y0 = Ycenter + meanY;
  float R = std::sqrt( std::pow(Xcenter,2) + std::pow(Ycenter,2) + Mz);
  return std::make_tuple( R, X0, Y0 );
}

std::tuple<float,float> eicrecon::TrackSeeding::lineFit(std::vector<std::pair<float,float>>& positions) const
{
  double xsum=0;
  double x2sum=0;
  double ysum=0;
  double xysum=0;
  for( const auto& [r,z]:positions )
  {
    xsum=xsum+r;                        //calculate sigma(xi)
    ysum=ysum+z;                        //calculate sigma(yi)
    x2sum=x2sum+std::pow(r,2);              //calculate sigma(x^2i)
    xysum=xysum+r*z;                    //calculate sigma(xi*yi)
  }

  const auto npts = positions.size();
  const double denominator = (x2sum*npts-std::pow(xsum,2));
  const float a= (xysum*npts-xsum*ysum)/denominator;            //calculate slope
  const float b= (x2sum*ysum-xsum*xysum)/denominator;           //calculate intercept
  return std::make_tuple( a, b );
}
