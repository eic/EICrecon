// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Minho Kim

#pragma once

#include <string>
#include <vector>

namespace eicrecon{

	struct CalorimeterHitAttenuationConfig{

		// parameters for attenuation
		std::vector<double>      attPars;
		std::vector<double>      layPars;


		std::string              readout{""};
		std::vector<std::string> fields{};




	};

} // namespace eicrecon
