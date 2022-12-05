// Copyright 2022, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include "ActsGeo.h"

std::vector<std::shared_ptr<Acts::DiscSurface>> rich::ActsGeo::TrackingPlanes(int radiator, int numPlanes) {

  // output list of surfaces
  std::vector<std::shared_ptr<Acts::DiscSurface>> discs;

  // dRICH DD4hep-ACTS bindings --------------------------------------------------------------------
  if(m_detName=="DRICH") {

    // vessel constants
    auto zmin  = m_det->constant<double>("DRICH_zmin");
    auto zmax  = m_det->constant<double>("DRICH_zmax");
    auto rmin0 = m_det->constant<double>("DRICH_rmin0");
    auto rmin1 = m_det->constant<double>("DRICH_rmin1");
    auto rmax0 = m_det->constant<double>("DRICH_rmax0");
    auto rmax1 = m_det->constant<double>("DRICH_rmax1");
    auto rmax2 = m_det->constant<double>("DRICH_rmax2");

    // aerogel constants
    auto snoutLength      = m_det->constant<double>("DRICH_snout_length");
    auto aerogelZpos      = m_det->constant<double>("DRICH_aerogel_zpos");
    auto aerogelThickness = m_det->constant<double>("DRICH_aerogel_thickness");

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
        trackRmax = [&] (auto z) { return rmax0 + snoutLength * (z - zmin); };
        break;
      case kGas:
        trackZmin = aerogelZpos + aerogelThickness/2;
        trackZmax = zmax;
        trackRmax = [&] (auto z) {
          auto z0 = z - zmin;
          if(z0 < snoutLength) return rmax0 + snoutLength * z0;
          else return rmax2;
        };
        break;
      default:
        m_log.PrintError("unknown radiator number {}",numPlanes);
        return discs;
    }
    trackRmin = [&] (auto z) { return rmin0 + boreSlope * (z - zmin); };

    // define discs
    m_log.PrintLog("Define ACTS disks for radiator {}: {} disks in z=[ {}, {} ]",
        RadiatorName(radiator), numPlanes, trackZmin, trackZmax);
    double trackZstep = std::abs(trackZmax-trackZmin) / (numPlanes-1);
    for(int i=0; i<numPlanes; i++) {
      auto z         = trackZmin + i*trackZstep;
      auto rmin      = trackRmin(z);
      auto rmax      = trackRmax(z);
      auto rbounds   = std::make_shared<Acts::RadialBounds>(rmin, rmax);
      auto transform = Acts::Transform3(Acts::Translation3(Acts::Vector3(0, 0, z)));
      discs.push_back(Acts::Surface::makeShared<Acts::DiscSurface>(transform, rbounds));
      m_log.PrintLog("  disk {}: z={} r=[ {}, {} ]", i, z, rmin, rmax);
    }
  }

  // pfRICH DD4hep-ACTS bindings --------------------------------------------------------------------
  else if(m_detName=="DRICH") {
    m_log.PrintError("TODO: pfRICH DD4hep-ACTS bindings have not yet been implemented");
  }

  // ------------------------------------------------------------------------------------------------
  else m_log.PrintError("ActsGeo is not defined for detector '{}'",m_detName);
  return discs;
}
