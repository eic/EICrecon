// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 W.J. Llope

#pragma once

#include <edm4eic/unit_system.h>

namespace eicrecon {

struct DircTruthIncidenceConfig {
  // Bulk refractive index of the bar quartz used for the "fast enough"
  // (Cherenkov threshold) cut: beta_entry > 1/n + betaMargin.
  double refractiveIndex = 1.4738;
  double betaMargin      = 0.0;

  // Keep only generator-status-1 particles.
  // Default: any charged, fast-enough particle (primary or not).
  bool primaryOnly = false;

  // Drop particles whose production vertex lies inside the crossed bar
  // (|vertex - hit midpoint| < L/2 + bornInsideTolerance). These never
  // "arrive" at the bar from outside.
  bool dropBornInside        = true;
  double bornInsideTolerance = 1.0 * edm4eic::unit::mm;

  // Particles destroyed inside the bar (hadronic interaction; MCParticle
  // endpoint at the hit's exit point within endsInsideTolerance) have
  // post-step momentum 0, so the DD4hep hit momentum is p_entry/2 exactly.
  // If set, use p_entry = 2 * p_hit for such hits (Phase 2 S2p finding).
  bool fixEndsInside         = true;
  double endsInsideTolerance = 1.0 * edm4eic::unit::mm;
};

} // namespace eicrecon
