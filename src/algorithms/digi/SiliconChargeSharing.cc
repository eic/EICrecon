// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024-2025 Chun Yuen Tsang, Prithwish Tribedy, Simon Gardner
//
// Spread energy deposition from one strip to neighboring strips within sensor boundaries

#include <DD4hep/Alignments.h>
#include <DD4hep/DetElement.h>
#include <DD4hep/Handle.h>
#include <DD4hep/Objects.h>
#include <DD4hep/Readout.h>
#include <DD4hep/Segmentations.h>
#include <DD4hep/Shapes.h>
#include <DD4hep/VolumeManager.h>
#include <DD4hep/detail/SegmentationsInterna.h>
#include <DDSegmentation/MultiSegmentation.h>
#include <DDSegmentation/Segmentation.h>
#include <Evaluator/DD4hepUnits.h>
#include <Math/GenVector/Cartesian3D.h>
#include <Math/GenVector/DisplacementVector3D.h>
#include <RtypesCore.h>
#include <TGeoBBox.h>
#include <TGeoMatrix.h>
#include <algorithms/geo.h>
#include <edm4hep/Vector3d.h>
#include <fmt/core.h>
#include <cmath>
#include <gsl/pointers>
#include <set>
#include <stdexcept>
#include <typeinfo>
#include <utility>

#include "DD4hep/Detector.h"
#include "SiliconChargeSharing.h"
#include "algorithms/digi/SiliconChargeSharingConfig.h"

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

    const auto* element = &m_converter->findContext(cellID)->element; // volume context
    // ToDo: Move this to init() and set it once for every detelement associated with the readout
    // Set transformation map if it hasn't already been set
    auto [transformIt, transformInserted] =
        m_transform_map.try_emplace(element, &element->nominal().worldTransformation());
    auto [segmentationIt, segmentationInserted] =
        m_segmentation_map.try_emplace(element, getLocalSegmentation(cellID));

    // Try and get a box of the detectorElement solid requiring segmentation
    // to be a CartesianGridXY, throwing exception in getLocalSegmentation
    try {
      dd4hep::Box box = element->solid();
      m_xy_range_map.try_emplace(element, box->GetDX(), box->GetDY());
    } catch (const std::bad_cast& e) {
      error("Failed to cast solid to Box: {}", e.what());
    }

    auto edep         = hit.getEDep();
    auto globalHitPos = hit.getPosition();
    auto hitPos =
        global2Local(dd4hep::Position(globalHitPos.x * dd4hep::mm, globalHitPos.y * dd4hep::mm,
                                      globalHitPos.z * dd4hep::mm),
                     transformIt->second);

    // therefore, we search neighbors within the segmentation of the same volume
    // to find the cell ID that correspond to globalHitPos.
    // Precise reason unknown, but we suspect it's cause by steps in Geant4
    // Perhaps position is the average of all steps in volume while cellID is just the first cell the track hits
    // They disagree when there are multiple step and scattering inside the volume
    const dd4hep::Position dummy;
    cellID = segmentationIt->second->cellID(hitPos, dummy, cellID);

    std::unordered_set<dd4hep::rec::CellID> tested_cells;
    std::unordered_map<dd4hep::rec::CellID, float> cell_charge;

    // Warning: This function is recursive, it stops shen it finds the edge of a detector element
    // or when the energy deposited in a cell is below the configured threshold
    findAllNeighborsInSensor(cellID, tested_cells, edep, hitPos, segmentationIt->second,
                             m_xy_range_map[element], hit, sharedHits);

  } // for simhits
} // SiliconChargeSharing:process

