// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Tomas Sosa, Wouter Deconinck

#pragma once

#include "algorithms/calorimetry/CalorimeterEOverPCut.h"
#include "algorithms/calorimetry/CalorimeterEOverPCutConfig.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class CalorimeterEOverPCut_factory
    : public JOmniFactory<CalorimeterEOverPCut_factory, CalorimeterEOverPCutConfig> {
public:
  using AlgoT = CalorimeterEOverPCut;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4eic::Cluster> m_clusters_input{this};
  PodioInput<edm4eic::TrackClusterMatch> m_assoc_input{this};
  PodioInput<edm4eic::CalorimeterHit> m_hits_input{this};

  PodioOutput<edm4eic::Cluster> m_clusters_output{this};
  PodioOutput<edm4eic::TrackClusterMatch> m_assoc_output{this};
  PodioOutput<edm4hep::ParticleID> m_pid_output{this};

  ParameterRef<double> m_eCutParam{this, "eOverPCut", config().eOverPCut,
                                  "E/p threshold for electron-like tag"};
  ParameterRef<int> m_maxLayerParam{this, "maxLayer", config().maxLayer,
                                  "Max calorimeter layer included in the energy sum"};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
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
