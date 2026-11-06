// Copyright (C) 2023 Wouter Deconinck
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <string>
#include <variant>
#include <edm4eic/unit_system.h>

namespace eicrecon {

struct CalorimeterIslandClusterConfig {

  std::string adjacencyMatrix;
  std::string peakNeighbourhoodMatrix;
  std::string readout;

  // neighbour checking distances
  double sectorDist;
  std::vector<std::variant<std::string, double>> localDistXY;
  std::vector<double> localDistXZ;
  std::vector<double> localDistYZ;
  std::vector<double> globalDistRPhi;
  std::vector<double> globalDistEtaPhi;
  std::vector<double> dimScaledLocalDistXY;

  bool splitCluster{false};
  double minClusterHitEdep;
  double minClusterCenterEdep;

  std::string transverseEnergyProfileMetric;
  double transverseEnergyProfileScale;
  double transverseEnergyProfileScaleUnits;

  // if > 0, neighbouring hits must also be within this time window (ns)
  double timeWindow{0.0 * edm4eic::unit::ns};
};

} // namespace eicrecon
