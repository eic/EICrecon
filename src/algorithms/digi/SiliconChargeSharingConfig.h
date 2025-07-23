// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Souvik Paul

#pragma once

namespace eicrecon {

struct SiliconChargeSharingConfig {
  // Parameters of Silicon signal generation
  float sigma_sharingx;
  float sigma_sharingy;
  float min_edep;
  std::string readout;
};

} // namespace eicrecon
