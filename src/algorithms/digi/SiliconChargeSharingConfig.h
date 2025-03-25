// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Souvik Paul

#pragma once

namespace eicrecon {

struct SiliconChargeSharingConfig {
  // Parameters of Silicon signal generation
  double sigma_sharingx;
  double sigma_sharingy;
  double m_minEDep;
  std::string readout;
};

} // namespace eicrecon
