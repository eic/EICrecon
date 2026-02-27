// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025, EICrecon Authors

#pragma once

#include <cstddef>
#include <limits>
#include <numbers>

#include <Acts/Definitions/Units.hpp>
#include <Acts/Seeding/SeedConfirmationRangeConfig.hpp>

namespace eicrecon {

/// Configuration for TrackSeeding2 using the Acts Seeding2 API
/// (available in Acts >= 45, using CylindricalSpacePointKDTree,
/// DoubletSeedFinder, TripletSeedFinder, BroadTripletSeedFilter).
struct TripletTrackSeedingConfig {

  //////////////////////////////////////////////////////////////////////////
  /// GEOMETRY / ACCEPTANCE PARAMETERS

  /// Maximum r of measurements used for seeding
  float rMax = 440. * Acts::UnitConstants::mm;
  /// Minimum r of measurements used for seeding
  float rMin = 33. * Acts::UnitConstants::mm;
  /// Maximum z of measurements used for seeding
  float zMax = 1700. * Acts::UnitConstants::mm;
  /// Minimum z of measurements used for seeding
  float zMin = -1500. * Acts::UnitConstants::mm;
  /// Minimum phi for seeding
  float phiMin = -std::numbers::pi_v<float>;
  /// Maximum phi for seeding
  float phiMax = std::numbers::pi_v<float>;

  //////////////////////////////////////////////////////////////////////////
  /// DOUBLET PARAMETERS (shared bottom/top by default; override per-side below)

  /// Minimum radial distance between doublet space points
  float deltaRMin = 10. * Acts::UnitConstants::mm;
  /// Maximum radial distance between doublet space points
  float deltaRMax = 450. * Acts::UnitConstants::mm;
  /// Minimum radial distance for top (outer) space point in doublet
  float deltaRMinTop = std::numeric_limits<float>::quiet_NaN();
  /// Maximum radial distance for top (outer) space point in doublet
  float deltaRMaxTop = std::numeric_limits<float>::quiet_NaN();
  /// Minimum radial distance for bottom (inner) space point in doublet
  float deltaRMinBottom = std::numeric_limits<float>::quiet_NaN();
  /// Maximum radial distance for bottom (inner) space point in doublet
  float deltaRMaxBottom = std::numeric_limits<float>::quiet_NaN();

  /// Minimum z-distance between doublet space points
  float deltaZMin = -std::numeric_limits<float>::infinity();
  /// Maximum z-distance between doublet space points
  float deltaZMax = std::numeric_limits<float>::infinity();

  /// Maximum impact parameter allowed for doublet candidates
  float impactMax = 3. * Acts::UnitConstants::mm;

  /// Enable interaction-point cut on doublet compatibility
  bool interactionPointCut = false;

  /// Minimum z of collision region
  float collisionRegionMin = -250. * Acts::UnitConstants::mm;
  /// Maximum z of collision region
  float collisionRegionMax = 250. * Acts::UnitConstants::mm;

  /// Maximum cotTheta for doublet candidates (equivalent to eta = 4)
  float cotThetaMax = 1.0 / tan(2. * atan(exp(-4.0)));

  /// Shrink the phi range for middle space points
  float deltaPhiMax = 0.085;

  /// Helix cut tolerance for doublet compatibility
  float helixCutTolerance = 1.0;

  //////////////////////////////////////////////////////////////////////////
  /// TRIPLET PARAMETERS

  /// Minimum pT for seeding (GeV)
  float minPt = (100. * Acts::UnitConstants::MeV) / (1.0 / tan(2. * atan(exp(-4.0))));
  /// Number of sigmas of scattering angle to consider
  float sigmaScattering = 5.0;
  /// Average radiation lengths per seed
  float radLengthPerSeed = 0.1;
  /// Tolerance parameter for strip module compatibility
  float toleranceParam = 1.1 * Acts::UnitConstants::mm;

  //////////////////////////////////////////////////////////////////////////
  /// MIDDLE SPACE POINT REGION

  /// Minimum r for middle space point
  float rMinMiddle = 20. * Acts::UnitConstants::mm;
  /// Maximum r for middle space point
  float rMaxMiddle = 400. * Acts::UnitConstants::mm;

  /// Use variable middle SP r range derived from SP extent
  bool useVariableMiddleSPRange = false;
  float deltaRMiddleMinSPRange  = 10. * Acts::UnitConstants::mm;
  float deltaRMiddleMaxSPRange  = 10. * Acts::UnitConstants::mm;

  /// Min/max z for middle space points
  std::pair<float, float> zOutermostLayers{-1500. * Acts::UnitConstants::mm,
                                           1700. * Acts::UnitConstants::mm};

  //////////////////////////////////////////////////////////////////////////
  /// SEED FILTER PARAMETERS

  float deltaInvHelixDiameter = 0.00003 * 1. / Acts::UnitConstants::mm;
  float compatSeedWeight      = 200.;
  float impactWeightFactor    = 1.;
  float zOriginWeightFactor   = 1.;
  unsigned int maxSeedsPerSpM = 0;
  std::size_t compatSeedLimit = 2;
  float seedWeightIncrement   = 0;

  /// Enable quality seed confirmation
  bool seedConfirmation = false;

  ///////////////////////////////////////
  /// CENTRAL SEED FILTER PARAMETERS
  float zMinSeedConfCentral            = -250. * Acts::UnitConstants::mm;
  float zMaxSeedConfCentral            = 250. * Acts::UnitConstants::mm;
  float rMaxSeedConfCentral            = 140. * Acts::UnitConstants::mm;
  std::size_t nTopForLargeRCentral     = 1;
  std::size_t nTopForSmallRCentral     = 2;
  float seedConfMinBottomRadiusCentral = 60.0 * Acts::UnitConstants::mm;
  float seedConfMaxZOriginCentral      = 150.0 * Acts::UnitConstants::mm;
  float minImpactSeedConfCentral       = 1.0 * Acts::UnitConstants::mm;

  ///////////////////////////////////////
  /// FORWARD / BACKWARD SEED FILTER PARAMETERS
  float zMinSeedConfForward            = -3000. * Acts::UnitConstants::mm;
  float zMaxSeedConfForward            = 3000. * Acts::UnitConstants::mm;
  float rMaxSeedConfForward            = 140. * Acts::UnitConstants::mm;
  std::size_t nTopForLargeRForward     = 1;
  std::size_t nTopForSmallRForward     = 2;
  float seedConfMinBottomRadiusForward = 60.0 * Acts::UnitConstants::mm;
  float seedConfMaxZOriginForward      = 150.0 * Acts::UnitConstants::mm;
  float minImpactSeedConfForward       = 1.0 * Acts::UnitConstants::mm;

  //////////////////////////////////////////////////////////////////////////
  /// PHYSICS / FIELD PARAMETERS

  /// Magnetic field in z (GeV/[e*mm] = T in Acts units)
  float bFieldInZ = 1.7 * Acts::UnitConstants::T;
  /// Beam position x offset
  float beamPosX = 0.;
  /// Beam position y offset
  float beamPosY = 0.;

  //////////////////////////////////////////////////////////////////////////
  /// TRACK PARAMETER ESTIMATION COVARIANCE

  float locaError   = 1.5 * Acts::UnitConstants::mm;
  float locbError   = 1.5 * Acts::UnitConstants::mm;
  float phiError    = 0.02 * Acts::UnitConstants::rad;
  float thetaError  = 0.002 * Acts::UnitConstants::rad;
  float qOverPError = 0.025 / Acts::UnitConstants::GeV;
  float timeError   = 0.1 * Acts::UnitConstants::mm;
};

} // namespace eicrecon
