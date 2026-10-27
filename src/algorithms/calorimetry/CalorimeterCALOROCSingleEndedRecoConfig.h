// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Aiden Wu

#pragma once

#include <edm4eic/unit_system.h>
#include <string>
#include <vector>

#include "algorithms/digi/CALOROCDigitizationConfig.h"

namespace eicrecon {

struct CalorimeterCALOROCSingleEndedRecoConfig {
  std::string calorocType{"1A"};
  CALOROCDigitizationConfig caloroc{};
  double adcToEnergy{1 * edm4eic::unit::GeV};

  // Visible energy = p0 + p1 * ToT + p2 * ToT squared.
  std::vector<double> totEnergyParameters{0 * edm4eic::unit::GeV,
                                          1 * edm4eic::unit::GeV / edm4eic::unit::ns,
                                          0 * edm4eic::unit::GeV / edm4eic::unit::ns / edm4eic::unit::ns};
  // Pedestal per sample in single-gain ADC counts.
  double pedestalADC{0};

  // Time walk: p0 + p1 * pow(Q - p2, p3), with Q the pedestal-subtracted ADC sum.
  // p0 and p1 are in ns; Q and p2 are numeric single-gain ADC counts.
  bool timeWalkCor{false};
  std::vector<double> timeWalkCorrectionParameters{0, 0, 0, 1};

  std::string sampFrac{"1.0"};
  std::string readout{""};
  std::string layerField{""};
};

} // namespace eicrecon
