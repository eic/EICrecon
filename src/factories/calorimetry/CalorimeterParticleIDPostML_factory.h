// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, Dmitry Kalinkin

#pragma once

#include "algorithms/onnx/CalorimeterParticleIDPostML.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class CalorimeterParticleIDPostML_factory
    : public JOmniFactory<CalorimeterParticleIDPostML_factory, NoConfig> {

public:
  using AlgoT = eicrecon::CalorimeterParticleIDPostML;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4eic::Cluster> m_cluster_input{this};
  PodioInput<edm4eic::MCRecoClusterParticleAssociation> m_cluster_assoc_input{this};
  PodioInput<edm4eic::Tensor> m_prediction_tensor_input{this};

  PodioOutput<edm4eic::Cluster> m_cluster_output{this};
  PodioOutput<edm4eic::MCRecoClusterParticleAssociation> m_cluster_assoc_output{this};
  PodioOutput<edm4hep::ParticleID> m_particle_id_output{this};

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
        {m_cluster_input(), m_cluster_assoc_input(), m_prediction_tensor_input()},
        {m_cluster_output().get(), m_cluster_assoc_output().get(), m_particle_id_output().get()});
  }
};

} // namespace eicrecon
