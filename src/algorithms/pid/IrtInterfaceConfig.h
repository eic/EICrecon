// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, 2025, 2026, Alexander Kiselev

#pragma once

#include <nlohmann/json.hpp>

#include <IRT2/CherenkovDetectorCollection.h>

namespace eicrecon {

// JOmniFactoryGeneratorT does not allow to use more than one extra config parameter ->
// bunch whatever is needed to pass in a single structure; do not want to repeat parsing
// of either the optics file or a JSON configuration file twice;
struct IrtConfig {
  IrtConfig() : m_eta_min(0.0), m_eta_max(0.0) {};

  std::string m_detector_name;
  std::string m_json_config_file_name;

  // FIXME: perhaps do it better later; but in general see no reason in parsing
  // the same fields in a JSON file twice;
  double m_eta_min, m_eta_max;
};

} // namespace eicrecon
