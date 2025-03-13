// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Souvik Paul

#pragma once

#include <DD4hep/DD4hepUnits.h>

namespace eicrecon {

struct SiliconPulseGenerationConfig {
  double minimum_separation = 50 * dd4hep::ns; // Minimum digitization time step
};

} // namespace eicrecon
