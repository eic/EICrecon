// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 W.J. Llope

#pragma once

#include <edm4eic/unit_system.h>

namespace eicrecon {

struct DircGRdigConfig {
  // Dead time of a pixel, measured from the start of a hit: the earliest
  // photon in a pixel makes a hit; photons arriving less than deadTime after
  // that hit's (smeared) time are merged into it (they only increase its
  // photon count); the first photon at or after t_hit + deadTime starts a new
  // hit. 0 (default) = every photon is its own hit.
  // Expected FCFD performance: ~25 ns recovery time (deadTime = 25.0).
  double deadTime = 0.0 * edm4eic::unit::ns;
  // TDC least significant bit: timeStamp = floor(t / timeResolution).
  // Default: FCFD. Consumers (DircGR timeResolution) must use the same value.
  double timeResolution = 0.020 * edm4eic::unit::ns;
  // Gaussian time jitter per photon (HRPPD single-pixel TTS + electronics,
  // ~0.018); 0 = off. Applied before the dead-time logic, so the earliest
  // MEASURED photon wins.
  double timeJitter = 0.0 * edm4eic::unit::ns;
};

} // namespace eicrecon
