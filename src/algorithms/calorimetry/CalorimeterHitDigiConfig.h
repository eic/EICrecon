// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Wouter Deconinck

#pragma once

#include <string>
#include <vector>

#include <Evaluator/DD4hepUnits.h>
#include <edm4eic/unit_system.h>

namespace eicrecon {

struct CalorimeterHitDigiConfig {

  std::vector<double> eRes;
  double tRes;

  // single hit energy deposition threshold
  double threshold{1.0 * dd4hep::keV};

  // readout settings
  std::string readoutType{"simple"};
  double lightYield{0. / edm4eic::unit::GeV};
  double photonDetectionEfficiency{0.0}; // (light collection efficiency) x (quantum efficiency)
  unsigned long long numEffectiveSipmPixels{0};

  // digitization settings
  unsigned int capADC{1};
  double capTime{1000}; // dynamic range in ns
  double dyRangeADC{1};
  unsigned int pedMeanADC{0};
  double pedSigmaADC{0};
  double resolutionTDC{1};
  std::string corrMeanScale{"1.0"};

  // signal sums
  std::string readout{""};
  std::vector<std::string> fields{};
};

} // namespace eicrecon
