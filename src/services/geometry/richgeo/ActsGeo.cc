// Copyright (C) 2022, 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#include "ActsGeo.h"

#include <DD4hep/Objects.h>
#include <Evaluator/DD4hepUnits.h>
#include <Math/GenVector/Cartesian3D.h>
#include <Math/GenVector/DisplacementVector3D.h>
#include <edm4hep/Vector3f.h>
#include <fmt/format.h>
#include <algorithm>
#include <cmath>
#include <utility>
#include <variant>

#include "algorithms/tracking/TrackPropagationConfig.h"
#include "services/geometry/richgeo/RichGeo.h"

// constructor
richgeo::ActsGeo::ActsGeo(std::string detName_, gsl::not_null<const dd4hep::Detector*> det_,
                          std::shared_ptr<spdlog::logger> log_)
    : m_detName(std::move(detName_)), m_det(det_), m_log(std::move(log_)) {}

// generate list ACTS disc surfaces, for a given radiator
std::vector<eicrecon::SurfaceConfig> richgeo::ActsGeo::TrackingPlanes(int radiator,
                                                                      int numPlanes) const {

  // output list of surfaces
  std::vector<eicrecon::SurfaceConfig> discs;

  // dRICH DD4hep-ACTS bindings --------------------------------------------------------------------
  if (m_detName == "DRICH") {

    // vessel constants
    auto zmin  = m_det->constant<double>("DRICH_zmin");
    auto zmax  = m_det->constant<double>("DRICH_zmax");
    auto rmin0 = m_det->constant<double>("DRICH_rmin0");
    auto rmin1 = m_det->constant<double>("DRICH_rmin1");
    auto rmax0 = m_det->constant<double>("DRICH_rmax0");
    auto rmax1 = m_det->constant<double>("DRICH_rmax1");
    auto rmax2 = m_det->constant<double>("DRICH_rmax2");

    // radiator constants
    auto snoutLength      = m_det->constant<double>("DRICH_snout_length");
    auto aerogelZpos      = m_det->constant<double>("DRICH_aerogel_zpos");
    auto aerogelThickness = m_det->constant<double>("DRICH_aerogel_thickness");
    auto filterZpos       = m_det->constant<double>("DRICH_filter_zpos");
    auto filterThickness  = m_det->constant<double>("DRICH_filter_thickness");
    auto window_thickness = m_det->constant<double>("DRICH_window_thickness");

    // radial wall slopes
    auto boreSlope  = (rmin1 - rmin0) / (zmax - zmin);
    auto snoutSlope = (rmax1 - rmax0) / snoutLength;

    // get z and radial limits where we will expect charged particles in the RICH
    double trackZmin = NAN;
    double trackZmax = NAN;
    std::function<double(double)> trackRmin;
    std::function<double(double)> trackRmax;
    switch (radiator) {
    case kAerogel:
      trackZmin = aerogelZpos - aerogelThickness / 2;
      trackZmax = aerogelZpos + aerogelThickness / 2;
      trackRmax = [&](auto z) { return rmax0 + snoutSlope * (z - zmin); };
      break;
    case kGas:
      trackZmin = filterZpos + filterThickness / 2;
      trackZmax = zmax - window_thickness;
      trackRmax = [&](auto z) {
        auto z0 = z - zmin;
        if (z0 < snoutLength) {
          return rmax0 + snoutSlope * z0;
        }
        return rmax2;
      };
      break;
    default:
      m_log->error("unknown radiator number {}", numPlanes);
      return discs;
    }
    trackRmin = [&](auto z) { return rmin0 + boreSlope * (z - zmin); };

    // define discs: `numPlanes` z-equidistant planes *within* the radiator;
    /* NOTE: do not allow planes to be at radiator boundary
     * NOTE: alternative binning strategies do not seem to work well with the IRT algorithm;
     *       this one seems to work the best
     *
     * EXAMPLE: numPlanes=4
     *
     *    trackZmin         trackZmax
     *       :                   :
     *       :                   :
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
    double trackZstep = std::abs(trackZmax - trackZmin) / (numPlanes + 1);
    for (int i = 0; i < numPlanes; i++) {
      auto z    = trackZmin + (i + 1) * trackZstep;
      auto rmin = trackRmin(z);
      auto rmax = trackRmax(z);
      discs.emplace_back(eicrecon::DiscSurfaceConfig{
          .id = "ForwardRICH_ID", .zmin = z, .rmin = rmin, .rmax = rmax});
      m_log->debug("  disk {}: z={} r=[ {}, {} ]", i, z, rmin, rmax);
    }
  }

  // pfRICH DD4hep-ACTS bindings --------------------------------------------------------------------
  else if (m_detName == "RICHEndcapN") {
    m_log->error("TODO: pfRICH DD4hep-ACTS bindings have not yet been implemented");
  }

  // ------------------------------------------------------------------------------------------------
  else {
    m_log->error("ActsGeo is not defined for detector '{}'", m_detName);
  }
  return discs;
}

// generate a cut to remove any track points that should not be used
std::function<bool(edm4eic::TrackPoint)> richgeo::ActsGeo::TrackPointCut(int radiator) const {

  // reject track points in dRICH gas that are beyond the dRICH mirrors
  // FIXME: assumes the full mirror spheres are much bigger than the dRICH
  // FIXME: needs to be generalized for dual or multi-mirror (per sector) design
  if (m_detName == "DRICH" && radiator == kGas) {

    // get sphere centers
    std::vector<dd4hep::Position> mirror_centers;
    for (int isec = 0; isec < m_det->constant<int>("DRICH_num_sectors"); isec++) {
      mirror_centers.emplace_back(
          m_det->constant<double>("DRICH_mirror_center_x_sec" + std::to_string(isec)) / dd4hep::mm,
          m_det->constant<double>("DRICH_mirror_center_y_sec" + std::to_string(isec)) / dd4hep::mm,
          m_det->constant<double>("DRICH_mirror_center_z_sec" + std::to_string(isec)) / dd4hep::mm);
    }
    auto mirror_radius = m_det->constant<double>("DRICH_mirror_radius") / dd4hep::mm;

    // beyond the mirror cut
    return [mirror_centers, mirror_radius](edm4eic::TrackPoint p) {
      return std::ranges::any_of(mirror_centers, [&p, &mirror_radius](const auto& c) {
        auto dist = std::hypot(c.x() - p.position.x, c.y() - p.position.y, c.z() - p.position.z);
        return dist < mirror_radius;
      });
    };
  }

  // otherwise return a cut which always passes
  return [](edm4eic::TrackPoint /* p */) { return true; };
}
