// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 - 2025, Simon Gardner

#pragma once

namespace eicrecon {
struct FarDetectorTransportationPreMLConfig {

  float beamE = 10.0;
  bool requireBeamElectron{true};
  bool beamE_set_from_metadata{false};
};
} // namespace eicrecon
