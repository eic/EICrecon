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

  // --- Per-branch stopping (loop / quality protection) for the CKF ---
  // The combinatorial track finder can spawn a branch that curls back through
  // a sensitive surface it has already used, re-fitting the same hit(s)
  // repeatedly. This is a positive-feedback loop: the Kalman gain becomes
  // ill-conditioned, q/p and the covariance diverge without bound, and the
  // covariance eventually overflows double precision, producing a NaN chi2 on
  // the next surface. ACTS's default branch stopper never stops, so nothing
  // contains this. See the connected implementation in CKFTracking.cc.

  // Stop (and drop) a branch that adds a measurement on a sensitive surface it
  // already has a measurement on (a loop). Enabled by default; set false to
  // restore the previous, unprotected behavior.
  bool stopOnLoop = true;
};
} // namespace eicrecon
