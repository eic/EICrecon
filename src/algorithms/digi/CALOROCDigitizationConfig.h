// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Minho Kim

#pragma once

#include <edm4eic/unit_system.h>
#include <cstddef>
#include <cstdint>
#include <map>
#include <string>

#include "algorithms/calorimetry/CalorimeterCALOROCReadoutConfig.h"

namespace eicrecon {

struct CALOROCDigitizationConfig {

  // Variables for CALOROC measurement
  std::size_t n_samples{7};
  double adc_phase{0 * edm4eic::unit::ns};
  double toa_thres{1.0};
  double tot_thres{1};
  // Baseline amplitude for samples beyond the stored waveform.
  double baseline{0};

  CalorimeterCALOROCReadoutConfig caloroc{};

  // Optional ToA thresholds by cell-ID field value, in waveform amplitude units.
  std::string readout{};
  std::string calibrationField{};
  std::map<std::int64_t, double> toa_thres_overrides{};
};

} // namespace eicrecon
