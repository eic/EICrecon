// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Chun Yuen Tsang

#pragma once

namespace eicrecon {
struct LGADHitCalibrationConfig {
  // parameters that convert ADC to EDep
  double c_slope = 1.175844e-6, c_intercept = 0;
  // parameters that convert TDC to hit time (ns)
  double t_slope = 0.024319882, t_intercept = 0.04314;
};
} // namespace eicrecon
