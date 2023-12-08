// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Sebouh Paul

// An algorithm for splitting calorimeter hits in overlapping cells into "subhits" based on the relative
// energies of hits on neighboring layers
//
// Author: Sebouh Paul
// Date: 12/04/2023


#pragma once
#include <DD4hep/Detector.h>
#include <DD4hep/IDDescriptor.h>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4eic/ProtoClusterCollection.h>
#include <edm4hep/Vector2f.h>
#include <edm4hep/utils/vector_utils.h>
#include <fmt/core.h>
#include <spdlog/logger.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <exception>
#include <functional>
#include <memory>
#include <set>
#include <vector>

#include "algorithms/interfaces/WithPodConfig.h"
#include "HEXPLITConfig.h"

namespace eicrecon {

  class HEXPLIT : public WithPodConfig<HEXPLITConfig> {

  public:
    void init(const dd4hep::Detector* detector, std::shared_ptr<spdlog::logger>& logger);
    std::unique_ptr<edm4eic::CalorimeterHitCollection> process(const edm4eic::CalorimeterHitCollection &hits) ;
    
  private:
    
    
    
    const int SUBCELLS=12;
//positions where the overlapping cells are relative to a given cell (in units of hexagon side length)
    double neighbor_offsets_x[12]={1.5*cos(0), 1.5*cos(M_PI/3), 1.5*cos(2*M_PI/3),1.5*cos(3*M_PI/3), 1.5*cos(4*M_PI/3), 1.5*cos(5*M_PI/3),
                             -sqrt(3)/2.*sin(0),-sqrt(3)/2.*sin(M_PI/3),-sqrt(3)/2.*sin(2*M_PI/3),-sqrt(3)/2.*sin(3*M_PI/3),-sqrt(3)/2.*sin(4*M_PI/3),-sqrt(3)/2.*sin(5*M_PI/3)};
    double neighbor_offsets_y[12]={1.5*sin(0), 1.5*sin(M_PI/3), 1.5*sin(2*M_PI/3),1.5*sin(3*M_PI/3), 1.5*sin(4*M_PI/3), 1.5*sin(5*M_PI/3),
                              sqrt(3)/2.*cos(0), sqrt(3)/2.*cos(M_PI/3), sqrt(3)/2.*cos(2*M_PI/3), sqrt(3)/2.*cos(3*M_PI/3), sqrt(3)/2.*cos(4*M_PI/3), sqrt(3)/2.*cos(5*M_PI/3)};

    //indices of the neighboring cells which overlap to produce a given subcell
    int neighbor_indices[12][3]={{0, 11,10}, {1, 6, 11},{2, 7, 6}, {3,8,7}, {4,9,8}, {5,10,9},
                             {6, 11, 7}, {7, 6, 8}, {8, 7, 9}, {9,8,10},{10,9,11},{11,10,6}};

//positions of the centers of subcells
    double subcell_offsets_x[12]={0.75*cos(0), 0.75*cos(M_PI/3), 0.75*cos(2*M_PI/3), 0.75*cos(3*M_PI/3), 0.75*cos(4*M_PI/3), 0.75*cos(5*M_PI/3),
                            -sqrt(3)/4*sin(0),-sqrt(3)/4*sin(M_PI/3),-sqrt(3)/4*sin(2*M_PI/3),-sqrt(3)/4*sin(3*M_PI/3),-sqrt(3)/4*sin(4*M_PI/3),-sqrt(3)/4*sin(5*M_PI/3)};
    double subcell_offsets_y[12]={0.75*sin(0), 0.75*sin(M_PI/3), 0.75*sin(2*M_PI/3), 0.75*sin(3*M_PI/3), 0.75*sin(4*M_PI/3), 0.75*sin(5*M_PI/3),
                             sqrt(3)/4*cos(0), sqrt(3)/4*cos(M_PI/3), sqrt(3)/4*cos(2*M_PI/3), sqrt(3)/4*cos(3*M_PI/3), sqrt(3)/4*cos(4*M_PI/3), sqrt(3)/4*cos(5*M_PI/3)};

  private:
    const dd4hep::Detector* m_detector;
    std::shared_ptr<spdlog::logger> m_log;

  };

} // namespace eicrecon
