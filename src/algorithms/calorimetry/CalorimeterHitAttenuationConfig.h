// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Minho Kim

#pragma once

#include <string>
#include <vector>

namespace eicrecon{

	struct CalorimeterHitAttenuationConfig{

		// parameters for attenuation function
		std::vector<double>      attPars;

		// fields for adding up energies and attenuate them
		std::string              readout{""};
		std::string              lengthField{""};
		std::string              zField{""};
	};

} // namespace eicrecon
