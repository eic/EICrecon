// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck

#include "TrackerHitLocalFromMeasurements.h"

#include <Acts/Definitions/TrackParametrization.hpp>
#include <Acts/Definitions/Units.hpp>
#include <ActsPodioEdm/MutableTrackerHitLocal.h>
#include <edm4eic/Cov3f.h>
#include <edm4eic/unit_system.h>
#include <edm4hep/Vector2f.h>
#include <array>
#include <cstdint>
#include <gsl/pointers>

namespace eicrecon {

void TrackerHitLocalFromMeasurements::process(const Input& input, const Output& output) const {
  const auto [meas2Ds] = input;
  auto [hits]          = output;

  constexpr std::array<std::uint8_t, 2> indices{static_cast<std::uint8_t>(Acts::eBoundLoc0),
                                                static_cast<std::uint8_t>(Acts::eBoundLoc1)};
  constexpr auto mm  = Acts::UnitConstants::mm / edm4eic::unit::mm;
  constexpr auto mm2 = mm * mm;

  for (const auto& meas2D : *meas2Ds) {
    auto hit = hits->create();
    hit.setCellID(meas2D.getSurface());
    hit.setTime(meas2D.getTime() / edm4eic::unit::ns); // TrackerHitLocal time is in ns
    hit.setSubspaceIndices(indices);
    hit.setValue(meas2D.getLoc().a * mm, 0);
    hit.setValue(meas2D.getLoc().b * mm, 1);
    hit.setCov(meas2D.getCovariance().xx * mm2, 0, 0);
    hit.setCov(meas2D.getCovariance().yy * mm2, 1, 1);
    hit.setCov(meas2D.getCovariance().xy * mm2, 0, 1);
    hit.setCov(meas2D.getCovariance().xy * mm2, 1, 0);
  }
}

} // namespace eicrecon
