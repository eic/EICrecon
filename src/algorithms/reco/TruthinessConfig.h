// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Wouter Deconinck

#pragma once

namespace eicrecon {

struct TruthinessConfig {
  // Penalty weights for different contributions
  double pidPenaltyWeight              = 1.0; // Weight for PID mismatch penalty (default: 1)
  double unassociatedMCPenaltyWeight   = 1.0; // Weight for unassociated MC particles (default: 1)
  double unassociatedRecoPenaltyWeight = 1.0; // Weight for unassociated reco particles (default: 1)

  // Default resolutions when covariance matrix is unavailable or zero (in GeV)
  double defaultEnergyResolution = 1.0; // Default energy uncertainty (default: 1.0 GeV)
  double defaultMomentumResolution =
      1.0; // Default momentum component uncertainty (default: 1.0 GeV)
};

} // namespace eicrecon
