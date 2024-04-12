// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Wouter Deconinck

#pragma once

#include <string>

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
  std::vector<std::variant<CylinderSurfaceConfig, DiscSurfaceConfig>> surfaces;
};

} // namespace eicrecon
