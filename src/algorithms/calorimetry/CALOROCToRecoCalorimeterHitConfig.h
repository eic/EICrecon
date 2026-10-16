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
  double adcToEnergy{1 * edm4eic::unit::GeV};

  // Visible energy = offset + slope * ToT + quadratic * ToT squared.
  double totEnergyOffset{0 * edm4eic::unit::GeV};
  double totEnergySlope{1 * edm4eic::unit::GeV / edm4eic::unit::ns};
  double totEnergyQuadratic{0 * edm4eic::unit::GeV / edm4eic::unit::ns / edm4eic::unit::ns};
  // Pedestal per sample in ADC counts.
  double pedestalADC{0};
  std::string readout{""};
  std::string layerField{""};
};

} // namespace eicrecon
