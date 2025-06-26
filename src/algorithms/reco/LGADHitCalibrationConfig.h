// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Chun Yuen Tsang

#pragma once

namespace eicrecon {
struct LGADHitCalibrationConfig {
  // parameters that convert ADC to EDep
  double c_slope = 2.8731e-7/2.7704/1.05205, c_intercept = 1.4353e-7;
  // parameters that convert TDC to hit time (ns)
  double t_slope = 0.024414, t_intercept = 0.0122074;
};
} // namespace eicrecon
