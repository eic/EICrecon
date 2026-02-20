// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Esteban Molina, Derek Anderson

#pragma once

namespace eicrecon {
struct ParticleConverterConfig {
  ///! Momentum resolution of tracker in this eta region
  double tracking_resolution = 1;

  ///! Energy resolution of combined EMCal + HCal system
  double caloResolution = 1;

  ///! Controls relative contribution of HCal energy to total
  double caloHadronScale = 1;

  ///! Normalizes total EMCal + HCal energy
  double caloEnergyNorm = 1;

  ///! Turn on/off taking resolution-weighted average of track
  ///! and calo energy
  bool useResolutionInEnergyCalc = false;
};
} // namespace eicrecon
