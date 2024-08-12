// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include "TrackSeeding.h"

#include <Acts/Definitions/Algebra.hpp>
#include <Acts/Definitions/Units.hpp>
#include <Acts/Seeding/Seed.hpp>
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
#include <boost/container/small_vector.hpp>
#include <boost/container/vector.hpp>
#include <edm4eic/Cov6f.h>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <cmath>
#include <functional>
#include <limits>
#include <tuple>
#include <type_traits>

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
      m_cfg.zMinSeedConfCentral,
      m_cfg.zMaxSeedConfCentral,
      m_cfg.rMaxSeedConfCentral,
      m_cfg.nTopForLargeRCentral,
      m_cfg.nTopForSmallRCentral,
      m_cfg.seedConfMinBottomRadiusCentral,
      m_cfg.seedConfMaxZOriginCentral,
      m_cfg.minImpactSeedConfCentral
    };

    m_seedFilterConfig.forwardSeedConfirmationRange = Acts::SeedConfirmationRangeConfig{
      m_cfg.zMinSeedConfForward,
      m_cfg.zMaxSeedConfForward,
      m_cfg.rMaxSeedConfForward,
      m_cfg.nTopForLargeRForward,
      m_cfg.nTopForSmallRForward,
      m_cfg.seedConfMinBottomRadiusForward,
      m_cfg.seedConfMaxZOriginForward,
      m_cfg.minImpactSeedConfForward
    };

    m_seedFilterConfig = m_seedFilterConfig.toInternalUnits();

    // Finder parameters
    m_seedFinderConfig.seedFilter = std::make_unique<Acts::SeedFilter<eicrecon::SpacePoint>>(Acts::SeedFilter<eicrecon::SpacePoint>(m_seedFilterConfig));
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

    m_seedFinderOptions.beamPos   = Acts::Vector2(m_cfg.beamPosX, m_cfg.beamPosY);
    m_seedFinderOptions.bFieldInZ = m_cfg.bFieldInZ;

    m_seedFinderConfig =
      m_seedFinderConfig.toInternalUnits().calculateDerivedQuantities();
    m_seedFinderOptions =
      m_seedFinderOptions.toInternalUnits().calculateDerivedQuantities(m_seedFinderConfig);

}

std::unique_ptr<edm4eic::TrackParametersCollection> eicrecon::TrackSeeding::produce(const edm4eic::TrackerHitCollection& trk_hits) {

  std::vector<const eicrecon::SpacePoint*> spacePoints = getSpacePoints(trk_hits);

  Acts::SeedFinderOrthogonal<eicrecon::SpacePoint> finder(m_seedFinderConfig); // FIXME move into class scope

#if Acts_VERSION_MAJOR >= 32
  std::function<std::tuple<Acts::Vector3, Acts::Vector2, std::optional<Acts::ActsScalar>>(
      const eicrecon::SpacePoint *sp)>
      create_coordinates = [](const eicrecon::SpacePoint *sp) {
        Acts::Vector3 position(sp->x(), sp->y(), sp->z());
        Acts::Vector2 variance(sp->varianceR(), sp->varianceZ());
        return std::make_tuple(position, variance, sp->t());
      };
#else
  std::function<std::pair<Acts::Vector3, Acts::Vector2>(
      const eicrecon::SpacePoint *sp)>
      create_coordinates = [](const eicrecon::SpacePoint *sp) {
        Acts::Vector3 position(sp->x(), sp->y(), sp->z());
        Acts::Vector2 variance(sp->varianceR(), sp->varianceZ());
        return std::make_pair(position, variance);
      };
#endif

  eicrecon::SeedContainer seeds = finder.createSeeds(m_seedFinderOptions, spacePoints, create_coordinates);

  std::unique_ptr<edm4eic::TrackParametersCollection> trackparams = makeTrackParams(seeds);

  for (auto& sp: spacePoints) {
    delete sp;
  }

  return std::move(trackparams);
}

