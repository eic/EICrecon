// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Wouter Deconinck

#include "algorithms/calorimetry/CalorimeterHitToTrackerHit.h"

#include <DD4hep/config.h>
#include <edm4eic/CovDiag3f.h>
#include <gsl/pointers>

using namespace dd4hep;

namespace eicrecon {

void CalorimeterHitToTrackerHit::init() {}

void CalorimeterHitToTrackerHit::process(const CalorimeterHitToTrackerHit::Input& input,
                                         const CalorimeterHitToTrackerHit::Output& output) const {

  const auto [calorimeter_hits] = input;
  auto [tracker_hits]           = output;

  for (const auto& calorimeter_hit : *calorimeter_hits) {

    // Get CellID
    auto cell_id{calorimeter_hit.getCellID()};

    // Determine detector element
    auto det_element = m_volume_manager.lookupDetElement(cell_id);

    // Cache position errors for detector elements
    static std::map<dd4hep::DetElement, edm4eic::CovDiag3f> position_error;
    if (position_error.count(det_element) == 0) {

      // Determine readout and segmentation
      auto readout      = m_converter->findReadout(det_element);
      auto segmentation = readout.segmentation();

      // Determine position uncertainty
      if (segmentation.type() == "CartesianGridXY") {
        auto cell_dimensions           = m_converter->cellDimensions(cell_id);
        position_error[det_element].xx = cell_dimensions[0] / std::sqrt(12);
        position_error[det_element].yy = cell_dimensions[1] / std::sqrt(12);
      } else {
        continue;
      }
    }

    // Create tracker hit
    tracker_hits->create(calorimeter_hit.getCellID(), calorimeter_hit.getPosition(),
                         position_error[det_element], calorimeter_hit.getTime(),
                         calorimeter_hit.getTimeError(), calorimeter_hit.getEnergy(),
                         calorimeter_hit.getEnergyError());
  }
}

} // namespace eicrecon
