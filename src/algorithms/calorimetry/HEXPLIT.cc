// Copyright (C) 2023 Sebouh Paul
// SPDX-License-Identifier: LGPL-3.0-or-later

// References:
//   https://arxiv.org/abs/2308.06939

#include <DD4hep/Alignments.h>
#include <DD4hep/DetElement.h>
#include <DD4hep/Objects.h>
#include <DD4hep/VolumeManager.h>
#include <Evaluator/DD4hepUnits.h>
#include <Math/GenVector/Cartesian3D.h>
#include <Math/GenVector/DisplacementVector3D.h>
#include <edm4hep/Vector3f.h>
#include <cstdlib>
#include <algorithm>
#include <cmath>
#include <gsl/pointers> // for not_null
#include <vector>

#include "HEXPLIT.h"
#include "algorithms/calorimetry/HEXPLITConfig.h"

namespace eicrecon {

//positions where the overlapping cells are relative to a given cell (in units of hexagon side length)
const std::vector<double> HEXPLIT::neighbor_offsets_x = []() {
  std::vector<double> x;
  double rs[2]      = {1.5, std::numbers::sqrt3 / 2.};
  double offsets[2] = {0, M_PI / 2};
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 6; j += 1) {
      x.push_back(rs[i] * cos(j * M_PI / 3 + offsets[i]));
    }
  }
  return x;
}();

const std::vector<double> HEXPLIT::neighbor_offsets_y = []() {
  std::vector<double> y;
  double rs[2]      = {1.5, std::numbers::sqrt3 / 2.};
  double offsets[2] = {0, M_PI / 2};
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 6; j += 1) {
      y.push_back(rs[i] * sin(j * M_PI / 3 + offsets[i]));
    }
  }
  return y;
}();

//indices of the neighboring cells which overlap to produce a given subcell
const int HEXPLIT::neighbor_indices[SUBCELLS][OVERLAP] = {
    {0, 11, 10}, {1, 6, 11}, {2, 7, 6}, {3, 8, 7},  {4, 9, 8},   {5, 10, 9},
    {6, 11, 7},  {7, 6, 8},  {8, 7, 9}, {9, 8, 10}, {10, 9, 11}, {11, 10, 6}};

//positions of the centers of subcells
const std::vector<double> HEXPLIT::subcell_offsets_x = []() {
  std::vector<double> x;
  double rs[2]      = {0.75, std::numbers::sqrt3 / 4.};
  double offsets[2] = {0, M_PI / 2};
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 6; j += 1) {
      x.push_back(rs[i] * cos(j * M_PI / 3 + offsets[i]));
    }
  }
  return x;
}();

const std::vector<double> HEXPLIT::subcell_offsets_y = []() {
  std::vector<double> y;
  double rs[2]      = {0.75, std::numbers::sqrt3 / 4.};
  double offsets[2] = {0, M_PI / 2};
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 6; j += 1) {
      y.push_back(rs[i] * sin(j * M_PI / 3 + offsets[i]));
    }
  }
  return y;
}();

void HEXPLIT::init() {}

void HEXPLIT::process(const HEXPLIT::Input& input, const HEXPLIT::Output& output) const {

  const auto [hits]  = input;
  auto [subcellHits] = output;

  double MIP   = m_cfg.MIP / dd4hep::GeV;
  double delta = m_cfg.delta_in_MIPs * MIP;
  double Emin  = m_cfg.Emin_in_MIPs * MIP;
  double tmax  = m_cfg.tmax / dd4hep::ns;

  auto volman = m_detector->volumeManager();

  for (const auto& hit : *hits) {
    //skip hits that do not pass E and t cuts
    if (hit.getEnergy() < Emin || hit.getTime() > tmax) {
      continue;
    }

    //keep track of the energy in each neighboring cell
    std::vector<double> Eneighbors(NEIGHBORS, 0.0);

    double sl = hit.getDimension().x / 2.;
    for (const auto& other_hit : *hits) {
      // maximum distance between where the neighboring cell is and where it should be
      // based on an ideal geometry using the staggered tessellation pattern.
      // Deviations could arise from rounding errors or from detector misalignment.
      double tol = 0.1; // in units of side lengths.

      //only look at hits nearby within two layers of the current layer
      int dz = std::abs(hit.getLayer() - other_hit.getLayer());
      if (dz > 2 || dz == 0) {
        continue;
      }
      if (other_hit.getEnergy() < Emin || other_hit.getTime() > tmax) {
        continue;
      }
      //difference in transverse position (in units of side lengths)
      double dx = (other_hit.getLocal().x - hit.getLocal().x) / sl;
      double dy = (other_hit.getLocal().y - hit.getLocal().y) / sl;
      if (std::abs(dx) > 2 || std::abs(dy) > std::numbers::sqrt3) {
        continue;
      }

      //loop over locations of the neighboring cells
      //and check if the jth hit matches this location
      for (int k = 0; k < NEIGHBORS; k++) {
        if (std::abs(dx - neighbor_offsets_x[k]) < tol &&
            std::abs(dy - neighbor_offsets_y[k]) < tol) {
          Eneighbors[k] += other_hit.getEnergy();
          break;
        }
      }
    }
    double weights[SUBCELLS];
    for (int k = 0; k < NEIGHBORS; k++) {
      Eneighbors[k] = std::max(Eneighbors[k], delta);
    }
    double sum_weights = 0;
    for (int k = 0; k < SUBCELLS; k++) {
      weights[k] = Eneighbors[neighbor_indices[k][0]] * Eneighbors[neighbor_indices[k][1]] *
                   Eneighbors[neighbor_indices[k][2]];
      sum_weights += weights[k];
    }
    for (int k = 0; k < SUBCELLS; k++) {

      //create the subcell hits.  First determine their positions in local coordinates.
      const decltype(edm4eic::CalorimeterHitData::local) local(
          hit.getLocal().x + subcell_offsets_x[k] * sl,
          hit.getLocal().y + subcell_offsets_y[k] * sl, hit.getLocal().z);

      //convert this to a position object so that the global position can be determined
      dd4hep::Position local_position;
      local_position.SetX(local.x * dd4hep::mm);
      local_position.SetY(local.y * dd4hep::mm);
      local_position.SetZ(local.z * dd4hep::mm);

      dd4hep::Position global_position;
      try {

        //also convert this to the detector's global coordinates.  To do: check if this is correct
        auto alignment = volman.lookupDetElement(hit.getCellID()).nominal();

        global_position = alignment.localToWorld(local_position);

      } catch (...) {
        // do this to prevent errors when running the test on the mock detector
        warning("Cannot find transformation from local to global coordinates.");
        global_position = local_position;
      }

      //convert this from position object to a vector object
      const decltype(edm4eic::CalorimeterHitData::position) position = {
          static_cast<float>(global_position.X() / dd4hep::mm),
          static_cast<float>(global_position.Y() / dd4hep::mm),
          static_cast<float>(global_position.Z() / dd4hep::mm)};

      //bounding box dimensions depend on the orientation of the rhombus
      int orientation = static_cast<int>(k % 3 == 0);
      const decltype(edm4eic::CalorimeterHitData::dimension) dimension(
          sl * (orientation != 0 ? 1 : 1.5),
          sl * std::numbers::sqrt3 / 2. * (orientation != 0 ? 2 : 1), hit.getDimension()[2]);

      subcellHits->create(hit.getCellID(), hit.getEnergy() * weights[k] / sum_weights, 0,
                          hit.getTime(), 0, position, dimension, hit.getSector(), hit.getLayer(),
                          local);
    }
  }
}

} // namespace eicrecon
