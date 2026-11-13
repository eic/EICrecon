// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Tom Bleher

#pragma once

#include <Acts/Definitions/Units.hpp>
#include <edm4eic/unit_system.h>

namespace eicrecon {

/// Configuration of B0TrackSeeding.
struct B0TrackSeedingConfig {
  /// Minimum momentum across the field, which also sets the scattering window of the Acts finders
  float minPt = 1. * Acts::UnitConstants::GeV;

  /// Minimum distance between the hits of a doublet, in r' of the frame of its middle hit. It is
  /// below the gaps between stations, so that a doublet spans two stations.
  float stationGap = 150. * Acts::UnitConstants::mm;

  /// Scale of the hit position errors in the slope cut of the Acts triplet finder. The cut allows
  /// several standard deviations of multiple scattering (sigmaScattering) but only one of the hit
  /// positions. Unscaled, it starts to reject triplets of genuine tracks above about 40 GeV, where
  /// scattering no longer covers the hit resolution, and half of them at 250 GeV.
  float hitErrorScale = 3.;

  /// Distance of the seed perigee upstream of its first hit, where the track finding starts
  double anchorDistance = 10. * Acts::UnitConstants::mm;

  // Seed prior, a loose diagonal envelope for the track finding, in EDM4eic units
  double positionError       = 0.1 * edm4eic::unit::mm;
  double angleError          = 2.e-3; // rad
  double qOverPRelativeError = 0.25;
  double timeError           = 10. * edm4eic::unit::ns;
};

} // namespace eicrecon
