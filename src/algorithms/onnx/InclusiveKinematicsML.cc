// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022, 2023 Wouter Deconinck, Tooba Ali

#include <onnxruntime_c_api.h>
#include <onnxruntime_cxx_api.h>
#include <algorithm>
#include <cstddef>
#include <exception>
#include <gsl/pointers>
#include <iterator>
#include <sstream>

#include "InclusiveKinematicsML.h"

namespace eicrecon {

static std::string print_shape(const std::vector<std::int64_t>& v) {
  std::stringstream ss("");
  for (std::size_t i = 0; i < v.size() - 1; i++) {
    ss << v[i] << "x";
  }
  ss << v[v.size() - 1];
  return ss.str();
}

template <typename T>
Ort::Value vec_to_tensor(std::vector<T>& data, const std::vector<std::int64_t>& shape) {
  Ort::MemoryInfo mem_info = Ort::MemoryInfo::CreateCpu(OrtAllocatorType::OrtArenaAllocator,
                                                        OrtMemType::OrtMemTypeDefault);
  auto tensor =
      Ort::Value::CreateTensor<T>(mem_info, data.data(), data.size(), shape.data(), shape.size());
  return tensor;
}

void InclusiveKinematicsML::init() {
  // onnxruntime setup
  m_env = Ort::Env(ORT_LOGGING_LEVEL_WARNING, "inclusive-kinematics-ml");
  Ort::SessionOptions session_options;
  session_options.SetInterOpNumThreads(1);
  session_options.SetIntraOpNumThreads(1);
  try {
    m_session = Ort::Session(m_env, m_cfg.modelPath.c_str(), session_options);

    // print name/shape of inputs
    Ort::AllocatorWithDefaultOptions allocator;
    debug("Input Node Name/Shape:");
    for (std::size_t i = 0; i < m_session.GetInputCount(); i++) {
      m_input_names.emplace_back(m_session.GetInputNameAllocated(i, allocator).get());
      if (m_session.GetInputTypeInfo(i).GetONNXType() == ONNX_TYPE_TENSOR) {
        m_input_shapes.emplace_back(
            m_session.GetInputTypeInfo(i).GetTensorTypeAndShapeInfo().GetShape());
        debug("\t{} : {}", m_input_names.at(i), print_shape(m_input_shapes.at(i)));
      } else {
        m_input_shapes.emplace_back();
        debug("\t{} : not a tensor", m_input_names.at(i));
      }
    }

    // print name/shape of outputs
    debug("Output Node Name/Shape:");
    for (std::size_t i = 0; i < m_session.GetOutputCount(); i++) {
      m_output_names.emplace_back(m_session.GetOutputNameAllocated(i, allocator).get());
      if (m_session.GetOutputTypeInfo(i).GetONNXType() == ONNX_TYPE_TENSOR) {
        m_output_shapes.emplace_back(
            m_session.GetOutputTypeInfo(i).GetTensorTypeAndShapeInfo().GetShape());
        debug("\t{} : {}", m_output_names.at(i), print_shape(m_output_shapes.at(i)));
      } else {
        m_output_shapes.emplace_back();
        debug("\t{} : not a tensor", m_output_names.at(i));
      }
    }

    // convert names to char*
    m_input_names_char.resize(m_input_names.size(), nullptr);
    std::ranges::transform(m_input_names, std::begin(m_input_names_char),
                           [&](const std::string& str) { return str.c_str(); });
    m_output_names_char.resize(m_output_names.size(), nullptr);
    std::ranges::transform(m_output_names, std::begin(m_output_names_char),
                           [&](const std::string& str) { return str.c_str(); });

  } catch (std::exception& e) {
    error(e.what());
  }
}

void InclusiveKinematicsML::process(const InclusiveKinematicsML::Input& input,
                                    const InclusiveKinematicsML::Output& output) const {

  const auto [electron, da] = input;
  auto [ml]                 = output;

  // Require valid inputs
  if (electron->empty() || da->empty()) {
    debug("skipping because input collections have no entries");
    return;
  }

  // Assume model has 1 input nodes and 1 output node.
  if (m_input_names.size() != 1 || m_output_names.size() != 1) {
    debug("skipping because model has incorrect input and output size");
    return;
  }

  // Prepare input tensor
  std::vector<float> input_tensor_values;
  std::vector<Ort::Value> input_tensors;
  for (auto&& i : *electron) {
    input_tensor_values.push_back(i.getX());
  }
  input_tensors.emplace_back(vec_to_tensor<float>(input_tensor_values, m_input_shapes.front()));

  // Double-check the dimensions of the input tensor
  if (!input_tensors[0].IsTensor() ||
      input_tensors[0].GetTensorTypeAndShapeInfo().GetShape() != m_input_shapes.front()) {
    debug("skipping because input tensor shape incorrect");
    return;
  }

  // Attempt inference
  try {
    auto output_tensors = m_session.Run(Ort::RunOptions{nullptr}, m_input_names_char.data(),
                                        input_tensors.data(), m_input_names_char.size(),
                                        m_output_names_char.data(), m_output_names_char.size());

    // Double-check the dimensions of the output tensors
    if (!output_tensors[0].IsTensor() || output_tensors.size() != m_output_names.size()) {
      debug("skipping because output tensor shape incorrect");
      return;
    }

    // Convert output tensor
    auto* output_tensor_data = output_tensors[0].GetTensorMutableData<float>();
    auto x   = output_tensor_data[0]; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto kin = ml->create();
    kin.setX(x);

  } catch (const Ort::Exception& exception) {
    error("error running model inference: {}", exception.what());
  }
}

} // namespace eicrecon
