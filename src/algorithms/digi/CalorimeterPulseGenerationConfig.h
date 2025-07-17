// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 - 2025 Simon Gardner, Chun Yuen Tsang, Prithwish Tribedy, Minho Kim

#pragma once

#include <string>
#include <vector>
#include <edm4eic/unit_system.h>

namespace eicrecon {

struct CalorimeterPulseGenerationConfig {

  std::string pulse_shape{""};
  std::vector<double> pulse_shape_params;

  // conversion factor from energy deposit to number of photoelectrons (Npe)
  double edep_to_npe{};

  // parameters for building pulse
  double timestep{0.2 * edm4eic::unit::ns};
  double min_sampling_time{0.0 * edm4eic::unit::ns};
  // maximal expected length of an afterpulse
  uint32_t max_time_bins{10000};
  double ignore_thres{10};
  double minimum_separation{50 * edm4eic::unit::ns};
};

} // namespace eicrecon
