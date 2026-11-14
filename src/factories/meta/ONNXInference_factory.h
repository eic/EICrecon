// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 - 2024, Wouter Deconinck, Simon Gardener, Dmitry Kalinkin

#pragma once

#include "algorithms/onnx/ONNXInference.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

class ONNXInference_factory
    : public JOmniAlgorithmFactory<ONNXInference_factory, eicrecon::ONNXInference,
                                   ONNXInferenceConfig> {
private:
  ParameterRef<std::string> m_modelPath{this, "modelPath", config().modelPath};
};

} // namespace eicrecon
