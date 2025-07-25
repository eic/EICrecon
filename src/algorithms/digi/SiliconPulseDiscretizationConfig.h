// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Simon Gardner

#pragma once

#include <edm4eic/unit_system.h>

namespace eicrecon {

struct SiliconPulseDiscretizationConfig {
  double EICROC_period = 25 * edm4eic::unit::ns;
  double local_period  = 25 * edm4eic::unit::ns / 1024; // 1024 TDC bin
  double global_offset = 0; // off-set to convert pulse time to global time
};

} // namespace eicrecon
