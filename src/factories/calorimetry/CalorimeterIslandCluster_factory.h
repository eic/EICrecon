// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Wouter Deconinck

#pragma once

#include "algorithms/calorimetry/CalorimeterIslandCluster.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

class CalorimeterIslandCluster_factory
    : public JOmniAlgorithmFactory<CalorimeterIslandCluster_factory,
                                   eicrecon::CalorimeterIslandCluster,
                                   CalorimeterIslandClusterConfig> {
private:
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
};

} // namespace eicrecon
