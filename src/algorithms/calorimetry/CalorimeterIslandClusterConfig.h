// Copyright (C) 2023 Wouter Deconinck
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <string>

namespace eicrecon {

    struct CalorimeterIslandClusterConfig {

        std::string adjacencyMatrix;
        std::string readout;

        // neighbour checking distances
        double sectorDist;
        std::vector<double> localDistXY;
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

    };

} // eicrecon
