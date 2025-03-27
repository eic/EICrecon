// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Souvik Paul

#pragma once

#include <edm4eic/unit_system.h>
#include "PulseShapeFunctors.h"

namespace eicrecon {

struct SiliconPulseGenerationConfig {
  // Parameters of Silicon signal generation
  std::shared_ptr<SignalPulse> pulse_shape_function;
  float ignore_thres      = 10; // When EDep drops below this value pulse stops
  float timestep          = 0.2 * edm4eic::unit::ns; // Minimum digitization time step
  float min_sampling_time = 0 * edm4eic::unit::ns; // Minimum sampling time
  int   max_time_bins     = 10000;


};

} // namespace eicrecon
