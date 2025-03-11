// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Souvik Paul

#pragma once

#include <DD4hep/DD4hepUnits.h>
#include "PulseShapeFunctors.h"

namespace eicrecon {

struct SiliconPulseGenerationConfig {
  // Parameters of Silicon signal generation
  std::shared_ptr<SignalPulse> pulse_shape_function;
  double ignore_thres   = 10; // When EDep drops below this value pulse stops
  double timestep       = 0.2 * dd4hep::ns; // Minimum digitization time step

};

} // namespace eicrecon
