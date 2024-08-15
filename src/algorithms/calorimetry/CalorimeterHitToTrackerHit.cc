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

    edm4eic::CovDiag3f position_error;

    [[maybe_unused]] auto tracker_hit = tracker_hits->create(
        calorimeter_hit.getCellID(), calorimeter_hit.getPosition(), position_error,
        calorimeter_hit.getTime(), calorimeter_hit.getTimeError(), calorimeter_hit.getEnergy(),
        calorimeter_hit.getEnergyError());
  }
}

} // namespace eicrecon
