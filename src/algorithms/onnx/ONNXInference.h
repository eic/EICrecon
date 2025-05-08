// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2024 Sylvester Joosten, Dmitry Romanov, Wouter Deconinck, Dmitry Kalinkin

#pragma once

#include <algorithms/algorithm.h>
#include <cstdint>
#include <onnxruntime_cxx_api.h>
#include <string>
#include <string_view>
#include <vector>
#include <edm4eic/TensorCollection.h>

#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/onnx/ONNXInferenceConfig.h"

namespace eicrecon {

using ONNXInferenceAlgorithm =
    algorithms::Algorithm<algorithms::Input<std::vector<edm4eic::TensorCollection>>,
                          algorithms::Output<std::vector<edm4eic::TensorCollection>>>;

class ONNXInference : public ONNXInferenceAlgorithm, public WithPodConfig<ONNXInferenceConfig> {

public:
  ONNXInference(std::string_view name)
      : ONNXInferenceAlgorithm{name, {"inputTensors"}, {"outputTensors"}, ""} {}

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  mutable Ort::Env m_env{nullptr};
  mutable Ort::Session m_session{nullptr};

  std::vector<std::string> m_input_names;
  std::vector<const char*> m_input_names_char;
  std::vector<std::vector<std::int64_t>> m_input_shapes;

  std::vector<std::string> m_output_names;
  std::vector<const char*> m_output_names_char;
  std::vector<std::vector<std::int64_t>> m_output_shapes;
};

} // namespace eicrecon
