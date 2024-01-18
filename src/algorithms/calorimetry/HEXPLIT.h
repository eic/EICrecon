// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Sebouh Paul

// An algorithm for splitting calorimeter hits in overlapping cells into "subhits" based on the relative
// energies of hits on neighboring layers
//
// Author: Sebouh Paul
// Date: 12/04/2023

#pragma once

#include <algorithms/algorithm.h>
#include <DD4hep/Detector.h>
#include <edm4eic/CalorimeterHitCollection.h>
#include <spdlog/logger.h>
#include <memory>

#include "HEXPLITConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

  class HEXPLIT : public WithPodConfig<HEXPLITConfig> {

  public:
    void init(const dd4hep::Detector* detector, std::shared_ptr<spdlog::logger>& logger);
    std::unique_ptr<edm4eic::CalorimeterHitCollection> process(const edm4eic::CalorimeterHitCollection &hits) ;

  private:



    const int SUBCELLS=12;
//positions where the overlapping cells are relative to a given cell (in units of hexagon side length)
    static const double neighbor_offsets_x[12];
    static const double neighbor_offsets_y[12];

    //indices of the neighboring cells which overlap to produce a given subcell
    static const int neighbor_indices[12][3];

//positions of the centers of subcells
    static const double subcell_offsets_x[12];
    static const double subcell_offsets_y[12];

  private:
    const dd4hep::Detector* m_detector;
    std::shared_ptr<spdlog::logger> m_log;

  };

} // namespace eicrecon
