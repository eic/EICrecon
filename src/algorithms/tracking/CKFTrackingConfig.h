// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <vector>

namespace eicrecon {
struct CKFTrackingConfig {
  std::vector<double> etaBins                    = {};
  std::vector<double> chi2CutOff                 = {15.};
  std::vector<std::size_t> numMeasurementsCutOff = {10};

  std::size_t numMeasurementsMin = 4;

  /// Include the measurement time (in addition to loc0 and loc1) in the calibrated measurements
  bool useTime = false;
};
} // namespace eicrecon
