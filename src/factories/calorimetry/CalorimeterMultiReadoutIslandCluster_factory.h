// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Aiden Wu

#pragma once

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <vector>

#include "algorithms/calorimetry/CalorimeterMultiReadoutIslandCluster.h"
#include "extensions/jana/JOmniFactory.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"

namespace eicrecon {

class CalorimeterMultiReadoutIslandCluster_factory
    : public JOmniFactory<CalorimeterMultiReadoutIslandCluster_factory,
                          CalorimeterMultiReadoutIslandClusterConfig> {
public:
  using AlgoT = eicrecon::CalorimeterMultiReadoutIslandCluster;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4eic::CalorimeterHit> m_hits_input{this};

  // Source collections supply runtime IDs in configured readout order.
  VariadicPodioInput<edm4eic::CalorimeterHit> m_source_hits_input{this};
  PodioOutput<edm4eic::ProtoCluster> m_proto_cluster_output{this};

  ParameterRef<double> m_globalDist{this, "globalDist", config().globalDist};

  Service<AlgorithmsInit_service> m_algorithmsInit{this};

public:
  void Configure() {
    if (!m_source_hits_input.collection_names.empty() &&
        m_source_hits_input.collection_names.size() != config().readouts.size()) {
      throw std::runtime_error("Expected one source collection per configured readout");
    }
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    const auto sources = m_source_hits_input();
    if (!sources.empty()) {
      std::vector<std::uint32_t> collectionIDs;
      collectionIDs.reserve(sources.size());
      for (const auto* source : sources) {
        if (source->isSubsetCollection()) {
          throw std::runtime_error("Multi-readout source inputs must be owning collections");
        }
        collectionIDs.push_back(source->getID());
      }
      m_algo->setSourceCollectionIDs(collectionIDs);
    }
    m_algo->process({m_hits_input()}, {m_proto_cluster_output().get()});
  }
};

} // namespace eicrecon
