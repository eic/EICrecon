// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, Dmitry Kalinkin

#pragma once

#include <string>
#include <vector>

#include "algorithms/onnx/CalorimeterParticleIDPreML.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class CalorimeterParticleIDPreML_factory
    : public JOmniFactory<CalorimeterParticleIDPreML_factory, NoConfig> {

public:
  using AlgoT = eicrecon::CalorimeterParticleIDPreML;

  // Override PreInit to inject the missing ParticleID collection name
  void PreInit(const std::string& tag,
               const std::vector<std::string>& input_names,
               const std::vector<std::string>& output_names) override 
  {
    // Copy the provided inputs (should be 2)
    std::vector<std::string> fixed_inputs = input_names;
    // If only 2 were provided, append the default ParticleID collection
    if (fixed_inputs.size() == 2) {
      fixed_inputs.push_back("ParticleID");  // adjust if your PID collection has a different name
    }
    // Call base PreInit with the corrected 3 inputs
    JOmniFactory<CalorimeterParticleIDPreML_factory, NoConfig>::
        PreInit(tag, fixed_inputs, output_names);
  }

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4eic::Cluster>                          m_cluster_input{this};
  PodioInput<edm4eic::MCRecoClusterParticleAssociation> m_cluster_assoc_input{this};
  PodioInput<edm4hep::ParticleID, true>                 m_pid_input{this};

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
    m_algo->process(
      { m_cluster_input(),
        m_cluster_assoc_input(),
        m_pid_input() },
      { m_feature_tensor_output().get(),
        m_target_tensor_output().get() }
    );
  }
};

} // namespace eicrecon
