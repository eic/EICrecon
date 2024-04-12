// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022, 2023 Wouter Deconinck, Tooba Ali

#include <assert.h>
#include <fmt/core.h>
#include <onnxruntime_c_api.h>
#include <onnxruntime_cxx_api.h>
#include <algorithm>
#include <cstddef>
#include <exception>
#include <gsl/pointers>
#include <iterator>
#include <ostream>

#include "InclusiveKinematicsML.h"

namespace eicrecon {

  static std::string print_shape(const std::vector<std::int64_t>& v) {
    std::stringstream ss("");
    for (std::size_t i = 0; i < v.size() - 1; i++) ss << v[i] << "x";
    ss << v[v.size() - 1];
    return ss.str();
  }

  template <typename T>
  Ort::Value vec_to_tensor(std::vector<T>& data, const std::vector<std::int64_t>& shape) {
    Ort::MemoryInfo mem_info =
        Ort::MemoryInfo::CreateCpu(OrtAllocatorType::OrtArenaAllocator, OrtMemType::OrtMemTypeDefault);
    auto tensor = Ort::Value::CreateTensor<T>(mem_info, data.data(), data.size(), shape.data(), shape.size());
    return tensor;
  }

  void InclusiveKinematicsML::init(std::shared_ptr<spdlog::logger>& logger) {
    m_log = logger;

    // onnxruntime setup
    Ort::Env env(ORT_LOGGING_LEVEL_VERBOSE, "inclusive-kinematics-ml");
    Ort::SessionOptions session_options;
    try {
      m_session = Ort::Session(env, m_cfg.modelPath.c_str(), session_options);

      // print name/shape of inputs
      Ort::AllocatorWithDefaultOptions allocator;
      m_log->debug("Input Node Name/Shape:");
      for (std::size_t i = 0; i < m_session.GetInputCount(); i++) {
        m_input_names.emplace_back(m_session.GetInputNameAllocated(i, allocator).get());
        m_input_shapes.emplace_back(m_session.GetInputTypeInfo(i).GetTensorTypeAndShapeInfo().GetShape());
        m_log->debug("\t{} : {}", m_input_names.at(i), print_shape(m_input_shapes.at(i)));
      }

      // print name/shape of outputs
      m_log->debug("Output Node Name/Shape:");
      for (std::size_t i = 0; i < m_session.GetOutputCount(); i++) {
        m_output_names.emplace_back(m_session.GetOutputNameAllocated(i, allocator).get());
        m_output_shapes.emplace_back(m_session.GetOutputTypeInfo(i).GetTensorTypeAndShapeInfo().GetShape());
        m_log->debug("\t{} : {}", m_output_names.at(i), print_shape(m_output_shapes.at(i)));
      }

      // convert names to char*
      m_input_names_char.resize(m_input_names.size(), nullptr);
      std::transform(std::begin(m_input_names), std::end(m_input_names), std::begin(m_input_names_char),
                     [&](const std::string& str) { return str.c_str(); });
      m_output_names_char.resize(m_output_names.size(), nullptr);
      std::transform(std::begin(m_output_names), std::end(m_output_names), std::begin(m_output_names_char),
                     [&](const std::string& str) { return str.c_str(); });

    } catch(std::exception& e) {
      m_log->error(e.what());
    }
  }

  void InclusiveKinematicsML::process(
      const InclusiveKinematicsML::Input& input,
      const InclusiveKinematicsML::Output& output) const {

    const auto [electron, da] = input;
    auto [ml] = output;

    // Require valid inputs
    if (electron->size() == 0 || da->size() == 0) return;

    // Assume model has 1 input nodes and 1 output node.
    assert(m_input_names.size() == 1 && m_output_names.size() == 1);

    // Prepare input tensor
    std::vector<float> input_tensor_values;
    std::vector<Ort::Value> input_tensors;
    for (std::size_t i = 0; i < electron->size(); i++) {
      input_tensor_values.push_back(electron->at(i).getX());
    }
    input_tensors.emplace_back(vec_to_tensor<float>(input_tensor_values, m_input_shapes.front()));

    // Double-check the dimensions of the input tensor
    assert(input_tensors[0].IsTensor() && input_tensors[0].GetTensorTypeAndShapeInfo().GetShape() == m_input_shapes.front());

    // Attempt inference
    try {
      auto output_tensors = m_session.Run(Ort::RunOptions{nullptr}, m_input_names_char.data(), input_tensors.data(),
                                          m_input_names_char.size(), m_output_names_char.data(), m_output_names_char.size());

      // Double-check the dimensions of the output tensors
      assert(output_tensors.size() == m_output_names.size() && output_tensors[0].IsTensor());

      // Convert output tensor
      float* output_tensor_data = output_tensors[0].GetTensorMutableData<float>();
      auto x  = output_tensor_data[0];
      auto kin = ml->create();
      kin.setX(x);

    } catch (const Ort::Exception& exception) {
      m_log->error("error running model inference: {}", exception.what());
    }
  }

} // namespace eicrecon
