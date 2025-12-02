// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Subhadip Pal

#pragma once

#include "algorithms/particle/CaloRemnantCombiner.h"
#include "extensions/jana/JOmniFactory.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"

namespace eicrecon {

class CaloRemnantCombiner_factory
    : public JOmniFactory<CaloRemnantCombiner_factory, CaloRemnantCombinerConfig> {
private:
  // Underlying algorithm
  std::unique_ptr<eicrecon::CaloRemnantCombiner> m_algo;

  // Declare inputs
  VariadicPodioInput<edm4eic::Cluster> m_in_calo_clusters{this};

  // Declare outputs
  PodioOutput<edm4eic::ReconstructedParticle> m_out_neutral_candidates{this};

  // Declare parameters
  ParameterRef<double> m_deltaRAddEM{this, "deltaRAddEM", config().deltaRAddEM};
  ParameterRef<double> m_deltaRAddH{this, "deltaRAddH", config().deltaRAddH};
  // services
  Service<AlgorithmsInit_service> m_algoInitSvc{this};

public:
  void Configure() {
    m_algo = std::make_unique<CaloRemnantCombiner>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    auto in_clusters = m_in_calo_clusters();
    std::vector<gsl::not_null<const edm4eic::ClusterCollection*>> in_algo;
    std::copy(in_clusters.cbegin(), in_clusters.cend(), std::back_inserter(in_algo));
    m_algo->process({in_algo}, {m_out_neutral_candidates().get()});
  }
};
} // namespace eicrecon
