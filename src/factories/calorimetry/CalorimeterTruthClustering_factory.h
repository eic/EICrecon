// Copyright 2023, Wouter Deconinck
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#ifndef EICRECON_FACTORY_PRECOMPILE
// Preprocessor-based precompilation pattern:
// When EICRECON_FACTORY_PRECOMPILE is not defined, plugin code sees only
// forward declarations and extern templates for fast compilation.
// The full definition is compiled once into a precompile library.

namespace eicrecon {
class CalorimeterTruthClustering_factory;
}

extern template class JOmniFactory<eicrecon::CalorimeterTruthClustering_factory, NoConfig>;

#else
// Full factory definition: compiled into precompile library

#include "algorithms/calorimetry/CalorimeterTruthClustering.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class CalorimeterTruthClustering_factory
    : public JOmniFactory<CalorimeterTruthClustering_factory, NoConfig> {
public:
  using AlgoT = eicrecon::CalorimeterTruthClustering;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4eic::CalorimeterHit> m_rc_hits_input{this};
  PodioInput<edm4eic::MCRecoCalorimeterHitLink> m_hit_link_input{this};
  PodioOutput<edm4eic::ProtoCluster> m_proto_clusters_output{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_rc_hits_input(), m_hit_link_input()}, {m_proto_clusters_output().get()});
  }
};

} // namespace eicrecon

#endif // EICRECON_FACTORY_PRECOMPILE
