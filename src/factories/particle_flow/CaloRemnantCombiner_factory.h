// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Subhadip Pal

#pragma once

#include "algorithms/particle_flow/CaloRemnantCombiner.h"
#include "extensions/jana/JOmniFactory.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"

namespace eicrecon {

class CaloRemnantCombiner_factory
    : public JOmniFactory<CaloRemnantCombiner_factory, CaloRemnantCombinerConfig> {

public:
  using AlgoT = eicrecon::CaloRemnantCombiner;

private:
  // Underlying algorithm
  std::unique_ptr<AlgoT> m_algo;

  // Declare inputs
  PodioInput<edm4eic::Cluster> m_in_ecal_clusters{this};
  PodioInput<edm4eic::Cluster> m_in_hcal_clusters{this};

  // Declare outputs
  PodioOutput<edm4eic::ReconstructedParticle> m_out_neutral_candidates{this};

  // Declare parameters
  ParameterRef<double> m_ecalDeltaR{this, "ecalDeltaR", config().ecalDeltaR};
  ParameterRef<double> m_hcalDeltaR{this, "hcalDeltaR", config().hcalDeltaR};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_in_ecal_clusters(), m_in_hcal_clusters()},
                    {m_out_neutral_candidates().get()});
  }
};
} // namespace eicrecon
