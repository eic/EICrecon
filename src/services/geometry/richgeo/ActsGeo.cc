// Copyright (C) 2022, 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#include "ActsGeo.h"

#include <Acts/Definitions/Algebra.hpp>
#include <Acts/Surfaces/DiscSurface.hpp>
#include <Acts/Surfaces/RadialBounds.hpp>
// ACTS
#include <Acts/Surfaces/Surface.hpp>
#include <DD4hep/Objects.h>
#include <Evaluator/DD4hepUnits.h>
#include <Math/GenVector/Cartesian3D.h>
#include <Math/GenVector/DisplacementVector3D.h>
#include <ctype.h>
#include <edm4hep/Vector3f.h>
#include <fmt/core.h>
#include <algorithm>
#include <cmath>

#include "services/geometry/richgeo/RichGeo.h"

// constructor
richgeo::ActsGeo::ActsGeo(std::string detName_, gsl::not_null<const dd4hep::Detector*> det_, std::shared_ptr<spdlog::logger> log_)
  : m_detName(detName_), m_det(det_), m_log(log_)
{
  // capitalize m_detName
  std::transform(m_detName.begin(), m_detName.end(), m_detName.begin(), ::toupper);
}

// generate list ACTS disc surfaces, for a given radiator
std::vector<std::shared_ptr<Acts::Surface>> richgeo::ActsGeo::TrackingPlanes(int radiator, int numPlanes) {

  // output list of surfaces
  std::vector<std::shared_ptr<Acts::Surface>> discs;

  // dRICH DD4hep-ACTS bindings --------------------------------------------------------------------
  if(m_detName=="DRICH") {

    // vessel constants
    auto zmin  = m_det->constant<double>("DRICH_zmin")  / dd4hep::mm;
    auto zmax  = m_det->constant<double>("DRICH_zmax")  / dd4hep::mm;
    auto rmin0 = m_det->constant<double>("DRICH_rmin0") / dd4hep::mm;
    auto rmin1 = m_det->constant<double>("DRICH_rmin1") / dd4hep::mm;
    auto rmax0 = m_det->constant<double>("DRICH_rmax0") / dd4hep::mm;
    auto rmax1 = m_det->constant<double>("DRICH_rmax1") / dd4hep::mm;
    auto rmax2 = m_det->constant<double>("DRICH_rmax2") / dd4hep::mm;

    // radiator constants
    auto snoutLength      = m_det->constant<double>("DRICH_snout_length")      / dd4hep::mm;
    auto aerogelZpos      = m_det->constant<double>("DRICH_aerogel_zpos")      / dd4hep::mm;
    auto aerogelThickness = m_det->constant<double>("DRICH_aerogel_thickness") / dd4hep::mm;
    auto filterZpos       = m_det->constant<double>("DRICH_filter_zpos")       / dd4hep::mm;
    auto filterThickness  = m_det->constant<double>("DRICH_filter_thickness")  / dd4hep::mm;
    auto window_thickness = m_det->constant<double>("DRICH_window_thickness")  / dd4hep::mm;

    // radial wall slopes
    auto boreSlope  = (rmin1 - rmin0) / (zmax - zmin);
    auto snoutSlope = (rmax1 - rmax0) / snoutLength;

    // get z and radial limits where we will expect charged particles in the RICH
    double trackZmin, trackZmax;
    std::function<double(double)> trackRmin, trackRmax;
    switch(radiator) {
      case kAerogel:
        trackZmin = aerogelZpos - aerogelThickness/2;
        trackZmax = aerogelZpos + aerogelThickness/2;
        trackRmax = [&] (auto z) { return rmax0 + snoutSlope * (z - zmin); };
        break;
      case kGas:
        trackZmin = filterZpos + filterThickness/2;
        trackZmax = zmax - window_thickness;
        trackRmax = [&] (auto z) {
          auto z0 = z - zmin;
          if(z0 < snoutLength) return rmax0 + snoutSlope * z0;
          else return rmax2;
        };
        break;
      default:
        m_log->error("unknown radiator number {}",numPlanes);
        return discs;
    }
    trackRmin = [&] (auto z) { return rmin0 + boreSlope * (z - zmin); };

    // define discs: `numPlanes` z-equidistant planes *within* the radiator;
    /* NOTE: do not allow planes to be at radiator boundary
     * NOTE: alternative binning strategies do not seem to work well with the IRT algorithm;
     *       this one seems to work the best
     *
     * EXAMPLE: numPlanes=4
     *
     *    trackZmin         trackZmax
     *       :                 :
     *       :                 :
     *       +===================+....trackRmax
     *       [   |   |   |   |   ]
     *       [   |   |   |   |   ]
     *       [   <--planes--->   ]
     *       [   0   1   2   3   ]
     *       [   |   |   |   |   ]
     *       [   |   |   |   |   ]
     *       +===================+....trackRmin
     *       :   :       :   :
     *     ->:   :<-     :   :
     *     trackZstep    :   :
     *                 ->:   :<-
     *                 trackZStep
     */
    m_log->debug("Define ACTS disks for {} radiator: {} disks in z=[ {}, {} ]",
        RadiatorName(radiator), numPlanes, trackZmin, trackZmax);
    double trackZstep = std::abs(trackZmax-trackZmin) / (numPlanes+1);
    for(int i=0; i<numPlanes; i++) {
      auto z         = trackZmin + (i+1)*trackZstep;
      auto rmin      = trackRmin(z);
      auto rmax      = trackRmax(z);
      auto rbounds   = std::make_shared<Acts::RadialBounds>(rmin, rmax);
      auto transform = Acts::Transform3(Acts::Translation3(Acts::Vector3(0, 0, z)));
      discs.push_back(Acts::Surface::makeShared<Acts::DiscSurface>(transform, rbounds));
      m_log->debug("  disk {}: z={} r=[ {}, {} ]", i, z, rmin, rmax);
    }
  }

  // pfRICH DD4hep-ACTS bindings --------------------------------------------------------------------
  else if(m_detName=="PFRICH") {
    m_log->error("TODO: pfRICH DD4hep-ACTS bindings have not yet been implemented");
  }

  // ------------------------------------------------------------------------------------------------
  else m_log->error("ActsGeo is not defined for detector '{}'",m_detName);
  return discs;
}

