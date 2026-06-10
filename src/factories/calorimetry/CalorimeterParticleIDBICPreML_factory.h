// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Tomas Sosa, Wouter Deconinck

#pragma once

#include "algorithms/onnx/CalorimeterParticleIDBICPreML.h"
#include "extensions/jana/JOmniFactory.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"

namespace eicrecon {

class CalorimeterParticleIDBICPreML_factory
    : public JOmniFactory<CalorimeterParticleIDBICPreML_factory, NoConfig> {

public:
  using AlgoT = eicrecon::CalorimeterParticleIDBICPreML;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4eic::Cluster> m_cluster_input{this};
  PodioInput<edm4hep::ParticleID, true> m_pid_input{this};

  PodioOutput<edm4eic::Tensor> m_feature_tensor_output{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_cluster_input(), m_pid_input()},
                    {m_feature_tensor_output().get()});
  }
};

} // namespace eicrecon