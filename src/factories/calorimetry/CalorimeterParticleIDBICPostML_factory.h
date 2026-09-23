// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Tomas Sosa, Wouter Deconinck

#pragma once

#include "algorithms/onnx/CalorimeterParticleIDBICPostML.h"
#include "extensions/jana/JOmniFactory.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"

namespace eicrecon {

class CalorimeterParticleIDBICPostML_factory
    : public JOmniFactory<CalorimeterParticleIDBICPostML_factory, NoConfig> {

public:
  using AlgoT = eicrecon::CalorimeterParticleIDBICPostML;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4eic::Cluster> m_standard_cluster_input{this};
  PodioInput<edm4eic::MCRecoClusterParticleAssociation> m_standard_cluster_assoc_input{this};
  PodioInput<edm4eic::Cluster> m_bic_cluster_input{this};
  PodioInput<edm4eic::Cluster> m_imaging_cluster_input{this};
  PodioInput<edm4eic::Cluster> m_standard_scifi_cluster_input{this};
  PodioInput<edm4eic::Cluster> m_selected_scifi_cluster_input{this};
  // ONNX produces no collection when an event has no BIC candidate.  This is
  // optional so that PostML can still emit the complete standard-cluster copy.
  PodioInput<edm4eic::Tensor, true> m_prediction_tensor_input{this};

  PodioOutput<edm4eic::Cluster> m_cluster_output{this};
  PodioOutput<edm4eic::MCRecoClusterParticleLink> m_cluster_link_output{this};
  PodioOutput<edm4eic::MCRecoClusterParticleAssociation> m_cluster_assoc_output{this};
  PodioOutput<edm4hep::ParticleID> m_particle_id_output{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_standard_cluster_input(), m_standard_cluster_assoc_input(),
                     m_bic_cluster_input(), m_imaging_cluster_input(),
                     m_standard_scifi_cluster_input(), m_selected_scifi_cluster_input(),
                     m_prediction_tensor_input()},
                    {m_cluster_output().get(), m_cluster_link_output().get(),
                     m_cluster_assoc_output().get(), m_particle_id_output().get()});
  }
};

} // namespace eicrecon
