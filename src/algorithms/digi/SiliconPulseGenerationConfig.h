// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Souvik Paul

#pragma once

#include <DD4hep/DD4hepUnits.h>

namespace eicrecon {

struct SiliconPulseGenerationConfig {
  // Parameters of Silicon signal generation
  std::function<double(const double)> pulse_shape_function;
  double risetime       = 1.5 * dd4hep::ns;
  double ignore_thres   = 0.001 * Vm; // When EDep drops below this value pulse stops
  double timestep       = 0.2 * dd4hep::ns; // Minimum digitization time step

};

} // namespace eicrecon
