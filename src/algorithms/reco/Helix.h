// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Xin Dong, Rongrong Ma

#include <algorithm>
#include <cmath>
#include <exception>
#include <limits>
#include <string>
#include <vector>
#include <utility>
#include <iostream>
#include <edm4hep/Vector3d.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>

#include <edm4eic/unit_system.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/ReconstructedParticleData.h>
#include <edm4eic/TrackParametersCollection.h>

namespace eicrecon {

class Helix {
    bool                   mSingularity;        // true for straight line case (B=0)
    edm4hep::Vector3f      mOrigin;
    double                 mDipAngle;
    double                 mCurvature;
    double                 mPhase;
    int                    mH;                  // -sign(q*B);

    double                 mCosDipAngle;
    double                 mSinDipAngle;
    double                 mCosPhase;
    double                 mSinPhase;

public:
    /// curvature, dip angle, phase, origin, h
    Helix(const double c, const double dip, const double phase, const edm4hep::Vector3f& o, const int h=-1);

    /// momentum, origin, b_field, charge
    Helix(const edm4hep::Vector3f& p, const edm4hep::Vector3f& o, const double B, const int q);    

    /// ReconstructParticle, b field
    Helix(const edm4eic::ReconstructedParticle& p, const double b_field);
    
    ~Helix() = default;
    
    double       dipAngle()   const;           
    double       curvature()  const;	/// 1/R in xy-plane
    double       phase()      const;	/// aziumth in xy-plane measured from ring center
    double       xcenter()    const;	/// x-center of circle in xy-plane
    double       ycenter()    const;	/// y-center of circle in xy-plane
    int          h()          const;	/// -sign(q*B);
    
    const edm4hep::Vector3f& origin() const;	/// starting point

    void setParameters(double c, double dip, double phase, const edm4hep::Vector3f& o, int h);

    void setParameters(const edm4hep::Vector3f& p, const edm4hep::Vector3f& o, const double B, const int q);

    /// edm4eic::TrackParameters, b field
    void setParameters(const edm4eic::TrackParameters& trk, const double b_field);

    /// coordinates of helix at point s
    double       x(double s)  const;
    double       y(double s)  const;
    double       z(double s)  const;

    edm4hep::Vector3f  at(double s) const;

    /// pointing vector of helix at point s
    double       cx(double s)  const;
    double       cy(double s)  const;
    double       cz(double s = 0)  const;
    
    edm4hep::Vector3f  cat(double s) const;

    /// returns period length of helix
    double       period()       const;
    
    /// path length at given r (cylindrical r)
    std::pair<double, double> pathLength(double r)   const;
    
    /// path length at given r (cylindrical r, cylinder axis at x,y)
    std::pair<double, double> pathLength(double r, double x, double y);
    
    /// path length at distance of closest approach to a given point
    double       pathLength(const edm4hep::Vector3f& p, bool scanPeriods = true) const;
    
    /// path length at intersection with plane
    double       pathLength(const edm4hep::Vector3f& r,
			    const edm4hep::Vector3f& n) const;

    /// path length at distance of closest approach in the xy-plane to a given point
    double       pathLength(double x, double y) const;

    /// path lengths at dca between two helices 
    std::pair<double, double> pathLengths(const Helix&,
                                     double minStepSize = 10*edm4eic::unit::um,
                                     double minRange = 10*edm4eic::unit::cm) const;
    
    /// minimal distance between point and helix
    double       distance(const edm4hep::Vector3f& p, bool scanPeriods = true) const;    
    
    /// checks for valid parametrization
    bool         valid(double world = 1.e+5) const {return !bad(world);}
    int            bad(double world = 1.e+5) const;
    
    /// move the origin along the helix to s which becomes then s=0
    virtual void moveOrigin(double s);
    
    static const double NoSolution;
    
    
    void setCurvature(double);	/// performs also various checks   
    void setPhase(double);	        
    void setDipAngle(double);

