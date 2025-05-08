// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024-2025 Chun Yuen Tsang, Prithwish Tribedy, Simon Gardner
//
// Spread energy deposition from one strip to neighboring strips within sensor boundaries

#include <DD4hep/DetElement.h>
#include <DD4hep/detail/SegmentationsInterna.h>
#include <DD4hep/detail/Handle.inl>
#include <DD4hep/Handle.h>
#include <DD4hep/Readout.h>
#include <DD4hep/Objects.h>
#include <DD4hep/Segmentations.h>
#include <DD4hep/MultiSegmentation.h>
#include <DDSegmentation/BitFieldCoder.h>
#include <DDSegmentation/MultiSegmentation.h>
#include <DDSegmentation/Segmentation.h>
#include <DD4hep/Volumes.h>
#include <Evaluator/DD4hepUnits.h>
#include <Math/GenVector/Cartesian3D.h>
#include <Math/GenVector/DisplacementVector3D.h>
#include <TGeoManager.h>
#include <TGeoMatrix.h>
#include <TGeoVolume.h>
#include <algorithms/geo.h>
#include <algorithms/service.h>
#include <fmt/core.h>
#include <cmath>
#include <gsl/pointers>
#include <stdexcept>
#include <utility>
#include <vector>

#include "DD4hep/Detector.h"
#include "SiliconChargeSharing.h"
#include "algorithms/digi/SiliconChargeSharingConfig.h"
#include "services/evaluator/EvaluatorSvc.h"

