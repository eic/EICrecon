// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong, Sylvester Joosten, Wouter Deconinck, Dmitry Romanov

#pragma once

#include <edm4eic/unit_system.h>

namespace eicrecon {
struct TrackerHitReconstructionConfig {
  // Assigned as TrackerHit::timeError; should match the digitization time smearing
  double timeResolution = 10 * edm4eic::unit::ns;
  // if > 0, the readout frame length of the digitization: timeError is then
  // sqrt(timeResolution^2 + integrationTime^2 / 12), the frame adding a uniform spread
  double integrationTime = 0 * edm4eic::unit::ns;
};
} // namespace eicrecon