// generate a cut to remove any track points that should not be used
std::function<bool(edm4eic::TrackPoint)> richgeo::ActsGeo::TrackPointCut(int radiator) {

  // reject track points in dRICH gas that are beyond the dRICH mirrors
  // FIXME: assumes the full mirror spheres are much bigger than the dRICH
  // FIXME: needs to be generalized for dual or multi-mirror (per sector) design
  if(m_detName=="DRICH" && radiator==kGas) {

    // get sphere centers
    std::vector<dd4hep::Position> mirror_centers;
    for(int isec = 0; isec < m_det->constant<int>("DRICH_num_sectors"); isec++)
      mirror_centers.emplace_back(
          m_det->constant<double>("DRICH_mirror_center_x_sec" + std::to_string(isec)) / dd4hep::mm,
          m_det->constant<double>("DRICH_mirror_center_y_sec" + std::to_string(isec)) / dd4hep::mm,
          m_det->constant<double>("DRICH_mirror_center_z_sec" + std::to_string(isec)) / dd4hep::mm
          );
    auto mirror_radius = m_det->constant<double>("DRICH_mirror_radius") / dd4hep::mm;

    // beyond the mirror cut
    return [mirror_centers, mirror_radius] (edm4eic::TrackPoint p) {
      for(const auto& c : mirror_centers) {
        auto dist = std::hypot(
            c.x() - p.position.x,
            c.y() - p.position.y,
            c.z() - p.position.z
            );
        if(dist < mirror_radius)
          return true;
      }
      return false;
    };
  }

  // otherwise return a cut which always passes
  return [] (edm4eic::TrackPoint p) { return true; };

}
