// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 - 2025, Simon Gardner

#pragma once

namespace eicrecon {
struct FarDetectorTransportationPostMLConfig {

  float beamE = 10.0;
  bool requireBeamElectron{true};
  int pdg_value = 11; // Default to electron

};
} // namespace eicrecon
