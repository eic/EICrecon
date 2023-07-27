// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Chao Peng, Sylvester Joosten, Whitney Armstrong

#pragma once

#include <string>

namespace eicrecon {

  struct ImagingTopoClusterConfig {

    // maximum difference in layer numbers that can be considered as neighbours
    int neighbourLayersRange = 1;
    // maximum distance of local (x, y) to be considered as neighbors at the same layer
    std::vector<double> localDistXY = {1.0 * dd4hep::mm, 1.0 * dd4hep::mm};
    // maximum distance of global (eta, phi) to be considered as neighbors at different layers
    std::vector<double> layerDistEtaPhi = {0.01, 0.01};
    // maximum global distance to be considered as neighbors in different sectors
    double sectorDist = 1.0 * dd4hep::cm;

    // minimum hit energy to participate clustering
    double minClusterHitEdep = 0.;
    // minimum cluster center energy (to be considered as a seed for cluster)
    double minClusterCenterEdep = 0.;
    // minimum cluster energy (to save this cluster)
    double minClusterEdep = 0.5 * dd4hep::MeV;
    // minimum number of hits (to save this cluster)
    int minClusterNhits = 10;

  };

} // namespace eicrecon
