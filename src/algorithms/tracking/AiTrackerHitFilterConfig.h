// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026, Romanov D.

#pragma once

#include <string>

namespace eicrecon {

struct AiTrackerHitFilterConfig {

  /// Window-finding mode:
  ///  "finder" - 2D-image CNN (finder2d_v1 ONNX) estimates t0 from the hits
  ///  "ideal"  - MC-truth t0 (earliest signal hit, albedo excluded) via hit associations
  ///  "off"    - passthrough: every hit goes to the signal output, no ONNX is run
  std::string windowMode{"finder"};

  /// ONNX model paths. finder model requires its external-data sibling
  /// (!) YES, THIS IS BAD. VERY BAD. YOU HAVE NEVER SEEN IT! (it is temporary for PR draft while no weights exists on a server)
  std::string finderModelPath{"/eic-workbench/ai-trkfilter-models/window_finder2d.onnx"};
  std::string wattModelPath{"/eic-workbench/ai-trkfilter-models/window_model.onnx"};
  std::string mlpModelPath{"/eic-workbench/ai-trkfilter-models/mlp_model.onnx"};

  /// Background-probability thresholds, keep hit iff score < threshold.

  float thresholdInWindow{0.99F};
  float thresholdOutWindow{0.81F};

  /// Window extent around t0 estimate [ns]: inside iff -front <= t-t0 <= back
  float windowFrontNs{50.0F};
  float windowBackNs{300.0F};
};

} // namespace eicrecon
