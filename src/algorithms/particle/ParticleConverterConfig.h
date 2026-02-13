// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Esteban Molina, Derek Anderson

#pragma once

namespace eicrecon {
struct ParticleConverterConfig {
  double tracking_resolution = 1;

  double ecal_resolution  = 1;
  double hcal_resolution  = 1;
  double calo_hadron_scale = 1;
  double calo_energy_norm = 1;

  bool use_resolution_in_ecalc = false;
};
} // namespace eicrecon
