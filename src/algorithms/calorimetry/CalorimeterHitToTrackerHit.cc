// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Wouter Deconinck

#include "algorithms/calorimetry/CalorimeterHitToTrackerHit.h"

#include <DD4hep/DetElement.h>
#include <DD4hep/Readout.h>
#include <DD4hep/Segmentations.h>
#include <edm4eic/CovDiag3f.h>
#include <cmath>
#include <gsl/pointers>
#include <map>
#include <set>
#include <vector>

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
    static thread_local std::map<dd4hep::DetElement, edm4eic::CovDiag3f> position_error;
    static thread_local std::set<dd4hep::DetElement> unsupported_segmentation;

    // Skip if we've already determined this detector element has unsupported segmentation
    if (unsupported_segmentation.contains(det_element)) {
      continue;
    }

    if (!position_error.contains(det_element)) {

      // Determine readout and segmentation
      auto readout      = m_converter->findReadout(det_element);
      auto segmentation = readout.segmentation();

      // Determine position uncertainty
      if (segmentation.type() == "CartesianGridXY") {
        auto cell_dimensions           = m_converter->cellDimensions(cell_id);
        const auto sigma_x             = cell_dimensions[0] / std::sqrt(12.0);
        const auto sigma_y             = cell_dimensions[1] / std::sqrt(12.0);
        position_error[det_element].xx = static_cast<float>(sigma_x * sigma_x);
        position_error[det_element].yy = static_cast<float>(sigma_y * sigma_y);
        position_error[det_element].zz = 0.0f;
      } else {
        warning("Skipping calorimeter hit with unsupported segmentation type '{}' for detector "
                "element '{}'. Only 'CartesianGridXY' is currently supported.",
                segmentation.type(), det_element.name());
        unsupported_segmentation.insert(det_element);
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
