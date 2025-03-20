// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Simon Gardner

#pragma once

#include <edm4eic/unit_system.h>
#include "PulseShapeFunctors.h"

namespace eicrecon {

struct SiliconPulseGenerationConfig {
  // Parameters of Silicon signal generation
  std::shared_ptr<SignalPulse> pulse_shape_function;
  double ignore_thres    = 10; // When EDep drops below this value pulse stops
  double timestep        = 0.2 * edm4eic::unit::ns; // Minimum digitization time step
  int    max_time_bins = 10000;

};

} // namespace eicrecon
