// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, Dmitry Kalinkin

#pragma once

#include "algorithms/onnx/CalorimeterParticleIDPreML.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class CalorimeterParticleIDPreML_factory
    : public JOmniFactory<CalorimeterParticleIDPreML_factory, NoConfig> {

  using Base = JOmniFactory<CalorimeterParticleIDPreML_factory, NoConfig>;

public:
  using AlgoT = eicrecon::CalorimeterParticleIDPreML;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4eic::Cluster> m_cluster_input{this};
  PodioInput<edm4eic::MCRecoClusterParticleAssociation> m_cluster_assoc_input{this};
  PodioInput<edm4hep::ParticleID> m_pid_input{this};

  PodioOutput<edm4eic::Tensor> m_feature_tensor_output{this};
  PodioOutput<edm4eic::Tensor> m_target_tensor_output{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void ChangeRun(int32_t /* run_number */) {}

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_cluster_input(), m_cluster_assoc_input(), m_pid_input()},
                    {m_feature_tensor_output().get(), m_target_tensor_output().get()});
  }

  void PreInit(std::string plugin_name, std::vector<std::string> input_names,
               std::vector<std::string> output_names) {
    // pad with empty strings so JOmniFactory::PreInit
    // never complains about “3 expected, 2 found”
    while (input_names.size() < 3) {
      input_names.emplace_back("");
    }
    Base::PreInit(std::move(plugin_name), std::move(input_names), std::move(output_names));
  }
};

} // namespace eicrecon
