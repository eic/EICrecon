// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, Nathan Brei, Dmitry Kalinkin

#pragma once

#include <vector>

namespace eicrecon {

struct PIDLookupConfig {
  std::string filename;
  std::string system;
  std::vector<int> pdg_values;
  std::vector<int> charge_values;
  std::vector<double> momentum_edges;
  std::vector<double> polar_edges;
  std::vector<double> azimuthal_binning;
  bool azimuthal_bin_centers_in_lut{false};
  bool momentum_bin_centers_in_lut{false};
  bool polar_bin_centers_in_lut{false};
  bool use_radians{false};
  bool missing_electron_prob{false};
};

} // namespace eicrecon
