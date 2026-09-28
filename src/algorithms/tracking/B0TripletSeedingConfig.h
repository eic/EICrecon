// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Tom Bleher

#pragma once

#include <Acts/Definitions/Units.hpp>
#include <cstddef>

namespace eicrecon {

struct B0TripletSeedingConfig {
  /// A z gap between consecutive hits larger than this starts a new station. It
  /// must exceed the z spread within a station (up to 12 mm) and stay below the
  /// station spacing (at least 270 mm).
  double stationGap = 50. * Acts::UnitConstants::mm;
  /// Maximum distance of the middle hit from the chord of the outer two,
  /// measured along the local magnetic field
  double maxResidual = 1. * Acts::UnitConstants::mm;
  /// Minimum momentum of the three-hit estimate
  double minMomentum = 1. * Acts::UnitConstants::GeV;
  /// Distance upstream of the first hit at which the seed is expressed
  double anchorDistance = 10. * Acts::UnitConstants::mm;
  /// Maximum number of seeds per event, smallest residual first
  std::size_t maxSeeds = 1000;

  /// Prior uncertainties of the seed parameters
  double positionError       = 0.1 * Acts::UnitConstants::mm;
  double angleError          = 2. * Acts::UnitConstants::mrad;
  double qOverPRelativeError = 0.25;
  double timeError           = 10. * Acts::UnitConstants::ns;
};

} // namespace eicrecon