std::vector<const eicrecon::SpacePoint*> eicrecon::TrackSeeding::getSpacePoints(const edm4eic::TrackerHitCollection& trk_hits)
{
  std::vector<const eicrecon::SpacePoint*> spacepoints;

  for(const auto hit : trk_hits)
    {
      const eicrecon::SpacePoint* sp = new SpacePoint(hit);
      spacepoints.push_back(sp);
    }

  return spacepoints;
}

std::unique_ptr<edm4eic::TrackParametersCollection> eicrecon::TrackSeeding::makeTrackParams(SeedContainer& seeds)
{
  auto trackparams = std::make_unique<edm4eic::TrackParametersCollection>();

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
      const auto xypos = findPCA(RX0Y0);


      //Determine charge
      int charge = determineCharge(xyHitPositions, xypos, RX0Y0);

      float theta = atan(1./std::get<0>(slopeZ0));
      // normalize to 0<theta<pi
      if(theta < 0)
        { theta += M_PI; }
      float eta = -log(tan(theta/2.));
      float pt = R * m_cfg.bFieldInZ; // pt[GeV] = R[mm] * B[GeV/mm]
      float p = pt * cosh(eta);
      float qOverP = charge / p;

      //Calculate phi at xypos
      auto xpos = xypos.first;
      auto ypos = xypos.second;

      auto vxpos = -1.*charge*(ypos-Y0);
      auto vypos = charge*(xpos-X0);

      auto phi = atan2(vypos,vxpos);

      const float z0 = seed.z();
      auto perigee = Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3(0,0,0));
      Acts::Vector3 global(xypos.first, xypos.second, z0);

      //Compute local position at PCA
      Acts::Vector2 localpos;
      Acts::Vector3 direction(sin(theta)*cos(phi), sin(theta)*sin(phi), cos(theta));

      auto local = perigee->globalToLocal(m_geoSvc->getActsGeometryContext(),
                                          global,
                                          direction);

      if(!local.ok())
      {
        continue;
      }

      localpos = local.value();

      auto trackparam = trackparams->create();
      trackparam.setType(-1); // type --> seed(-1)
      trackparam.setLoc({static_cast<float>(localpos(0)), static_cast<float>(localpos(1))}); // 2d location on surface
      trackparam.setPhi(static_cast<float>(phi)); // phi [rad]
      trackparam.setTheta(theta); //theta [rad]
      trackparam.setQOverP(qOverP); // Q/p [e/GeV]
      trackparam.setTime(10); // time in ns
      edm4eic::Cov6f cov;
      cov(0,0) = m_cfg.locaError / Acts::UnitConstants::mm; // loc0
      cov(1,1) = m_cfg.locbError / Acts::UnitConstants::mm; // loc1
      cov(2,2) = m_cfg.phiError / Acts::UnitConstants::rad; // phi
      cov(3,3) = m_cfg.thetaError / Acts::UnitConstants::rad; // theta
      cov(4,4) = m_cfg.qOverPError * Acts::UnitConstants::GeV; // qOverP
      cov(5,5) = m_cfg.timeError / Acts::UnitConstants::ns; // time
      trackparam.setCovariance(cov);
    }

  return std::move(trackparams);
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

int eicrecon::TrackSeeding::determineCharge(std::vector<std::pair<float,float>>& positions, const std::pair<float,float>& PCA, std::tuple<float,float,float>& RX0Y0) const
{

  const auto& firstpos = positions.at(0);
  auto hit_x = firstpos.first;
  auto hit_y = firstpos.second;

  auto xpos = PCA.first;
  auto ypos = PCA.second;

  float X0 = std::get<1>(RX0Y0);
  float Y0 = std::get<2>(RX0Y0);

  Acts::Vector3 B_z(0,0,1);
  Acts::Vector3 radial(X0-xpos, Y0-ypos, 0);
  Acts::Vector3 hit(hit_x-xpos, hit_y-ypos, 0);

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