namespace eicrecon {

void SiliconChargeSharing::init() {
  m_converter = algorithms::GeoSvc::instance().cellIDPositionConverter();
  m_seg       = algorithms::GeoSvc::instance().detector()->readout(m_cfg.readout).segmentation();
}

void SiliconChargeSharing::process(const SiliconChargeSharing::Input& input,
                                   const SiliconChargeSharing::Output& output) const {
  const auto [simhits] = input;
  auto [sharedHits]    = output;

  for (const auto& hit : *simhits) {

    auto cellID = hit.getCellID();

    auto element = &m_converter->findContext(cellID)->element; // volume context
    // ToDo: Move this to init() and set it once for every detelement associated with the readout
    // Set transformation map if it hasn't already been set
    m_transform_map.try_emplace(element, &element->nominal().worldTransformation());
    m_segmentation_map.try_emplace(element, getLocalSegmentation(cellID));

    // Try and get a box of the detectorElement solid
    try {
      dd4hep::Box box = element->solid();

      if (box) {
        m_x_range_map.try_emplace(element, box->GetDX());
        m_y_range_map.try_emplace(element, box->GetDY());
      }
    } catch (const std::bad_cast& e) {
      error("Failed to cast solid to Box: {}", e.what());
    }

    auto edep         = hit.getEDep();
    auto globalHitPos = hit.getPosition();
    auto hitPos =
        global2Local(dd4hep::Position(globalHitPos.x * dd4hep::mm, globalHitPos.y * dd4hep::mm,
                                      globalHitPos.z * dd4hep::mm),
                     m_transform_map[element]);

    std::unordered_set<dd4hep::rec::CellID> tested_cells;
    std::vector<std::pair<dd4hep::rec::CellID, float>> cell_charge;
    findAllNeighborsInSensor(cellID, tested_cells, cell_charge, edep, hitPos, element);

    //cout number of neighbors found
    // error("Found {} neighbors for cellID {}", cell_charge.size(), cellID);

    // Create a new simhit for each cell with deposited energy
    for (const auto& [testCellID, edep_cell] : cell_charge) {
      auto globalCellPos = m_converter->position(testCellID);

      edm4hep::MutableSimTrackerHit newHit = hit.clone();
      newHit.setCellID(testCellID);
      newHit.setEDep(edep_cell);
      newHit.setPosition({globalCellPos.x(), globalCellPos.y(), globalCellPos.z()});
      sharedHits->push_back(newHit);
    }

  } // for simhits
} // SiliconChargeSharing:process

// Recursively find neighbors where a charge is deposited
void SiliconChargeSharing::findAllNeighborsInSensor(
    const dd4hep::rec::CellID testCellID, std::unordered_set<dd4hep::rec::CellID>& tested_cells,
    std::vector<std::pair<dd4hep::rec::CellID, float>>& cell_charge, const float edep,
    const dd4hep::Position hitPos, const dd4hep::DetElement* element) const {

  // Tag cell as tested
  tested_cells.insert(testCellID);

  auto localPos = cell2LocalPosition(testCellID);

  // Check if the cell is within the sensor boundaries
  if (std::abs(localPos.x()) > m_x_range_map[element] ||
      std::abs(localPos.y()) > m_y_range_map[element]) {
    return;
  }

  auto segmentation = m_segmentation_map[element];

  // cout the local position and hit position
  auto xDimension = segmentation->gridSizeX();
  auto yDimension = segmentation->gridSizeY();
  // Calculate deposited energy in cell
  float edepCell = energyAtCell(xDimension, yDimension, localPos, hitPos, edep);
  // error("energy {} at cellID {}", edepCell, testCellID);
  if (edepCell < m_cfg.min_edep) {
    return;
  }

  // Store cellID and deposited energy
  cell_charge.push_back(std::make_pair(testCellID, edepCell));

  // As there is charge in the cell, test the neighbors too
  std::set<dd4hep::rec::CellID> testCellNeighbours;
  segmentation->neighbours(testCellID, testCellNeighbours);

  for (const auto& neighbourCell : testCellNeighbours) {
    // error("Testing neighbour cellID {}", neighbourCell);
    if (tested_cells.find(neighbourCell) == tested_cells.end()) {
      findAllNeighborsInSensor(neighbourCell, tested_cells, cell_charge, edep, hitPos, element);
    }
  }
}

// Calculate integral of Gaussian distribution
float SiliconChargeSharing::integralGaus(float mean, float sd, float low_lim, float up_lim) const {
  // return integral Gauss(mean, sd) dx from x = low_lim to x = up_lim
  // default value is set when sd = 0
  float up  = mean > up_lim ? -0.5 : 0.5;
  float low = mean > low_lim ? -0.5 : 0.5;
  if (sd > 0) {
    up  = -0.5 * std::erf(std::sqrt(2) * (mean - up_lim) / sd);
    low = -0.5 * std::erf(std::sqrt(2) * (mean - low_lim) / sd);
  }
  return up - low;
}

// Convert cellID to local position
dd4hep::Position SiliconChargeSharing::cell2LocalPosition(const dd4hep::rec::CellID& cell) const {
  auto position = m_seg->position(cell); // local position
  return position;
}

// Convert global position to local position
dd4hep::Position SiliconChargeSharing::global2Local(const dd4hep::Position& globalPosition,
                                                    const TGeoHMatrix* transform) const {

  double g[3];
  double l[3];

  globalPosition.GetCoordinates(g);
  transform->MasterToLocal(g, l);
  dd4hep::Position localPosition;
  localPosition.SetCoordinates(l);
  return localPosition;
}

// Calculate energy deposition in a cell relative to the hit position
float SiliconChargeSharing::energyAtCell(const double xDimension, const double yDimension,
                                         const dd4hep::Position localPos,
                                         const dd4hep::Position hitPos, const float edep) const {
  float energy = edep *
                 integralGaus(hitPos.x(), m_cfg.sigma_sharingx, localPos.x() - 0.5 * xDimension,
                              localPos.x() + 0.5 * xDimension) *
                 integralGaus(hitPos.y(), m_cfg.sigma_sharingy, localPos.y() - 0.5 * yDimension,
                              localPos.y() + 0.5 * yDimension);
  return energy;
}

//Get the segmentation relevent to a cellID
const dd4hep::DDSegmentation::CartesianGridXY*
SiliconChargeSharing::getLocalSegmentation(const dd4hep::rec::CellID& cellID) const {
  // Get the segmentation type
  auto segmentation_type                                   = m_seg.type();
  const dd4hep::DDSegmentation::Segmentation* segmentation = m_seg.segmentation();
  // Check if the segmentation is a multi-segmentation
  while (segmentation_type == "MultiSegmentation") {
    const auto* multi_segmentation =
        dynamic_cast<const dd4hep::DDSegmentation::MultiSegmentation*>(segmentation);
    segmentation      = &multi_segmentation->subsegmentation(cellID);
    segmentation_type = segmentation->type();
  }

  // Try to cast the segmentation to CartesianGridXY
  const auto* cartesianGrid =
      dynamic_cast<const dd4hep::DDSegmentation::CartesianGridXY*>(segmentation);
  if (!cartesianGrid) {
    throw std::runtime_error("Segmentation is not of type CartesianGridXY");
  }

  return cartesianGrid;
}

} // namespace eicrecon
