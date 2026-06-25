// Copyright 2023, Wouter Deconinck
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

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
  PodioInput<edm4eic::MCRecoCalorimeterHitAssociation> m_hit_assoc_input{this};
  PodioOutput<edm4eic::ProtoCluster> m_proto_clusters_output{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_rc_hits_input(), m_hit_assoc_input()}, {m_proto_clusters_output().get()});
  }
};

} // namespace eicrecon
