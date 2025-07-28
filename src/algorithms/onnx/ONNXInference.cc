// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2024 Wouter Deconinck, Tooba Ali, Dmitry Kalinkin

#include <edm4eic/EDM4eicVersion.h>
#include <sstream>

#if EDM4EIC_VERSION_MAJOR >= 8
#include <fmt/core.h>
#include <onnxruntime_c_api.h>
#include <onnxruntime_cxx_api.h>
#include <algorithm>
#include <cstddef>
#include <gsl/pointers>
#include <iterator>
#include <stdexcept>

#include "ONNXInference.h"

namespace eicrecon {

static std::string print_shape(const std::vector<std::int64_t>& v) {
  std::stringstream ss("");
  for (std::size_t i = 0; i < v.size() - 1; i++) {
    ss << v[i] << " x ";
  }
  ss << v[v.size() - 1];
  return ss.str();
}

static bool check_shape_consistency(const std::vector<std::int64_t>& shape1,
                                    const std::vector<std::int64_t>& shape2) {
  if (shape2.size() != shape1.size()) {
    return false;
  }
  for (std::size_t ix = 0; ix < shape1.size(); ix++) {
    if ((shape1[ix] != -1) && (shape2[ix] != -1) && (shape1[ix] != shape2[ix])) {
      return false;
    }
  }
  return true;
}

template <typename T>
static Ort::Value iters_to_tensor(typename std::vector<T>::const_iterator data_begin,
                                  typename std::vector<T>::const_iterator data_end,
                                  std::vector<int64_t>::const_iterator shape_begin,
                                  std::vector<int64_t>::const_iterator shape_end) {
  Ort::MemoryInfo mem_info = Ort::MemoryInfo::CreateCpu(OrtAllocatorType::OrtArenaAllocator,
                                                        OrtMemType::OrtMemTypeDefault);
  auto tensor =
      Ort::Value::CreateTensor<T>(mem_info, const_cast<T*>(&*data_begin), data_end - data_begin,
                                  &*shape_begin, shape_end - shape_begin);
  return tensor;
}

void ONNXInference::init() {
  // onnxruntime setup
  m_env = Ort::Env(ORT_LOGGING_LEVEL_WARNING, name().data());
  Ort::SessionOptions session_options;
  session_options.SetInterOpNumThreads(1);
  session_options.SetIntraOpNumThreads(1);
  try {
    m_session = Ort::Session(m_env, m_cfg.modelPath.c_str(), session_options);
    Ort::AllocatorWithDefaultOptions allocator;

    // print name/shape of inputs
    debug("Input Node Name/Shape:");
    for (std::size_t i = 0; i < m_session.GetInputCount(); i++) {
      m_input_names.emplace_back(m_session.GetInputNameAllocated(i, allocator).get());
      m_input_shapes.emplace_back(
          m_session.GetInputTypeInfo(i).GetTensorTypeAndShapeInfo().GetShape());
      debug("\t{} : {}", m_input_names.at(i), print_shape(m_input_shapes.at(i)));
    }

    // print name/shape of outputs
    debug("Output Node Name/Shape: {}", m_session.GetOutputCount());
    for (std::size_t i = 0; i < m_session.GetOutputCount(); i++) {
      m_output_names.emplace_back(m_session.GetOutputNameAllocated(i, allocator).get());

      if (m_session.GetOutputTypeInfo(i).GetONNXType() != ONNX_TYPE_TENSOR) {
        m_output_shapes.emplace_back();
        debug("\t{} : not a tensor", m_output_names.at(i));
      } else {
        m_output_shapes.emplace_back(
            m_session.GetOutputTypeInfo(i).GetTensorTypeAndShapeInfo().GetShape());
        debug("\t{} : {}", m_output_names.at(i), print_shape(m_output_shapes.at(i)));
      }
    }

    // convert names to char*
    m_input_names_char.resize(m_input_names.size(), nullptr);
    std::transform(std::begin(m_input_names), std::end(m_input_names),
                   std::begin(m_input_names_char),
                   [&](const std::string& str) { return str.c_str(); });
    m_output_names_char.resize(m_output_names.size(), nullptr);
    std::transform(std::begin(m_output_names), std::end(m_output_names),
                   std::begin(m_output_names_char),
                   [&](const std::string& str) { return str.c_str(); });

  } catch (const Ort::Exception& exception) {
    error("ONNX error {}", exception.what());
    throw;
  }
}

void ONNXInference::process(const ONNXInference::Input& input,
                            const ONNXInference::Output& output) const {

  const auto [in_tensors] = input;
  auto [out_tensors]      = output;

  // Require valid inputs
  if (in_tensors.size() != m_input_names.size()) {
    error("The ONNX model requires {} tensors, whereas {} were provided", m_input_names.size(),
          in_tensors.size());
    throw std::runtime_error(
        fmt::format("The ONNX model requires {} tensors, whereas {} were provided",
                    m_input_names.size(), in_tensors.size()));
  }

  // Prepare input tensor
  std::vector<float> input_tensor_values;
  std::vector<Ort::Value> input_tensors;

  for (std::size_t ix = 0; ix < m_input_names.size(); ix++) {
    edm4eic::Tensor in_tensor = in_tensors[ix]->at(0);
    if (in_tensor.getElementType() == ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT) {
      input_tensors.emplace_back(
          iters_to_tensor<float>(in_tensor.floatData_begin(), in_tensor.floatData_end(),
                                 in_tensor.shape_begin(), in_tensor.shape_end()));
    } else if (in_tensor.getElementType() == ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64) {
      input_tensors.emplace_back(
          iters_to_tensor<int64_t>(in_tensor.int64Data_begin(), in_tensor.int64Data_end(),
                                   in_tensor.shape_begin(), in_tensor.shape_end()));
    }

    auto input_shape = input_tensors[ix].GetTensorTypeAndShapeInfo().GetShape();
    std::vector<std::int64_t> input_expected_shape = m_input_shapes[ix];
    if (!check_shape_consistency(input_shape, input_expected_shape)) {
      error("Input tensor shape incorrect {} != {}", print_shape(input_shape),
            print_shape(input_expected_shape));
      throw std::runtime_error(fmt::format("Input tensor shape incorrect {} != {}",
                                           print_shape(input_shape),
                                           print_shape(input_expected_shape)));
    }
  }

  // Attempt inference
  std::vector<Ort::Value> onnx_values;
  try {
    onnx_values = m_session.Run(Ort::RunOptions{nullptr}, m_input_names_char.data(),
                                input_tensors.data(), m_input_names_char.size(),
                                m_output_names_char.data(), m_output_names_char.size());
  } catch (const Ort::Exception& exception) {
    error("Error running model inference: {}", exception.what());
    throw;
  }

  try {
    for (std::size_t ix = 0; ix < onnx_values.size(); ix++) {
      Ort::Value& onnx_tensor = onnx_values[ix];
      if (!onnx_tensor.IsTensor()) {
        error("The output \"{}\" is not a tensor. ONNXType {} is not yet supported. Skipping...",
              m_output_names_char[ix], static_cast<int>(onnx_tensor.GetTypeInfo().GetONNXType()));
        continue;
      }
      auto onnx_tensor_type             = onnx_tensor.GetTensorTypeAndShapeInfo();
      edm4eic::MutableTensor out_tensor = out_tensors[ix]->create();
      out_tensor.setElementType(static_cast<int32_t>(onnx_tensor_type.GetElementType()));
      std::size_t num_values = 1;
      for (int64_t dim_size : onnx_tensor_type.GetShape()) {
        out_tensor.addToShape(dim_size);
        num_values *= dim_size;
      }
      if (onnx_tensor_type.GetElementType() == ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT) {
        auto* data = onnx_tensor.GetTensorMutableData<float>();
        for (std::size_t value_ix = 0; value_ix < num_values; value_ix++) {
          out_tensor.addToFloatData(data[value_ix]);
        }
      } else if (onnx_tensor_type.GetElementType() == ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64) {
        auto* data = onnx_tensor.GetTensorMutableData<int64_t>();
        for (std::size_t value_ix = 0; value_ix < num_values; value_ix++) {
          out_tensor.addToInt64Data(data[value_ix]);
        }
      } else {
        error("Unsupported ONNXTensorElementDataType {}",
              static_cast<int>(onnx_tensor_type.GetElementType()));
      }
    }
  } catch (const Ort::Exception& exception) {
    error("Error running model inference: {}", exception.what());
    throw;
  }
}

} // namespace eicrecon
#endif
