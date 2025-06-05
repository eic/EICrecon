// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Minho Kim

#pragma once

#include <string>
#include <vector>
#include <edm4eic/unit_system.h>

namespace eicrecon{

        struct CalorimeterPulseGenerationConfig{

                std::string              readout{""};

		std::string		 pulse_shape{""};
		std::vector<double>      pulse_shape_params;
		
		double                   timestep{0.2 * edm4eic::unit::ns};
		double                   min_sampling_time{0.0 * edm4eic::unit::ns};
		uint32_t                 max_time_bin{10000};
		double                   ignore_thres{10};
		
	};

} // namespace eicrecon
