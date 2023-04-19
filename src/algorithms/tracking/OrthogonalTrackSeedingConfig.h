// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <vector>

#include "Acts/Seeding/SeedFilterConfig.hpp"
#include "Acts/Seeding/SeedFinderOrthogonalConfig.hpp"
#include "Acts/Seeding/SeedFilter.hpp"

#include "SpacePoint.h"

namespace eicrecon {

  struct OrthogonalTrackSeedingConfig {
    float m_rMax = 500. * Acts::UnitConstants::mm;
    float m_rMin = 33. * Acts::UnitConstants::mm;
    float m_deltaRMinTopSP = 1. * Acts::UnitConstants::mm;
    float m_deltaRMaxTopSP = 400. * Acts::UnitConstants::mm;
    float m_deltaRMinBottomSP = 1. * Acts::UnitConstants::mm;
    float m_deltaRMaxBottomSP = 400. * Acts::UnitConstants::mm;
    float m_collisionRegionMin = -300 * Acts::UnitConstants::mm;
    float m_collisionRegionMax = 300 * Acts::UnitConstants::mm;
    float m_zMin = -800. * Acts::UnitConstants::mm;
    float m_zMax = 800. * Acts::UnitConstants::mm;

    /// max number of seeds a single middle sp can belong to
    float m_maxSeedsPerSpM = 1;
    float m_cotThetaMax = 16;
    float m_sigmaScattering = 5;
    float m_radLengthPerSeed = 0.1;
    float m_minPt = 100. * Acts::UnitConstants::MeV; // MeV
    float m_bFieldInZ = 1.7 * Acts::UnitConstants::T; // T in Acts units of GeV/(e*mm)
    float m_beamPosX = 0;
    float m_beamPosY = 0;

    /// Maximum transverse PCA allowed
    float m_impactMax = 20. * Acts::UnitConstants::mm;

    /// Middle spacepoint must fall between these two radii
    float m_rMinMiddle = 20. * Acts::UnitConstants::mm;
    float m_rMaxMiddle = 400. * Acts::UnitConstants::mm;

    Acts::SeedFilterConfig m_seedFilterConfig;
    Acts::SeedFinderOrthogonalConfig<SpacePoint> m_seedFinderConfig;

    void configure()
    {
      m_seedFilterConfig.maxSeedsPerSpM = m_maxSeedsPerSpM;

      m_seedFinderConfig.seedFilter = std::make_unique<Acts::SeedFilter<eicrecon::SpacePoint>>(Acts::SeedFilter<eicrecon::SpacePoint>(m_seedFilterConfig));
      m_seedFinderConfig.rMax = m_rMax;
      m_seedFinderConfig.deltaRMinTopSP = m_deltaRMinTopSP;
      m_seedFinderConfig.deltaRMaxTopSP = m_deltaRMaxTopSP;
      m_seedFinderConfig.deltaRMinBottomSP = m_deltaRMinBottomSP;
      m_seedFinderConfig.deltaRMaxBottomSP = m_deltaRMaxBottomSP;
      m_seedFinderConfig.collisionRegionMin = m_collisionRegionMin;
      m_seedFinderConfig.collisionRegionMax = m_collisionRegionMax;
      m_seedFinderConfig.zMin = m_zMin;
      m_seedFinderConfig.zMax = m_zMax;
      m_seedFinderConfig.maxSeedsPerSpM = m_maxSeedsPerSpM;
      m_seedFinderConfig.cotThetaMax = m_cotThetaMax;
      m_seedFinderConfig.sigmaScattering = m_sigmaScattering;
      m_seedFinderConfig.radLengthPerSeed = m_radLengthPerSeed;
      m_seedFinderConfig.minPt = m_minPt;
      m_seedFinderConfig.bFieldInZ = m_bFieldInZ;
      m_seedFinderConfig.beamPos =
	Acts::Vector2(m_beamPosX, m_beamPosY);
      m_seedFinderConfig.impactMax = m_impactMax;
      m_seedFinderConfig.rMinMiddle = m_rMinMiddle;
      m_seedFinderConfig.rMaxMiddle = m_rMaxMiddle;
      // Taken from SeedingOrthogonalAlgorithm.cpp, e.g.
      // calculation of scattering using the highland formula
      // convert pT to p once theta angle is known
      m_seedFinderConfig.highland =
	13.6 * std::sqrt(m_seedFinderConfig.radLengthPerSeed) *
	(1 + 0.038 * std::log(m_seedFinderConfig.radLengthPerSeed));
      float maxScatteringAngle =
	m_seedFinderConfig.highland / m_seedFinderConfig.minPt;
      m_seedFinderConfig.maxScatteringAngle2 =
	maxScatteringAngle * maxScatteringAngle;

      // Helix radius in homogeneous magnetic field
      // in ACTS Units are GeV, mm, and GeV/(e*mm)
      m_seedFinderConfig.pTPerHelixRadius = m_seedFinderConfig.bFieldInZ;
      m_seedFinderConfig.minHelixDiameter2 =
	std::pow(m_seedFinderConfig.minPt * 2 /
		 m_seedFinderConfig.pTPerHelixRadius,
		 2);

      m_seedFinderConfig.pT2perRadius =
	std::pow(
          m_seedFinderConfig.highland / m_seedFinderConfig.pTPerHelixRadius,
	  2);

    }

  };

}
