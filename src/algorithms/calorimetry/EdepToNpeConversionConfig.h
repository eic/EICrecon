// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Minho Kim, Aiden Wu

#pragma once

#include <string>
#include <vector>

namespace eicrecon {

struct EdepToNpeConversionConfig {
  std::string readout{};
  std::vector<std::string>
      edep_to_npe_fields{}; // Fields the edep-to-npe conversion factor depends on
  std::string
      edep_to_npe_filename{}; // Lookup table name to get the field-dependent edep-to-npe conversion factors
  double edep_to_npe{};                           // Constant edep-to-npe conversion factor
  unsigned long long num_effective_sipm_pixels{}; // Zero disables SiPM saturation
};

} // namespace eicrecon
