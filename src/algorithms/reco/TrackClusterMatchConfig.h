// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Tristan Protzman

#pragma once

#include <string>
#include <DD4hep/DD4hepUnits.h>

namespace eicrecon {
struct TrackClusterMatchConfig {
  double matching_distance = 0.5;
  std::string calo_id      = "";
};
} // namespace eicrecon
