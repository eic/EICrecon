// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Sebouh Paul

// An algorithm for splitting calorimeter hits in overlapping cells into "subhits" based on the relative
// energies of hits on neighboring layers
//
// Author: Sebouh Paul
// Date: 12/04/2023

#pragma once

#include <DD4hep/Detector.h>
#include <algorithms/algorithm.h>
#include <algorithms/geo.h>
#include <edm4eic/CalorimeterHitCollection.h>
#include <gsl/pointers>
#include <string>      // for basic_string
#include <string_view> // for string_view
#include <vector>

#include "HEXPLITConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using HEXPLITAlgorithm =
    algorithms::Algorithm<algorithms::Input<const edm4eic::CalorimeterHitCollection>,
                          algorithms::Output<edm4eic::CalorimeterHitCollection>>;

class HEXPLIT : public HEXPLITAlgorithm, public WithPodConfig<HEXPLITConfig> {

public:
  HEXPLIT(std::string_view name)
      : HEXPLITAlgorithm{
            name, {"inputHits"}, {"outputSubcellHits"}, "Split hits into subcell hits"} {}

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  // number of subcells that a single cell is divided into
  static const int SUBCELLS = 12;
  // number of neighboring positions whose overlap define the subcells
  static const int NEIGHBORS = 12;
  // number of neighboring cells that overlap to obtain a subcell
  static const int OVERLAP = 3;
  //positions where the overlapping cells are relative to a given cell (in units of hexagon side length)
  static const std::vector<double> neighbor_offsets_x;
  static const std::vector<double> neighbor_offsets_y;
  //indices of the neighboring cells which overlap to produce a given subcell
  static const int neighbor_indices[SUBCELLS][OVERLAP];
  //positions of the centers of subcells
  static const std::vector<double> subcell_offsets_x;
  static const std::vector<double> subcell_offsets_y;

private:
  const dd4hep::Detector* m_detector{algorithms::GeoSvc::instance().detector()};
};

} // namespace eicrecon
