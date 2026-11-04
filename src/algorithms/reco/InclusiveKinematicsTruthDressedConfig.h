// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck

#pragma once

#include <string>

namespace eicrecon {

struct InclusiveKinematicsTruthDressedConfig {

  // How real photons radiated from the lepton line are added to the scattered lepton:
  //  - "auto": by parentage, when the event record has photons emitted after the hard
  //    scattering (e.g. DJANGOH 4.6.21 or Pythia8 with QED showers); by cone otherwise
  //    (e.g. DJANGOH 4.6.10, which attaches all photons to the beam lepton);
  //  - "parentage": only photons emitted after the hard scattering;
  //  - "cone": only photons inside the dressing cone.
  std::string photonAssignment = "auto";

  // Dressing cone around the bare scattered lepton, Delta R in (pseudorapidity, azimuth).
  // Photons collinear with the beam (ISR) are far outside this cone in pseudorapidity.
  double coneDeltaR = 0.1;
};

} // namespace eicrecon
