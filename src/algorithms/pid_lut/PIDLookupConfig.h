// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, Nathan Brei, Dmitry Kalinkin

#include <vector>

namespace eicrecon {

struct PIDLookupConfig {
  std::string filename;
  int system;
  std::vector<int> pdg_values;
  std::vector<int> charge_values;
  std::vector<double> momentum_binning;
  std::vector<double> polar_binning;
  std::vector<double> azimuthal_binning;
};

} // namespace eicrecon
