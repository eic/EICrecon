// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 - 2024, Wouter Deconinck, Simon Gardener, Dmitry Kalinkin

#pragma once

#include "algorithms/onnx/ONNXInference.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class ONNXInference_factory : public JOmniFactory<ONNXInference_factory, ONNXInferenceConfig> {

public:
  using AlgoT = eicrecon::ONNXInference;

private:
  std::unique_ptr<AlgoT> m_algo;

  VariadicPodioInput<edm4eic::Tensor> m_input_tensors{this};

  VariadicPodioOutput<edm4eic::Tensor> m_output_tensors{this};

  ParameterRef<std::string> m_modelPath{this, "modelPath", config().modelPath};

  Service<AlgorithmsInit_service> m_algorithmsInit{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void ChangeRun(int32_t /* run_number */) {}

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    std::vector<gsl::not_null<const edm4eic::TensorCollection*>> in_collections;
    for (const auto& in_collection : m_input_tensors()) {
      in_collections.push_back(gsl::not_null<const edm4eic::TensorCollection*>{in_collection});
    }

    std::vector<gsl::not_null<edm4eic::TensorCollection*>> out_collections;
    for (const auto& out_collection : m_output_tensors()) {
      out_collections.push_back(gsl::not_null<edm4eic::TensorCollection*>{out_collection.get()});
    }

    m_algo->process(in_collections, out_collections);
  }
};

} // namespace eicrecon
