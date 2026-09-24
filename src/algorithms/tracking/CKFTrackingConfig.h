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

  // --- Per-branch stopping for the CKF ---
  // A branch whose fit has diverged keeps inflating its covariance until it
  // overflows double precision and produces a NaN chi2. ACTS's default branch
  // stopper never stops, so nothing contains this. A branch is dropped once
  // the filtered variance of q/p exceeds this limit, or is no longer positive.
  // The default sits midway between the variance a seed asserts (0.1) and what
  // a diverged fit reaches (1e10 and up); it corresponds to a sigma(q/p) that
  // is already many times |q/p| for any track worth keeping.
  // See the implementation in CKFTracking.cc.
  double maxQOverPVariance = 1e5;
};
} // namespace eicrecon
