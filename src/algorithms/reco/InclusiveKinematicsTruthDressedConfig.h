// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck

#pragma once

namespace eicrecon {

struct InclusiveKinematicsTruthDressedConfig {

  // Dressing cone around the bare scattered lepton, Delta R in (pseudorapidity, azimuth), used
  // when the event record does not identify photons emitted after the hard scattering.
  // Photons collinear with the beam (ISR) are far outside this cone in pseudorapidity.
  double coneDeltaR = 0.1;
};

} // namespace eicrecon
