// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Aiden Wu

#pragma once

#include <string>

#include "algorithms/digi/CALOROCDigitizationConfig.h"

namespace eicrecon {

struct CALOROCToRawCalorimeterHitConfig {
  std::string calorocType{"1B"};
  double calorocResponseToEnergy{1};
  CALOROCDigitizationConfig caloroc{};

  unsigned int capADC{1};
  double dyRangeADC{1};
  unsigned int pedMeanADC{0};
  double resolutionTDC{1};
};

} // namespace eicrecon
