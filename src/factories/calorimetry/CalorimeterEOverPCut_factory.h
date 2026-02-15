// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Tomas Sosa, Wouter Deconinck

#pragma once

#include "algorithms/calorimetry/CalorimeterEoverPCut.h"
#include "algorithms/calorimetry/CalorimeterEoverPCutConfig.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class CalorimeterEoverPCut_factory
    : public JOmniFactory<CalorimeterEoverPCut_factory, CalorimeterEoverPCutConfig> {
public:
  using AlgoT = CalorimeterEoverPCut;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4eic::Cluster> m_clusters_input{this};
  PodioInput<edm4eic::TrackClusterMatch> m_assoc_input{this};
  PodioInput<edm4eic::CalorimeterHit> m_hits_input{this};

  PodioOutput<edm4eic::Cluster> m_clusters_output{this};
  PodioOutput<edm4eic::TrackClusterMatch> m_assoc_output{this};
  PodioOutput<edm4hep::ParticleID> m_pid_output{this};

  ParameterRef<double> m_eCutParam{this, "eOverPCut", config().eOverPCut};
  ParameterRef<int> m_maxLayerParam{this, "maxLayer", config().maxLayer};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void ChangeRun(int32_t) {}

  void Process(int32_t, uint64_t) {
    m_algo->process({m_clusters_input(), m_assoc_input(), m_hits_input()},
                    {m_clusters_output().get(), m_assoc_output().get(), m_pid_output().get()});
  }
};

} // namespace eicrecon
