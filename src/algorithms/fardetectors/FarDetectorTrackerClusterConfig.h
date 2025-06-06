// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 - 2024, Simon Gardner

#pragma once

#include <edm4eic/unit_system.h>

namespace eicrecon {
struct FarDetectorTrackerClusterConfig {

  // Readout identifiers for dividing detector
  std::string readout{""};
  std::string x_field{"x"};
  std::string y_field{"y"};

  // Timing limit to add a hit to a cluster
  double hit_time_limit{10 * edm4eic::unit::ns};
};
} // namespace eicrecon
