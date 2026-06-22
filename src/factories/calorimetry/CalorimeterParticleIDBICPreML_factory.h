// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Tomas Sosa, Wouter Deconinck

#pragma once

#include "algorithms/onnx/CalorimeterParticleIDBICPreML.h"
#include "algorithms/onnx/CalorimeterParticleIDBICPreMLConfig.h"
#include "extensions/jana/JOmniFactory.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"

namespace eicrecon {

class CalorimeterParticleIDBICPreML_factory
    : public JOmniFactory<CalorimeterParticleIDBICPreML_factory,
                          CalorimeterParticleIDBICPreMLConfig> {

public:
  using AlgoT = eicrecon::CalorimeterParticleIDBICPreML;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4eic::Cluster> m_imaging_cluster_input{this};
  PodioInput<edm4eic::Cluster> m_scifi_cluster_input{this};
  PodioInput<edm4hep::ParticleID, true> m_pid_input{this};

  PodioOutput<edm4eic::Tensor> m_feature_tensor_output{this};

  ParameterRef<int> m_nLayers{this, "nLayers", config().nLayers};
  ParameterRef<int> m_nHits{this, "nHits", config().nHits};
  ParameterRef<int> m_scifiLayerOffset{this, "scifiLayerOffset", config().scifiLayerOffset};
  ParameterRef<float> m_r0Min{this, "r0Min", config().r0Min};
  ParameterRef<float> m_r0Max{this, "r0Max", config().r0Max};
  ParameterRef<float> m_etaMin{this, "etaMin", config().etaMin};
  ParameterRef<float> m_etaMax{this, "etaMax", config().etaMax};
  ParameterRef<float> m_phiMin{this, "phiMin", config().phiMin};
  ParameterRef<float> m_phiMax{this, "phiMax", config().phiMax};
  ParameterRef<double> m_maxMatchDeltaR{this, "maxMatchDeltaR", config().maxMatchDeltaR};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_imaging_cluster_input(), m_scifi_cluster_input(), m_pid_input()},
                    {m_feature_tensor_output().get()});
  }
};

} // namespace eicrecon
