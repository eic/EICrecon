// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Aiden Wu

#pragma once

#include <edm4eic/unit_system.h>
#include <string>

#include "algorithms/digi/CALOROCDigitizationConfig.h"

namespace eicrecon {

struct CALOROCToRecoCalorimeterHitConfig {
  std::string calorocType{"1A"};
  CALOROCDigitizationConfig caloroc{};
  double responseToEnergy{1 * edm4eic::unit::GeV};
  // Extra ADC counts per ToT code for saturated 1A channels.
  double totToADC{1};
  std::string readout{""};
  std::string layerField{""};
};

} // namespace eicrecon
