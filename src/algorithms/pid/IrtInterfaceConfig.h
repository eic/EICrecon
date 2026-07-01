//
// Copyright 2025, Alexander Kiselev
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <nlohmann/json.hpp>

#include <IRT2/CherenkovDetectorCollection.h>

namespace eicrecon {

// JOmniFactoryGeneratorT does not allow to use more than one extra config parameter ->
// bunch whatever is needed to pass in a single structure; do not want to repeat parsing
// of either the optics file or a JSON configuration file twice;
struct IrtConfig {
  IrtConfig() : m_irt_geometry(0), m_irt_detector(0), m_eta_min(0.0), m_eta_max(0.0) {};

  IRT2::CherenkovDetectorCollection* m_irt_geometry;
  IRT2::CherenkovDetector* m_irt_detector;
  nlohmann::json m_json_config;

  // FIXME: perhaps do it better later; but in general see no reason in parsing
  // the same fields in a JSON file twice;
  double m_eta_min, m_eta_max;
};

} // namespace eicrecon
