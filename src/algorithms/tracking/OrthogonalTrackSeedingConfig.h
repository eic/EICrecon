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
    float m_rMax = 440. * Acts::UnitConstants::mm; // max r to look for hits to compose seeds
    float m_rMin = 33. * Acts::UnitConstants::mm; // min r to look for hits to compose seeds
    float m_zMax = 1700. * Acts::UnitConstants::mm; // max z to look for hits to compose seeds
    float m_zMin = -1500. * Acts::UnitConstants::mm; // min z to look for hits to compose seeds
    float m_deltaRMinTopSP = 10. * Acts::UnitConstants::mm; // Min distance in r between middle and top SP in one seed
    float m_deltaRMaxTopSP = 200. * Acts::UnitConstants::mm; // Max distance in r between middle and top SP in one seed
    float m_deltaRMinBottomSP = 10. * Acts::UnitConstants::mm; // Min distance in r between middle and bottom SP in one seed
    float m_deltaRMaxBottomSP = 200. * Acts::UnitConstants::mm; // Max distance in r between middle and bottom SP in one seed
    float m_collisionRegionMin = -250 * Acts::UnitConstants::mm; // Min z for primary vertex
    float m_collisionRegionMax = 250 * Acts::UnitConstants::mm; // Max z for primary vertex

    float m_maxSeedsPerSpM = 0; // max number of seeds a single middle sp can belong to - 1
    float m_cotThetaMax = 1.0 / tan(2. * atan(exp(-4.0))); // Cotangent of max theta angle (based on eta)

    float m_sigmaScattering = 5; // How many standard devs of scattering angles to consider
    float m_radLengthPerSeed = 0.1; // Average radiation lengths of material on the length of a seed
    float m_minPt = (100. * Acts::UnitConstants::MeV) / m_cotThetaMax; // MeV (in Acts units of GeV) - minimum transverse momentum
    float m_bFieldInZ = 1.7 * Acts::UnitConstants::T; // T (in Acts units of GeV/[e*mm]) - Magnetic field strength
    float m_beamPosX = 0; // x offset for beam position
    float m_beamPosY = 0; // y offset for beam position
    float m_impactMax = 3. * Acts::UnitConstants::mm; // Maximum transverse PCA allowed
    float m_bFieldMin = 0.1 * Acts::UnitConstants::T; // T (in Acts units of GeV/[e*mm]) - Minimum Magnetic field strength
    float m_rMinMiddle = 20. * Acts::UnitConstants::mm; // Middle spacepoint must fall between these two radii
    float m_rMaxMiddle = 400. * Acts::UnitConstants::mm;

    //////////////////////////////////////////////////////////////////////////
    /// SEED FILTER GENERAL PARAMETERS
    /// The parameters below control the process of filtering out seeds before
    /// sending them off to track reconstruction. These parameters first correspond
    /// to global settings (more loose) followed by more strict cuts for the central
    /// and forward/backward regions separately.

    float m_maxSeedsPerSpM_filter = 0; // max number of seeds a single middle sp can belong to - 1
    float m_deltaRMin = 5* Acts::UnitConstants::mm;
    bool m_seedConfirmation = false;
    float m_deltaInvHelixDiameter = 0.00003 * 1. / Acts::UnitConstants::mm;
    float m_impactWeightFactor = 1.;
    float m_zOriginWeightFactor = 1.;
    float m_compatSeedWeight = 200.;
    size_t m_compatSeedLimit = 2;
    bool m_curvatureSortingInFilter = false;
    float m_seedWeightIncrement = 0;

    ///////////////////////////////////////
    /// CENTRAL SEED FILTER PARAMETERS
    float  m_zMinSeedConf_cent = -250 * Acts::UnitConstants::mm;
    float  m_zMaxSeedConf_cent = 250 * Acts::UnitConstants::mm;
    float  m_rMaxSeedConf_cent = 140 * Acts::UnitConstants::mm;
    size_t m_nTopForLargeR_cent = 1;
    size_t m_nTopForSmallR_cent = 2;
    float  m_seedConfMinBottomRadius_cent = 60.0 * Acts::UnitConstants::mm;
    float  m_seedConfMaxZOrigin_cent = 150.0 * Acts::UnitConstants::mm;
    float  m_minImpactSeedConf_cent = 1.0 * Acts::UnitConstants::mm;

    ///////////////////////////////////////
    /// FORWARD / BACKWARD SEED FILTER PARAMETERS
    float  m_zMinSeedConf_forw = -3000 * Acts::UnitConstants::mm;
    float  m_zMaxSeedConf_forw = 3000 * Acts::UnitConstants::mm;
    float  m_rMaxSeedConf_forw = 140 * Acts::UnitConstants::mm;
    size_t m_nTopForLargeR_forw = 1;
    size_t m_nTopForSmallR_forw = 2;
    float  m_seedConfMinBottomRadius_forw = 60.0 * Acts::UnitConstants::mm;
    float  m_seedConfMaxZOrigin_forw = 150.0 * Acts::UnitConstants::mm;
    float  m_minImpactSeedConf_forw = 1.0 * Acts::UnitConstants::mm;

  };
}
