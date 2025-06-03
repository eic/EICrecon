// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Simon Gardner

#pragma once

#include <edm4eic/unit_system.h>

namespace eicrecon {

struct SiliconPulseGenerationConfig {
  // Parameters of Silicon signal generation
  std::string pulse_shape_function       = "LandauPulse"; // Pulse shape function
  std::vector<double> pulse_shape_params = {1.0, 0.1};    // Parameters of the pulse shape function
  double ignore_thres                    = 10; // When EDep drops below this value pulse stops
  double timestep          = 0.2 * edm4eic::unit::ns; // Minimum digitization time step
  double min_sampling_time = 0 * edm4eic::unit::ns;   // Minimum sampling time
  uint32_t max_time_bins   = 10000;
};

} // namespace eicrecon
