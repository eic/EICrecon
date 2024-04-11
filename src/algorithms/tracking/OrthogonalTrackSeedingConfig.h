// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <cstddef>

#include <Acts/Definitions/Units.hpp>

namespace eicrecon {

  struct OrthogonalTrackSeedingConfig {

    //////////////////////////////////////////////////////////////////////////
    /// SEED FINDER GENERAL PARAMETERS
    float rMax = 440. * Acts::UnitConstants::mm; // max r to look for hits to compose seeds
    float rMin = 33. * Acts::UnitConstants::mm; // min r to look for hits to compose seeds
    float zMax = 1700. * Acts::UnitConstants::mm; // max z to look for hits to compose seeds
    float zMin = -1500. * Acts::UnitConstants::mm; // min z to look for hits to compose seeds
    float deltaRMinTopSP = 10. * Acts::UnitConstants::mm; // Min distance in r between middle and top SP in one seed
    float deltaRMaxTopSP = 200. * Acts::UnitConstants::mm; // Max distance in r between middle and top SP in one seed
    float deltaRMinBottomSP = 10. * Acts::UnitConstants::mm; // Min distance in r between middle and bottom SP in one seed
    float deltaRMaxBottomSP = 200. * Acts::UnitConstants::mm; // Max distance in r between middle and bottom SP in one seed
    float collisionRegionMin = -250 * Acts::UnitConstants::mm; // Min z for primary vertex
    float collisionRegionMax = 250 * Acts::UnitConstants::mm; // Max z for primary vertex

    unsigned int maxSeedsPerSpM = 0; // max number of seeds a single middle sp can belong to - 1
    float cotThetaMax = 1.0 / tan(2. * atan(exp(-4.0))); // Cotangent of max theta angle (based on eta)

    float sigmaScattering = 5; // How many standard devs of scattering angles to consider
    float radLengthPerSeed = 0.1; // Average radiation lengths of material on the length of a seed
    float minPt = (100. * Acts::UnitConstants::MeV) / cotThetaMax; // MeV (in Acts units of GeV) - minimum transverse momentum
    float bFieldInZ = 1.7 * Acts::UnitConstants::T; // T (in Acts units of GeV/[e*mm]) - Magnetic field strength
    float beamPosX = 0; // x offset for beam position
    float beamPosY = 0; // y offset for beam position
    float impactMax = 3. * Acts::UnitConstants::mm; // Maximum transverse PCA allowed
    float bFieldMin = 0.1 * Acts::UnitConstants::T; // T (in Acts units of GeV/[e*mm]) - Minimum Magnetic field strength
    float rMinMiddle = 20. * Acts::UnitConstants::mm; // Middle spacepoint must fall between these two radii
    float rMaxMiddle = 400. * Acts::UnitConstants::mm;

    //////////////////////////////////////////////////////////////////////////
    /// SEED FILTER GENERAL PARAMETERS
    /// The parameters below control the process of filtering out seeds before
    /// sending them off to track reconstruction. These parameters first correspond
    /// to global settings (more loose) followed by more strict cuts for the central
    /// and forward/backward regions separately.

    float maxSeedsPerSpM_filter = 0; // max number of seeds a single middle sp can belong to - 1
    float deltaRMin = 5* Acts::UnitConstants::mm;
    bool seedConfirmation = false;
    float deltaInvHelixDiameter = 0.00003 * 1. / Acts::UnitConstants::mm;
    float impactWeightFactor = 1.;
    float zOriginWeightFactor = 1.;
    float compatSeedWeight = 200.;
    size_t compatSeedLimit = 2;
    float seedWeightIncrement = 0;

    ///////////////////////////////////////
    /// CENTRAL SEED FILTER PARAMETERS
    float  zMinSeedConf_cent = -250 * Acts::UnitConstants::mm;
    float  zMaxSeedConf_cent = 250 * Acts::UnitConstants::mm;
    float  rMaxSeedConf_cent = 140 * Acts::UnitConstants::mm;
    size_t nTopForLargeR_cent = 1;
    size_t nTopForSmallR_cent = 2;
    float  seedConfMinBottomRadius_cent = 60.0 * Acts::UnitConstants::mm;
    float  seedConfMaxZOrigin_cent = 150.0 * Acts::UnitConstants::mm;
    float  minImpactSeedConf_cent = 1.0 * Acts::UnitConstants::mm;

    ///////////////////////////////////////
    /// FORWARD / BACKWARD SEED FILTER PARAMETERS
    float  zMinSeedConf_forw = -3000 * Acts::UnitConstants::mm;
    float  zMaxSeedConf_forw = 3000 * Acts::UnitConstants::mm;
    float  rMaxSeedConf_forw = 140 * Acts::UnitConstants::mm;
    size_t nTopForLargeR_forw = 1;
    size_t nTopForSmallR_forw = 2;
    float  seedConfMinBottomRadius_forw = 60.0 * Acts::UnitConstants::mm;
    float  seedConfMaxZOrigin_forw = 150.0 * Acts::UnitConstants::mm;
    float  minImpactSeedConf_forw = 1.0 * Acts::UnitConstants::mm;

    //////////////////////////////////////
    ///Seed Covariance Error Matrix
    float loc_a_Error = 1.5 * Acts::UnitConstants::mm;     //Error on Loc a
    float loc_b_Error = 1.5 * Acts::UnitConstants::mm;     //Error on Loc b
    float phi_Error = 0.02 * Acts::UnitConstants::rad;     //Error on phi
    float theta_Error = 0.002 * Acts::UnitConstants::rad;  //Error on theta
    float qOverP_Error = 0.025 / Acts::UnitConstants::GeV; //Error on q over p
    float time_Error = 0.1 * Acts::UnitConstants::mm;      //Error on time
    // Note: Acts native time units are mm: https://acts.readthedocs.io/en/latest/core/definitions/units.html
  };
}
