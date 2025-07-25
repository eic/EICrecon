// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Wouter Deconinck

#pragma once

#include <string>

namespace eicrecon {

struct InclusiveKinematicsMLConfig {

  std::string modelPath{"calibrations/onnx/identity_gemm_w1x1_b1.onnx"};
};

} // namespace eicrecon
