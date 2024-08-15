// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Wouter Deconinck

#include "algorithms/calorimetry/CalorimeterHitToTrackerHit.h"

#include <DD4hep/Alignments.h>
#include <DD4hep/IDDescriptor.h>
#include <DD4hep/Objects.h>
#include <DD4hep/Readout.h>
#include <DD4hep/Segmentations.h>
#include <DD4hep/Shapes.h>
#include <DD4hep/VolumeManager.h>
#include <DD4hep/Volumes.h>
#include <DD4hep/config.h>
#include <DDSegmentation/BitFieldCoder.h>
#include <Evaluator/DD4hepUnits.h>
#include <Math/GenVector/Cartesian3D.h>
#include <Math/GenVector/DisplacementVector3D.h>
#include <algorithm>
#include <algorithms/service.h>
#include <cctype>
#include <edm4eic/EDM4eicVersion.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <gsl/pointers>
#include <map>
#include <ostream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "services/evaluator/EvaluatorSvc.h"

using namespace dd4hep;

namespace eicrecon {

void CalorimeterHitToTrackerHit::init() {}

void CalorimeterHitToTrackerHit::process(const CalorimeterHitToTrackerHit::Input& input,
                                         const CalorimeterHitToTrackerHit::Output& output) const {

  const auto [calorimeter_hits] = input;
  auto [tracker_hits]           = output;

  for (const auto& calorimeter_hit : *calorimeter_hits) {

    edm4eic::CovDiag3f position_error;

    [[maybe_unused]] auto tracker_hit = tracker_hits->create(
        calorimeter_hit.getCellID(), calorimeter_hit.getPosition(), position_error,
        calorimeter_hit.getTime(), calorimeter_hit.getTimeError(), calorimeter_hit.getEnergy(),
        calorimeter_hit.getEnergyError());
  }
}

} // namespace eicrecon
