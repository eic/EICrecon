// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Simon Gardner

#pragma once

#include <edm4eic/unit_system.h>

namespace eicrecon {

struct PulseCombinerConfig {
  double minimum_separation =
      50 * edm4eic::unit::ns; // Minimum distance between pulses to keep separate
  std::string readout       = "";
  std::string combine_field = "";
};

} // namespace eicrecon
