// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include "TrackSeeding.h"

#include "Acts/Seeding/InternalSeed.hpp"
#include "Acts/Seeding/SeedFilterConfig.hpp"
#include "Acts/Seeding/SeedFinderOrthogonalConfig.hpp"
#include "Acts/Seeding/SpacePointGrid.hpp"
#include "Acts/Utilities/KDTree.hpp"
#include "Acts/Seeding/Seed.hpp"
#include "Acts/Seeding/SeedFilter.hpp"
#include "Acts/Seeding/SeedFinderOrthogonal.hpp"
#include "Acts/Surfaces/PerigeeSurface.hpp"

#include <TDatabasePDG.h>
#include <tuple>

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
    
    m_cfg.configure();
}

std::vector<edm4eic::TrackParameters*> eicrecon::TrackSeeding::produce(std::vector<const edm4eic::TrackerHit*> trk_hits) {

  eicrecon::SeedContainer seeds = runSeeder(trk_hits);

  std::vector<edm4eic::TrackParameters*> result = makeTrackParams(seeds);

  return result;
}

eicrecon::SeedContainer eicrecon::TrackSeeding::runSeeder(std::vector<const edm4eic::TrackerHit*>& trk_hits)
{
  std::vector<const eicrecon::SpacePoint*> spacePoints = getSpacePoints(trk_hits);

  Acts::SeedFinderOrthogonal<eicrecon::SpacePoint> finder(m_cfg.m_seedFinderConfig);
  eicrecon::SeedContainer seeds = finder.createSeeds(spacePoints);
 
  return seeds;
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
      for(auto& spptr : seed.sp())
	{
	  xyHitPositions.push_back(std::make_pair(spptr->x(), spptr->y()));
	  rzHitPositions.push_back(std::make_pair(spptr->r(), spptr->z()));
	}

      auto RX0Y0 = circleFit(xyHitPositions);
      float R = std::get<0>(RX0Y0);
      float X0 = std::get<1>(RX0Y0);
      float Y0 = std::get<2>(RX0Y0);
      auto slopeZ0 = lineFit(rzHitPositions);
  
      int charge = determineCharge(xyHitPositions);
      float theta = atan(1./std::get<0>(slopeZ0));
      // normalize to 0<theta<pi
      if(theta < 0)
	{ theta += M_PI; }
      float eta = -log(tan(theta/2.));
      float pt = 0.3 * R * (m_cfg.m_bFieldInZ * 1000) / 100.;
      float p = pt * cosh(eta);
      float qOverP = charge / p;
      
      const auto xypos = findRoot(RX0Y0);
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
      
      edm4eic::TrackParameters *params = new edm4eic::TrackParameters{
	-1, // type --> seed(-1)
	{(float)localpos(0), (float)localpos(1)}, // 2d location on surface
	{0.1,0.1}, //covariance of location
	theta, //theta rad
	atan2(xyHitPositions.at(0).second, xyHitPositions.at(0).first), // phi of first hit (rad)
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
std::pair<float, float> eicrecon::TrackSeeding::findRoot(std::tuple<float,float,float>& circleParams) const
{
  const float R = std::get<0>(circleParams);
  const float X0 = std::get<1>(circleParams);
  const float Y0 = std::get<2>(circleParams);
  const double miny = (std::sqrt(square(X0) * square(R) * square(Y0) + square(R) 
		      * pow(Y0,4)) + square(X0) * Y0 + pow(Y0, 3)) 
    / (square(X0) + square(Y0));

  const double miny2 = (-std::sqrt(square(X0) * square(R) * square(Y0) + square(R) 
		      * pow(Y0,4)) + square(X0) * Y0 + pow(Y0, 3)) 
    / (square(X0) + square(Y0));

  const double minx = std::sqrt(square(R) - square(miny - Y0)) + X0;
  const double minx2 = -std::sqrt(square(R) - square(miny2 - Y0)) + X0;
  
  /// Figure out which of the two roots is actually closer to the origin
  const float x = ( std::abs(minx) < std::abs(minx2)) ? minx:minx2;
  const float y = ( std::abs(miny) < std::abs(miny2)) ? miny:miny2;
  return std::make_pair(x,y);
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
