// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Tomas Sosa

#pragma once

#include "algorithms/calorimetry/CalorimeterEoverPCut.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

/// Factory that uses only the algorithm’s own defaults (ecut=0.74, max_layer=12).
class CalorimeterEoverPCut_factory
    : public JOmniFactory<CalorimeterEoverPCut_factory, NoConfig> {
public:
  using AlgoT = CalorimeterEoverPCut;

private:
  std::unique_ptr<AlgoT> m_algo;

  // Exactly one‑argument PodioInput/Output as in PreML
  PodioInput<edm4eic::Cluster>                          m_clusters_input{this};
  PodioInput<edm4eic::MCRecoClusterParticleAssociation> m_assoc_input   {this};
  PodioOutput<edm4hep::ParticleID>                      m_pid_output    {this};

public:
  void Configure() {
    // Just instantiate and init—algorithm’s constructor already set the defaults
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->init();
  }

  void ChangeRun(int32_t /*run_number*/) {}

  void Process(int32_t /*run_number*/, uint64_t /*event_number*/) {
    m_algo->process({ m_clusters_input(), m_assoc_input() },
                    { m_pid_output() });
  }
};

} // namespace eicrecon
