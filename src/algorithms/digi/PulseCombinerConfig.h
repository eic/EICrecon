// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Souvik Paul

#pragma once

#include <edm4eic/unit_system.h>

namespace eicrecon {

struct PulseCombinerConfig {
  double minimum_separation = 50 * edm4eic::unit::ns; // Minimum digitization time step
  std::string readout = "";
  std::string combine_field = "";
};

} // namespace eicrecon