// Recursively find neighbors where a charge is deposited
void SiliconChargeSharing::findAllNeighborsInSensor(
    const dd4hep::rec::CellID testCellID, std::unordered_set<dd4hep::rec::CellID>& tested_cells,
    const float edep, const dd4hep::Position hitPos,
    const dd4hep::DDSegmentation::CartesianGridXY* segmentation,
    const std::pair<double, double>& xy_range, const edm4hep::SimTrackerHit& hit,
    edm4hep::SimTrackerHitCollection* sharedHits) const {

  // Tag cell as tested
  tested_cells.insert(testCellID);

  auto localPos = cell2LocalPosition(testCellID);

  // Check if the cell is within the sensor boundaries
  if (std::abs(localPos.x()) > xy_range.first || std::abs(localPos.y()) > xy_range.second) {
    return;
  }

  // cout the local position and hit position
  auto xDimension = segmentation->gridSizeX();
  auto yDimension = segmentation->gridSizeY();
  // Calculate deposited energy in cell
  float edepCell = energyAtCell(xDimension, yDimension, localPos, hitPos, edep);
  if (edepCell <= m_cfg.min_edep) {
    return;
  }

  // Create a new simhit for cell with deposited energy
  auto globalCellPos = m_converter->position(testCellID);

  edm4hep::MutableSimTrackerHit shared_hit = hit.clone();
  shared_hit.setCellID(testCellID);
  shared_hit.setEDep(edepCell);
  shared_hit.setPosition({globalCellPos.x() / dd4hep::mm, globalCellPos.y() / dd4hep::mm,
                          globalCellPos.z() / dd4hep::mm});
  sharedHits->push_back(shared_hit);

  // As there is charge in the cell, test the neighbors too
  std::set<dd4hep::rec::CellID> testCellNeighbours;
  segmentation->neighbours(testCellID, testCellNeighbours);

  for (const auto& neighbourCell : testCellNeighbours) {
    if (tested_cells.find(neighbourCell) == tested_cells.end()) {
      findAllNeighborsInSensor(neighbourCell, tested_cells, edep, hitPos, segmentation, xy_range,
                               hit, sharedHits);
    }
  }
}

// Calculate integral of Gaussian distribution
float SiliconChargeSharing::integralGaus(float mean, float sd, float low_lim, float up_lim) {
  // return integral Gauss(mean, sd) dx from x = low_lim to x = up_lim
  // default value is set when sd = 0
  float up  = mean > up_lim ? -0.5 : 0.5;
  float low = mean > low_lim ? -0.5 : 0.5;
  if (sd > 0) {
    up  = -0.5 * std::erf(std::numbers::sqrt2 * (mean - up_lim) / sd);
    low = -0.5 * std::erf(std::numbers::sqrt2 * (mean - low_lim) / sd);
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
                                                    const TGeoHMatrix* transform) {

  double g[3];
  double l[3];

  globalPosition.GetCoordinates(static_cast<Double_t*>(g));
  transform->MasterToLocal(static_cast<const Double_t*>(g), static_cast<Double_t*>(l));
  dd4hep::Position localPosition;
  localPosition.SetCoordinates(static_cast<const Double_t*>(l));
  return localPosition;
}

// Calculate energy deposition in a cell relative to the hit position
float SiliconChargeSharing::energyAtCell(const double xDimension, const double yDimension,
                                         const dd4hep::Position localPos,
                                         const dd4hep::Position hitPos, const float edep) const {
  auto sigma_sharingx = m_cfg.sigma_sharingx;
  auto sigma_sharingy = m_cfg.sigma_sharingy;
  if (m_cfg.sigma_mode == SiliconChargeSharingConfig::ESigmaMode::rel) {
    sigma_sharingx *= xDimension;
    sigma_sharingy *= yDimension;
  }
  float energy = edep *
                 integralGaus(hitPos.x(), sigma_sharingx, localPos.x() - 0.5 * xDimension,
                              localPos.x() + 0.5 * xDimension) *
                 integralGaus(hitPos.y(), sigma_sharingy, localPos.y() - 0.5 * yDimension,
                              localPos.y() + 0.5 * yDimension);
  return energy;
}

// Get the segmentation relevant to a cellID
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
  if (cartesianGrid == nullptr) {
    throw std::runtime_error("Segmentation is not of type CartesianGridXY");
  }

  return cartesianGrid;
}

} // namespace eicrecon
