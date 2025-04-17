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
#include <unordered_map>
#include <utility>
#include <vector>


#include "DD4hep/Detector.h"
#include "SiliconChargeSharing.h"
#include "algorithms/digi/SiliconChargeSharingConfig.h"
#include "services/evaluator/EvaluatorSvc.h"

namespace eicrecon {

void SiliconChargeSharing::init() {
  m_detector  = algorithms::GeoSvc::instance().detector();
  m_converter = algorithms::GeoSvc::instance().cellIDPositionConverter();
  m_seg       = m_detector->readout(m_cfg.readout).segmentation();
}

void SiliconChargeSharing::process(const SiliconChargeSharing::Input& input,
                                const SiliconChargeSharing::Output& output) const {
  const auto [simhits] = input;
  auto [sharedHits]    = output;

  for (const auto& hit : *simhits) {

    auto   cellID     = hit.getCellID();
    auto   edep       = hit.getEDep();
    auto   time       = hit.getTime();
    auto   momentum   = hit.getMomentum();
    auto   hitPos     = global2Local(hit);
    auto   mcParticle = hit.getMCParticle();


    std::unordered_set<dd4hep::rec::CellID> tested_cells;
    std::vector<std::pair<dd4hep::rec::CellID,float>> cell_charge;
    findAllNeighborsInSensor(cellID, tested_cells, cell_charge, edep, hitPos);

    //cout number of neighbors found
    error("Found {} neighbors for cellID {}", cell_charge.size(), cellID);

    // Create a new simhit for each cell with deposited energy
    for(const auto& [testCellID, edep_cell] : cell_charge) {
      auto globalPos = m_converter->position(testCellID);
      auto hit = sharedHits->create();
      hit.setCellID(testCellID);
      hit.setEDep(edep_cell);
      hit.setTime(time);
      hit.setPosition({globalPos.x(), globalPos.y(), globalPos.z()});
      hit.setMomentum({momentum.x, momentum.y, momentum.z});
      hit.setMCParticle(mcParticle);
    }

  } // for simhits
} // SiliconChargeSharing:process

// Recursively find neighbors where a charge is deposited
void SiliconChargeSharing::findAllNeighborsInSensor( dd4hep::rec::CellID testCellID,
    std::unordered_set<dd4hep::rec::CellID>& tested_cells, std::vector<std::pair<dd4hep::rec::CellID,float>>& cell_charge, float edep, dd4hep::Position hitPos) const {

  // Tag cell as tested
  tested_cells.insert(testCellID);

  // Calculate deposited energy in cell
  float edepCell = energyAtCell(testCellID, hitPos,edep);
  error("energy {} at cellID {}", edepCell, testCellID);
  if(edepCell <= m_cfg.min_edep) {
    return;
  }

  // Store cellID and deposited energy
  cell_charge.push_back(std::make_pair(testCellID, edepCell));

  // Get local segmentation if multiSegmentation is used
  auto segmentation = getLocalSegmentation(testCellID);

  // As there is charge in the cell, test the neighbors too
  std::set<dd4hep::rec::CellID> testCellNeighbours;
  segmentation.neighbours(testCellID, testCellNeighbours);
  std::string segType = segmentation->type();
  error("Segmentation type: {}", segType);

  error("sEGMENTATION {} neighbours for cellID {}", testCellNeighbours.size(), testCellID);

  for (const auto& neighbourCell : testCellNeighbours) {
    error("Testing neighbour cellID {}", neighbourCell);
    if (tested_cells.find(neighbourCell) == tested_cells.end()) {
      findAllNeighborsInSensor(neighbourCell, tested_cells, cell_charge, edep, hitPos);
    }
  }

}

// Calculate integral of Gaussian distribution
float SiliconChargeSharing::integralGaus(float mean, float sd, 
                                         float low_lim, float up_lim) const {
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
dd4hep::Position SiliconChargeSharing::global2Local(const edm4hep::SimTrackerHit& hit) const {
  auto  volumeManager = m_detector->volumeManager();
  auto  detelement    = volumeManager.lookupDetElement(hit.getCellID());
  const TGeoMatrix& transform = detelement.nominal().worldTransformation();

  // Units given by transformation do not match with the units of the hit position
  double g[3] = {hit.getPosition().x/10, hit.getPosition().y/10, hit.getPosition().z/10};
  double l[3];

  transform.MasterToLocal(g, l);
  dd4hep::Position position;
  position.SetCoordinates(l);
  return position;
}

// Calculate energy deposition in a cell relative to the hit position
float SiliconChargeSharing::energyAtCell(const dd4hep::rec::CellID& cell, dd4hep::Position hitPos, float edep) const {
  auto localPos      = cell2LocalPosition(cell);

  // cout the local position and hit position
  auto cellDimension = m_converter->cellDimensions(cell);
  float energy = edep*integralGaus(hitPos.x(), m_cfg.sigma_sharingx, localPos.x() - 0.5 * cellDimension[0],
                      localPos.x() + 0.5 * cellDimension[0]) *
                  integralGaus(hitPos.y(), m_cfg.sigma_sharingy, localPos.y() - 0.5 * cellDimension[1],
                      localPos.y() + 0.5 * cellDimension[1]);
  return energy;
}

dd4hep::Segmentation  SiliconChargeSharing::getLocalSegmentation(
    const dd4hep::rec::CellID& cellID) const {
  // Get the segmentation type
  auto segmentation_type = m_seg.type();
  const dd4hep::DDSegmentation::Segmentation* segmentation = m_seg.segmentation();
  // Check if the segmentation is a multi-segmentation
  while (segmentation_type == "MultiSegmentation") {
    const auto* multi_segmentation = dynamic_cast<const dd4hep::DDSegmentation::MultiSegmentation*>(segmentation);
    segmentation = &multi_segmentation->subsegmentation(cellID);    
    segmentation_type = segmentation->type();
  }
  return dd4hep::Segmentation(dd4hep::Handle<dd4hep::SegmentationObject>(segmentation));
}

} // namespace eicrecon
