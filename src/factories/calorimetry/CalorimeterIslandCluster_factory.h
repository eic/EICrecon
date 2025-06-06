// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Wouter Deconinck

#pragma once

#include "algorithms/calorimetry/CalorimeterIslandCluster.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class CalorimeterIslandCluster_factory
    : public JOmniFactory<CalorimeterIslandCluster_factory, CalorimeterIslandClusterConfig> {
public:
  using AlgoT = eicrecon::CalorimeterIslandCluster;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4eic::CalorimeterHit> m_calo_hit_input{this};
  PodioOutput<edm4eic::ProtoCluster> m_proto_cluster_output{this};

  ParameterRef<double> m_sectorDist{this, "sectorDist", config().sectorDist};
  // ParameterRef<std::vector<double>> m_localDistXY{this, "localDistXY", config().localDistXY};
  ParameterRef<std::vector<double>> m_localDistXZ{this, "localDistXZ", config().localDistXZ};
  ParameterRef<std::vector<double>> m_localDistYZ{this, "localDistYZ", config().localDistYZ};
  ParameterRef<std::vector<double>> m_globallDistRPhi{this, "globalDistRPhi",
                                                      config().globalDistRPhi};
  ParameterRef<std::vector<double>> m_globalDistEtaPhi{this, "globalDistEtaPhi",
                                                       config().globalDistEtaPhi};
  ParameterRef<std::vector<double>> m_dimScalledLocalDistXY{this, "dimScaledLocalDistXY",
                                                            config().dimScaledLocalDistXY};
  ParameterRef<std::string> m_adjacencyMatrix{this, "adjacencyMatrix", config().adjacencyMatrix};
  ParameterRef<std::string> m_readout{this, "readoutClass", config().readout};
  ParameterRef<bool> m_splitCluster{this, "splitCluster", config().splitCluster};
  ParameterRef<double> m_minClusterHitEdep{this, "minClusterHitEdep", config().minClusterHitEdep};
  ParameterRef<double> m_minClusterCenterEdep{this, "minClusterCenterEdep",
                                              config().minClusterCenterEdep};
  ParameterRef<std::string> m_tepm{this, "transverseEnergyProfileMetric",
                                   config().transverseEnergyProfileMetric};
  ParameterRef<double> m_teps{this, "transverseEnergyProfileScale",
                              config().transverseEnergyProfileScale};

  Service<AlgorithmsInit_service> m_algorithmsInit{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    // Remove spaces from adjacency matrix
    // cfg.adjacencyMatrix.erase(
    //  std::remove_if(cfg.adjacencyMatrix.begin(), cfg.adjacencyMatrix.end(), ::isspace), cfg.adjacencyMatrix.end());
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void ChangeRun(int32_t /* run_number */) {}

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_calo_hit_input()}, {m_proto_cluster_output().get()});
  }
};

} // namespace eicrecon
