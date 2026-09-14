// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Aiden Wu

#include <cmath>
#include <random>
#include <stdexcept>

#include "EdepToSiPMConversion.h"

namespace eicrecon {

void EdepToSiPMConversion::init() {
  if (m_cfg.edep_to_npe <= 0) {
    throw std::runtime_error("edep_to_npe must be positive");
  }
  if (m_cfg.num_effective_sipm_pixels == 0) {
    throw std::runtime_error("num_effective_sipm_pixels must be positive");
  }
} // EdepToSiPMConversion:init

void EdepToSiPMConversion::process(const EdepToSiPMConversion::Input& input,
                                   const EdepToSiPMConversion::Output& output) const {
  const auto [headers, inhits] = input;
  auto [outhits]               = output;

  auto seed = m_uid.getUniqueID(*headers, name());
  std::mt19937 generator(seed);

  const auto n_pixels = static_cast<unsigned long>(m_cfg.num_effective_sipm_pixels);
  const double n_pixels_d = static_cast<double>(n_pixels);

  for (const auto& hit : *inhits) {
    const double mean_npe = hit.getEnergy() * m_cfg.edep_to_npe;
    const double p = -std::expm1(-mean_npe / n_pixels_d);
    std::binomial_distribution<unsigned long> binomial(n_pixels, p);
    const unsigned long n_pixels_fired = binomial(generator);

    if (n_pixels_fired == 0) {
      continue;
    }

    auto out_hit = hit.clone();
    out_hit.setEnergy(static_cast<float>(n_pixels_fired));
    outhits->push_back(out_hit);
  }

} // EdepToSiPMConversion:process

} // namespace eicrecon
