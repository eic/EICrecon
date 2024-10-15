// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Chun Yuen Tsang

#pragma once

namespace eicrecon {
    struct BTOFHitReconstructionConfig {
        // parameters that convert ADC to EDep
	double c_slope = 3.86976e-7, c_intercept=2.42716e-5;
	// parameters that convert TDC to hit time (ns)
	double t_slope = 0.0197305, t_intercept = 0.208047;
	// if false, we use weighted ADC averages for position.
	// By default, the position is just the center of cell with max ADC value in a sensor
	bool use_ave = true;
    };
}
