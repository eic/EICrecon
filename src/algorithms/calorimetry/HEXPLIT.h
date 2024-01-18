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

using HEXPLITAlgorithm = algorithms::Algorithm<
     algorithms::Input<
        const edm4eic::CalorimeterHitCollection
     >,
     algorithms::Output<
        edm4eic::CalorimeterHitCollection
     >
   >;

  class HEXPLIT
  : public HEXPLITAlgorithm,
    public WithPodConfig<HEXPLITConfig> {

  public:
    HEXPLIT(std::string_view name)
             : HEXPLITAlgorithm{name,
                                   {"inputHits"},
                                   {"outputSubcellHits"},
                                   "Split hits into subcell hits"} {}

    void init(const dd4hep::Detector* detector, std::shared_ptr<spdlog::logger>& logger);
    void process(const Input&, const Output&) const final;

  private:
    static constexpr int SUBCELLS = 12;

    // positions where the overlapping cells are relative to a given cell (in units of hexagon side length)
    static const std::array<double, SUBCELLS> neighbor_offsets_x;
    static const std::array<double, SUBCELLS> neighbor_offsets_y;

    // indices of the neighboring cells which overlap to produce a given subcell
    static const std::array<std::array<int, 3>, SUBCELLS> neighbor_indices;

    // positions of the centers of subcells
    static const std::array<double, SUBCELLS> subcell_offsets_x;
    static const std::array<double, SUBCELLS> subcell_offsets_y;

  private:
    const dd4hep::Detector* m_detector;
    std::shared_ptr<spdlog::logger> m_log;

  };

} // namespace eicrecon
