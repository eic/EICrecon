// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Wouter Deconinck

#pragma once

#include <string>
#include <edm4eic/TrackPoint.h>

namespace eicrecon {

  struct CylinderSurfaceConfig {
    std::string id;
    std::variant<std::string, double> rmin;
    std::variant<std::string, double> zmin;
    std::variant<std::string, double> zmax;
  };

  struct DiscSurfaceConfig {
    std::string id;
    std::variant<std::string, double> zmin;
    std::variant<std::string, double> rmin;
    std::variant<std::string, double> rmax;
  };

  struct TrackPropagationConfig {
    std::vector<std::variant<CylinderSurfaceConfig,DiscSurfaceConfig>> filter_surfaces{};
    std::vector<std::variant<CylinderSurfaceConfig,DiscSurfaceConfig>> target_surfaces{};

    std::function<bool(edm4eic::TrackPoint)> track_point_cut{
      [](const edm4eic::TrackPoint&) { return true; }
    };
    bool skip_track_on_track_point_cut_failure{false};
  };

} // eicrecon
