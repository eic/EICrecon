// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Minho Kim

#pragma once

#include <edm4eic/unit_system.h>

namespace eicrecon {

	struct CALOROCDigitizationConfig {

		// Variables for CALOROC measurement
		std::size_t n_samples{7};
		double time_window{25 * edm4eic::unit::ns};
		double adc_phase{0 * edm4eic::unit::ns};
		double toa_thres{1};
		double tot_thres{1};

		// Variables for digitization
		unsigned int capADC{1024};
		// ADC dynamic range for 1A chip
		double dyRangeSingleGainADC{1};
		// ADC dynamic ranges for 1B chip
		double dyRangeHighGainADC{1};
		double dyRangeLowGainADC{1};
		unsigned int capTOA{1024};
                double dyRangeTOA{25 * edm4eic::unit::ns};
                unsigned int capTOT{4096};
                double dyRangeTOT{200 * edm4eic::unit::ns};

	};

} // namespace eicrecon
