// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Aiden Wu

#pragma once

namespace eicrecon {

struct EdepToSiPMConversionConfig {
  double edep_to_npe{};
  unsigned long long num_effective_sipm_pixels{};
};

} // namespace eicrecon