    /// value of S where distance in x-y plane is minimal
    double fudgePathLength(const edm4hep::Vector3f&) const;

    // Requires:  signed Magnetic Field
    edm4hep::Vector3f momentum(double) const;     // returns the momentum at origin
    edm4hep::Vector3f momentumAt(double, double) const; // returns momentum at S
    int                   charge(double)   const;     // returns charge of particle
    // 2d DCA to x,y point signed relative to curvature
    double curvatureSignedDistance(double x, double y) ;
    // 2d DCA to x,y point signed relative to rotation 
    double geometricSignedDistance(double x, double y) ;
    // 3d DCA to 3d point signed relative to curvature
    double curvatureSignedDistance(const edm4hep::Vector3f&) ;
    // 3d DCA to 3d point signed relative to rotation
    double geometricSignedDistance(const edm4hep::Vector3f&) ;
    
    //
    void Print() const;

}; // end class Helix

//
//     Non-member functions
//
//int operator== (const Helix&, const Helix&);
//int operator!= (const Helix&, const Helix&);
//std::ostream& operator<<(std::ostream&, const Helix&);

//
//     Inline functions
//
inline int Helix::h() const {return mH;}

inline double Helix::dipAngle() const {return mDipAngle;}

inline double Helix::curvature() const {return mCurvature;}

inline double Helix::phase() const {return mPhase;}

inline double Helix::x(double s) const
{
    if (mSingularity)
	return mOrigin.x - s*mCosDipAngle*mSinPhase;
    else
	return mOrigin.x + (cos(mPhase + s*mH*mCurvature*mCosDipAngle)-mCosPhase)/mCurvature;
}
 
inline double Helix::y(double s) const
{
    if (mSingularity)
	return mOrigin.y + s*mCosDipAngle*mCosPhase;
    else
	return mOrigin.y + (sin(mPhase + s*mH*mCurvature*mCosDipAngle)-mSinPhase)/mCurvature;
}

inline double Helix::z(double s) const
{
    return mOrigin.z + s*mSinDipAngle;
}

inline double Helix::cx(double s)  const
{
    if (mSingularity)
	return -mCosDipAngle*mSinPhase;
    else
	return -sin(mPhase + s*mH*mCurvature*mCosDipAngle)*mH*mCosDipAngle;
}

inline double Helix::cy(double s)  const
{
    if (mSingularity)
	return mCosDipAngle*mCosPhase;
    else
	return cos(mPhase + s*mH*mCurvature*mCosDipAngle)*mH*mCosDipAngle;
}

inline double Helix::cz(double /* s */)  const
{
    return mSinDipAngle;
}    

inline const edm4hep::Vector3f& Helix::origin() const {return mOrigin;}

inline edm4hep::Vector3f Helix::at(double s) const
{
    return edm4hep::Vector3f(x(s), y(s), z(s));
}

inline edm4hep::Vector3f Helix::cat(double s) const
{
    return edm4hep::Vector3f(cx(s), cy(s), cz(s));
}

inline double Helix::pathLength(double X, double Y) const
{
    return fudgePathLength(edm4hep::Vector3f(X, Y, 0));
}
inline int Helix::bad(double WorldSize) const
{

//    int ierr;
    if (!::finite(mDipAngle    )) 	return   11;
    if (!::finite(mCurvature   )) 	return   12;

//    ierr = mOrigin.bad(WorldSize);
//    if (ierr)                           return    3+ierr*100;

    if (::fabs(mDipAngle)  >1.58)	return   21;
    double qwe = ::fabs(::fabs(mDipAngle)-M_PI/2);
    if (qwe < 1./WorldSize      ) 	return   31; 

    if (::fabs(mCurvature) > WorldSize)	return   22;
    if (mCurvature < 0          )	return   32;

    if (abs(mH) != 1            )       return   24; 

    return 0;
}

}
