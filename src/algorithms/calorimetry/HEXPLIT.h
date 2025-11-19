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
    typedef struct stagger_pattern{
      int SUBCELLS;
      int NEIGHBORS;
      int OVERLAP;
      std::vector<double> neighbor_offsets_x;
      std::vector<double> neighbor_offsets_y;
      std::vector<std::vector<int>> neighbor_indices;
      std::vector<double>  subcell_offsets_x;
      std::vector<double>  subcell_offsets_y;
    } stagger_pattern;
      
  // number of subcells that a single cell is divided into
  static const int SUBCELLS_H4=12;
  // number of neighboring positions whose overlap define the subcells
  static const int NEIGHBORS_H4=12;
  // number of neighboring cells that overlap to obtain a subcell
  static const int OVERLAP_H4=3;
  //positions where the overlapping cells are relative to a given cell (in units of hexagon side length)
  static const std::vector<double> neighbor_offsets_x_H4;
  static const std::vector<double>  neighbor_offsets_y_H4;
  //indices of the neighboring cells which overlap to produce a given subcell
  static const std::vector<std::vector<int>> neighbor_indices_H4;
  //positions of the centers of subcells
  static const std::vector<double>  subcell_offsets_x_H4;
  static const std::vector<double>  subcell_offsets_y_H4;

  const stagger_pattern stag_H4 = {
        .SUBCELLS=SUBCELLS_H4,
        .NEIGHBORS=NEIGHBORS_H4,
        .neighbor_offsets_x=neighbor_offsets_x_H4,
        .neighbor_offsets_y=neighbor_offsets_y_H4,
        .neighbor_indices=neighbor_indices_H4,
        .subcell_offsets_x=subcell_offsets_x_H4,
        .subcell_offsets_y=subcell_offsets_y_H4,
      };
      
  static const int SUBCELLS_S2=4;
  // number of neighboring positions whose overlap define the subcells
  static const int NEIGHBORS_S2=4;
  // number of neighboring cells that overlap to obtain a subcell
  static const int OVERLAP_S2=1;
  //positions where the overlapping cells are relative to a given cell (in units of hexagon side length)
  static const std::vector<double> neighbor_offsets_x_S2;
  static const std::vector<double>  neighbor_offsets_y_S2;
  //indices of the neighboring cells which overlap to produce a given subcell
  static const std::vector<std::vector<int>> neighbor_indices_S2;
  //positions of the centers of subcells
  static const std::vector<double>  subcell_offsets_x_S2;
  static const std::vector<double>  subcell_offsets_y_S2;

  const stagger_pattern stag_S2 = {
        .SUBCELLS=SUBCELLS_S2,
        .NEIGHBORS=NEIGHBORS_S2,
        .neighbor_offsets_x=neighbor_offsets_x_S2,
        .neighbor_offsets_y=neighbor_offsets_y_S2,
        .neighbor_indices=neighbor_indices_S2,
        .subcell_offsets_x=subcell_offsets_x_S2,
        .subcell_offsets_y=subcell_offsets_y_S2,
      };
      
  stagger_pattern stag=stag_H4;
      
private:
  const dd4hep::Detector* m_detector{algorithms::GeoSvc::instance().detector()};

  };

} // namespace eicrecon
