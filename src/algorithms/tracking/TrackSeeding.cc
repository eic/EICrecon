// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 - 2026, Joe Osborn, Dmitry Romanov, Wouter Deconinck

#include "TrackSeeding.h"

#include <Acts/Definitions/Algebra.hpp>
#include <Acts/Definitions/Units.hpp>
#include <Acts/Seeding/SeedConfirmationRangeConfig.hpp>
#include <Acts/Surfaces/PerigeeSurface.hpp>
#include <Acts/Surfaces/Surface.hpp>
#include <Acts/Utilities/Result.hpp>
#include <edm4eic/Cov6f.h>
#include <edm4eic/EDM4eicVersion.h>
#include <edm4hep/Vector2f.h>
#include <edm4hep/Vector3f.h>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <spdlog/common.h>
#include <array>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <tuple>

// Acts version-specific includes
#if TRACKSEEDING_HAS_SEEDING2
// Modern Seeding2 API includes
#include <Acts/Definitions/Direction.hpp>
#include <Acts/EventData/SeedContainer2.hpp>
#include <Acts/EventData/SpacePointColumns.hpp>
#include <Acts/EventData/SpacePointContainer2.hpp>
#include <Acts/EventData/Types.hpp>
#include <Acts/Geometry/Extent.hpp>
#include <Acts/Seeding2/BroadTripletSeedFilter.hpp>
#include <Acts/Seeding2/CylindricalSpacePointKDTree.hpp>
#include <Acts/Seeding2/DoubletSeedFinder.hpp>
#include <Acts/Seeding2/TripletSeedFinder.hpp>
#include <Acts/Seeding2/TripletSeeder.hpp>
#include <Acts/Utilities/AxisDefinitions.hpp>
#include <Acts/Utilities/Logger.hpp>
#include <Acts/Utilities/RangeXD.hpp>
#endif

#if TRACKSEEDING_HAS_ORTHOGONAL
// Orthogonal API includes
#include <Acts/EventData/Seed.hpp>
#include <Acts/EventData/SpacePointProxy.hpp>
#include <Acts/Seeding/SeedFilter.hpp>
#include <Acts/Seeding/SeedFilterConfig.hpp>
#include <Acts/Seeding/SeedFinderConfig.hpp>
#include <Acts/Seeding/SeedFinderOrthogonal.hpp>
#include <Acts/Seeding/SeedFinderOrthogonalConfig.hpp>
#include <Acts/Seeding/SeedFinderUtils.hpp>
#endif

#include "extensions/spdlog/SpdlogToActs.h"

namespace eicrecon {

void TrackSeeding::init() {
  // Step 1: Resolve Auto to specific method based on Acts version
  m_resolvedMethod = m_cfg.seedingMethod;
  if (m_resolvedMethod == TrackSeedingConfig::SeedingMethod::Auto) {
#if TRACKSEEDING_HAS_SEEDING2
    // Prefer Seeding2 (modern API) when available
    m_resolvedMethod = TrackSeedingConfig::SeedingMethod::Seeding2;
#elif TRACKSEEDING_HAS_ORTHOGONAL
    m_resolvedMethod = TrackSeedingConfig::SeedingMethod::Orthogonal;
#else
#error "No seeding method available - check Acts version compatibility"
#endif
  }

  // Step 2: Validate method availability
#if !TRACKSEEDING_HAS_SEEDING2
  if (m_resolvedMethod == TrackSeedingConfig::SeedingMethod::Seeding2) {
    throw std::runtime_error("TrackSeeding: Seeding2 method not available in Acts " +
                             std::to_string(Acts_VERSION_MAJOR) + "." +
                             std::to_string(Acts_VERSION_MINOR) +
                             " is installed. Use seedingMethod='auto' or 'orthogonal'.");
  }
#endif
#if !TRACKSEEDING_HAS_ORTHOGONAL
  if (m_resolvedMethod == TrackSeedingConfig::SeedingMethod::Orthogonal) {
    throw std::runtime_error("TrackSeeding: Orthogonal method not available in Acts " +
                             std::to_string(Acts_VERSION_MAJOR) + "." +
                             std::to_string(Acts_VERSION_MINOR) +
                             ". Use seedingMethod='auto' or 'seeding2'.");
  }
#endif

  // Step 3: Initialize based on resolved method
#if TRACKSEEDING_HAS_SEEDING2
  if (m_resolvedMethod == TrackSeedingConfig::SeedingMethod::Seeding2) {
    // Initialize Seeding2
#if TRACKSEEDING_HAS_SEEDING2 && TRACKSEEDING_HAS_ORTHOGONAL
    m_seedingData = Seeding2Data{};
    auto& data    = std::get<Seeding2Data>(m_seedingData);
#else
    auto& data = m_seedingData;
#endif

    data.actsLogger = Acts::getDefaultLogger(
        "TrackSeeding",
        eicrecon::SpdlogToActsLevel(static_cast<spdlog::level::level_enum>(this->level())));

    data.filterConfig.deltaInvHelixDiameter = m_cfg.deltaInvHelixDiameter;
    data.filterConfig.deltaRMin             = m_cfg.deltaRMin;
    data.filterConfig.compatSeedWeight      = m_cfg.compatSeedWeight;
    data.filterConfig.impactWeightFactor    = m_cfg.impactWeightFactor;
    data.filterConfig.zOriginWeightFactor   = m_cfg.zOriginWeightFactor;
    data.filterConfig.maxSeedsPerSpM        = m_cfg.maxSeedsPerSpM;
    data.filterConfig.compatSeedLimit       = m_cfg.compatSeedLimit;
    data.filterConfig.seedWeightIncrement   = m_cfg.seedWeightIncrement;
    data.filterConfig.seedConfirmation      = m_cfg.seedConfirmation;

    data.filterConfig.centralSeedConfirmationRange = Acts::SeedConfirmationRangeConfig{
        .zMinSeedConf            = m_cfg.zMinSeedConfCentral,
        .zMaxSeedConf            = m_cfg.zMaxSeedConfCentral,
        .rMaxSeedConf            = m_cfg.rMaxSeedConfCentral,
        .nTopForLargeR           = m_cfg.nTopForLargeRCentral,
        .nTopForSmallR           = m_cfg.nTopForSmallRCentral,
        .seedConfMinBottomRadius = m_cfg.seedConfMinBottomRadiusCentral,
        .seedConfMaxZOrigin      = m_cfg.seedConfMaxZOriginCentral,
        .minImpactSeedConf       = m_cfg.minImpactSeedConfCentral};

    data.filterConfig.forwardSeedConfirmationRange = Acts::SeedConfirmationRangeConfig{
        .zMinSeedConf            = m_cfg.zMinSeedConfForward,
        .zMaxSeedConf            = m_cfg.zMaxSeedConfForward,
        .rMaxSeedConf            = m_cfg.rMaxSeedConfForward,
        .nTopForLargeR           = m_cfg.nTopForLargeRForward,
        .nTopForSmallR           = m_cfg.nTopForSmallRForward,
        .seedConfMinBottomRadius = m_cfg.seedConfMinBottomRadiusForward,
        .seedConfMaxZOrigin      = m_cfg.seedConfMaxZOriginForward,
        .minImpactSeedConf       = m_cfg.minImpactSeedConfForward};

    data.seedFinder.emplace();

    // Configure bottom doublet finder
    Acts::DoubletSeedFinder::Config bottomDoubletFinderConfig;
    bottomDoubletFinderConfig.spacePointsSortedByRadius = false;
    bottomDoubletFinderConfig.candidateDirection        = Acts::Direction::Backward();
    bottomDoubletFinderConfig.deltaRMin                 = m_cfg.deltaRMinBottomSP;
    bottomDoubletFinderConfig.deltaRMax                 = m_cfg.deltaRMaxBottomSP;
    bottomDoubletFinderConfig.impactMax                 = m_cfg.impactMax;
    bottomDoubletFinderConfig.collisionRegionMin        = m_cfg.collisionRegionMin;
    bottomDoubletFinderConfig.collisionRegionMax        = m_cfg.collisionRegionMax;
    bottomDoubletFinderConfig.cotThetaMax               = m_cfg.cotThetaMax;
    bottomDoubletFinderConfig.minPt                     = m_cfg.minPt;
    data.bottomDoubletFinder = Acts::DoubletSeedFinder::create(
        Acts::DoubletSeedFinder::DerivedConfig(bottomDoubletFinderConfig, m_cfg.bFieldInZ));

    // Configure top doublet finder
    Acts::DoubletSeedFinder::Config topDoubletFinderConfig = bottomDoubletFinderConfig;
    topDoubletFinderConfig.candidateDirection              = Acts::Direction::Forward();
    topDoubletFinderConfig.deltaRMin                       = m_cfg.deltaRMinTopSP;
    topDoubletFinderConfig.deltaRMax                       = m_cfg.deltaRMaxTopSP;
    data.topDoubletFinder = Acts::DoubletSeedFinder::create(
        Acts::DoubletSeedFinder::DerivedConfig(topDoubletFinderConfig, m_cfg.bFieldInZ));

    // Configure triplet finder
    Acts::TripletSeedFinder::Config tripletFinderConfig;
    tripletFinderConfig.useStripInfo     = false;
    tripletFinderConfig.sortedByCotTheta = true;
    tripletFinderConfig.minPt            = m_cfg.minPt;
    tripletFinderConfig.sigmaScattering  = m_cfg.sigmaScattering;
    tripletFinderConfig.radLengthPerSeed = m_cfg.radLengthPerSeed;
    tripletFinderConfig.impactMax        = m_cfg.impactMax;
    data.tripletFinder = Acts::TripletSeedFinder::create(
        Acts::TripletSeedFinder::DerivedConfig(tripletFinderConfig, m_cfg.bFieldInZ));
  }
#endif

#if TRACKSEEDING_HAS_ORTHOGONAL
  if (m_resolvedMethod == TrackSeedingConfig::SeedingMethod::Orthogonal) {
    // Initialize Orthogonal
#if TRACKSEEDING_HAS_SEEDING2 && TRACKSEEDING_HAS_ORTHOGONAL
    m_seedingData = OrthogonalData{};
    auto& data    = std::get<OrthogonalData>(m_seedingData);
#else
    auto& data = m_seedingData;
#endif

    data.seedFilterConfig.maxSeedsPerSpM = m_cfg.maxSeedsPerSpM;
    // Original OrthogonalTrackSeedingConfig had filter deltaRMin = 5mm
    data.seedFilterConfig.deltaRMin             = 5. * Acts::UnitConstants::mm;
    data.seedFilterConfig.seedConfirmation      = m_cfg.seedConfirmation;
    data.seedFilterConfig.deltaInvHelixDiameter = m_cfg.deltaInvHelixDiameter;
    data.seedFilterConfig.impactWeightFactor    = m_cfg.impactWeightFactor;
    data.seedFilterConfig.zOriginWeightFactor   = m_cfg.zOriginWeightFactor;
    data.seedFilterConfig.compatSeedWeight      = m_cfg.compatSeedWeight;
    data.seedFilterConfig.compatSeedLimit       = m_cfg.compatSeedLimit;
    data.seedFilterConfig.seedWeightIncrement   = m_cfg.seedWeightIncrement;

    data.seedFilterConfig.centralSeedConfirmationRange = Acts::SeedConfirmationRangeConfig{
        .zMinSeedConf            = m_cfg.zMinSeedConfCentral,
        .zMaxSeedConf            = m_cfg.zMaxSeedConfCentral,
        .rMaxSeedConf            = m_cfg.rMaxSeedConfCentral,
        .nTopForLargeR           = m_cfg.nTopForLargeRCentral,
        .nTopForSmallR           = m_cfg.nTopForSmallRCentral,
        .seedConfMinBottomRadius = m_cfg.seedConfMinBottomRadiusCentral,
        .seedConfMaxZOrigin      = m_cfg.seedConfMaxZOriginCentral,
        .minImpactSeedConf       = m_cfg.minImpactSeedConfCentral};

    data.seedFilterConfig.forwardSeedConfirmationRange = Acts::SeedConfirmationRangeConfig{
        .zMinSeedConf            = m_cfg.zMinSeedConfForward,
        .zMaxSeedConf            = m_cfg.zMaxSeedConfForward,
        .rMaxSeedConf            = m_cfg.rMaxSeedConfForward,
        .nTopForLargeR           = m_cfg.nTopForLargeRForward,
        .nTopForSmallR           = m_cfg.nTopForSmallRForward,
        .seedConfMinBottomRadius = m_cfg.seedConfMinBottomRadiusForward,
        .seedConfMaxZOrigin      = m_cfg.seedConfMaxZOriginForward,
        .minImpactSeedConf       = m_cfg.minImpactSeedConfForward};

#if Acts_VERSION_MAJOR < 42
    data.seedFilterConfig = data.seedFilterConfig.toInternalUnits();
#endif

    data.seedFinderConfig.seedFilter =
        std::make_unique<Acts::SeedFilter<proxy_type>>(data.seedFilterConfig);
    data.seedFinderConfig.rMax               = m_cfg.rMax;
    data.seedFinderConfig.rMin               = m_cfg.rMin;
    data.seedFinderConfig.deltaRMinTopSP     = m_cfg.deltaRMinTopSP;
    data.seedFinderConfig.deltaRMaxTopSP     = m_cfg.deltaRMaxTopSP;
    data.seedFinderConfig.deltaRMinBottomSP  = m_cfg.deltaRMinBottomSP;
    data.seedFinderConfig.deltaRMaxBottomSP  = m_cfg.deltaRMaxBottomSP;
    data.seedFinderConfig.collisionRegionMin = m_cfg.collisionRegionMin;
    data.seedFinderConfig.collisionRegionMax = m_cfg.collisionRegionMax;
    data.seedFinderConfig.zMin               = m_cfg.zMin;
    data.seedFinderConfig.zMax               = m_cfg.zMax;
    data.seedFinderConfig.maxSeedsPerSpM     = m_cfg.maxSeedsPerSpM;
    data.seedFinderConfig.cotThetaMax        = m_cfg.cotThetaMax;
    data.seedFinderConfig.sigmaScattering    = m_cfg.sigmaScattering;
    data.seedFinderConfig.radLengthPerSeed   = m_cfg.radLengthPerSeed;
    data.seedFinderConfig.minPt              = m_cfg.minPt;
    data.seedFinderConfig.impactMax          = m_cfg.impactMax;
    data.seedFinderConfig.rMinMiddle         = m_cfg.rMinMiddle;
    data.seedFinderConfig.rMaxMiddle         = m_cfg.rMaxMiddle;
    data.seedFinderConfig.deltaPhiMax        = m_cfg.deltaPhiMax;

    data.seedFinderOptions.beamPos   = Acts::Vector2(m_cfg.beamPosX, m_cfg.beamPosY);
    data.seedFinderOptions.bFieldInZ = m_cfg.bFieldInZ;

    data.seedFinderConfig = data.seedFinderConfig
#if Acts_VERSION_MAJOR < 42
                                .toInternalUnits()
#endif
                                .calculateDerivedQuantities();
    data.seedFinderOptions = data.seedFinderOptions
#if Acts_VERSION_MAJOR < 42
                                 .toInternalUnits()
#endif
                                 .calculateDerivedQuantities(data.seedFinderConfig);
  }
#endif
}

void TrackSeeding::process(const Input& input, const Output& output) const {
  const auto [trk_hits]        = input;
  auto [trk_seeds, trk_params] = output;

#if TRACKSEEDING_HAS_SEEDING2
  if (m_resolvedMethod == TrackSeedingConfig::SeedingMethod::Seeding2) {
    // Get Seeding2 data from variant or direct member
#if TRACKSEEDING_HAS_SEEDING2 && TRACKSEEDING_HAS_ORTHOGONAL
    const auto& data = std::get<Seeding2Data>(m_seedingData);
#else
    const auto& data = m_seedingData;
#endif

    // ========== Seeding2 Processing ==========
    if (trk_hits->empty()) {
      return;
    }
    // Build SpacePointContainer2 from tracker hits
    Acts::SpacePointContainer2 spacePoints(
        Acts::SpacePointColumns::PackedXY | Acts::SpacePointColumns::PackedZR |
        Acts::SpacePointColumns::Phi | Acts::SpacePointColumns::VarianceZ |
        Acts::SpacePointColumns::VarianceR | Acts::SpacePointColumns::CopyFromIndex);
    spacePoints.reserve(trk_hits->size());

    Acts::Experimental::CylindricalSpacePointKDTreeBuilder kdTreeBuilder;
    kdTreeBuilder.reserve(trk_hits->size());

    Acts::Extent rRangeSPExtent;

    for (std::uint32_t i = 0; i < trk_hits->size(); ++i) {
      const auto hit = (*trk_hits)[i];
      const float hx = hit.getPosition()[0];
      const float hy = hit.getPosition()[1];
      const float hz = hit.getPosition()[2];
      const float hr = std::hypot(hx, hy);

      if (hr < m_cfg.rMin || hr > m_cfg.rMax || hz < m_cfg.zMin || hz > m_cfg.zMax) {
        continue;
      }

      const float varR =
          (hx * hx * hit.getPositionError().xx + hy * hy * hit.getPositionError().yy) /
          (hx * hx + hy * hy + std::numeric_limits<float>::epsilon());
      const float varZ = hit.getPositionError().zz;

      Acts::SpacePointIndex2 spIdx = spacePoints.size();
      auto sp                      = spacePoints.createSpacePoint();
      sp.xy()                      = {hx, hy};
      sp.zr()                      = {hz, hr};
      sp.phi()                     = std::atan2(hy, hx);
      sp.varianceZ()               = varZ;
      sp.varianceR()               = varR;
      sp.copyFromIndex()           = i;

      kdTreeBuilder.insert(spIdx, sp.phi(), hr, hz);
      rRangeSPExtent.extend({hx, hy, hz});
    }

    // Build KD-tree
    Acts::Experimental::CylindricalSpacePointKDTree kdTree = kdTreeBuilder.build();

    // Configure KD-tree options for bottom and top candidate searches
    Acts::Experimental::CylindricalSpacePointKDTree::Options bottomOptions;
    bottomOptions.rMax   = m_cfg.rMax;
    bottomOptions.zMin   = m_cfg.zMin;
    bottomOptions.zMax   = m_cfg.zMax;
    bottomOptions.phiMin = m_cfg.phiMin;
    bottomOptions.phiMax = m_cfg.phiMax;
    // Use bottom-specific deltaR parameters for bottom region
    bottomOptions.deltaRMin          = m_cfg.deltaRMinBottomSP;
    bottomOptions.deltaRMax          = m_cfg.deltaRMaxBottomSP;
    bottomOptions.collisionRegionMin = m_cfg.collisionRegionMin;
    bottomOptions.collisionRegionMax = m_cfg.collisionRegionMax;
    bottomOptions.cotThetaMax        = m_cfg.cotThetaMax;
    bottomOptions.deltaPhiMax        = m_cfg.deltaPhiMax;

    Acts::Experimental::CylindricalSpacePointKDTree::Options topOptions = bottomOptions;
    // Use top-specific deltaR parameters for top region
    topOptions.deltaRMin = m_cfg.deltaRMinTopSP;
    topOptions.deltaRMax = m_cfg.deltaRMaxTopSP;

    // Configure doublet finders
    // (constructed once in init() and stored in data)

    // Variable middle SP radial region of interest
    const Acts::Range1D<float> rMiddleSPRange(
        std::floor(rRangeSPExtent.min(Acts::AxisDirection::AxisR) / 2) * 2 +
            m_cfg.deltaRMiddleMinSPRange,
        std::floor(rRangeSPExtent.max(Acts::AxisDirection::AxisR) / 2) * 2 -
            m_cfg.deltaRMiddleMaxSPRange);

    // Setup seed filter
    Acts::BroadTripletSeedFilter::State filterState;
    Acts::BroadTripletSeedFilter::Cache filterCache;
    Acts::BroadTripletSeedFilter seedFilter(data.filterConfig, filterState, filterCache,
                                            *data.actsLogger);

    // Thread-local storage for candidates and cache
    thread_local Acts::TripletSeeder::Cache seederCache;
    thread_local Acts::Experimental::CylindricalSpacePointKDTree::Candidates candidates;

    // Output seed container
    Acts::SeedContainer2 actsSeeds;
    actsSeeds.assignSpacePointContainer(spacePoints);

    // Run the seeding algorithm by iterating over all points in the tree
    for (const auto& middle : kdTree) {
      const auto spM = spacePoints.at(middle.second).asConst();

      // Cut: Ensure middle space point lies within valid r-region
      const float rM = spM.zr()[1];
      if (m_cfg.useVariableMiddleSPRange) {
        if (rM < rMiddleSPRange.min() || rM > rMiddleSPRange.max()) {
          continue;
        }
      } else {
        if (rM > m_cfg.rMaxMiddle || rM < m_cfg.rMinMiddle) {
          continue;
        }
      }

      // Remove middle SPs outside phi and z region of interest
      const float zM = spM.zr()[0];
      if (zM < m_cfg.zOutermostLayers.first || zM > m_cfg.zOutermostLayers.second) {
        continue;
      }
      if (const float phiM = spM.phi(); phiM > m_cfg.phiMax || phiM < m_cfg.phiMin) {
        continue;
      }

      // Determine number of top seeds for confirmation
      std::size_t nTopSeedConf = 0;
      if (m_cfg.seedConfirmation) {
        // Choose central or forward region based on z of middle space point
        bool useCentralRegion =
            (zM <= m_cfg.zMaxSeedConfCentral && zM >= m_cfg.zMinSeedConfCentral);
        float rMaxSeedConf =
            useCentralRegion ? m_cfg.rMaxSeedConfCentral : m_cfg.rMaxSeedConfForward;
        std::size_t nTopForLargeR =
            useCentralRegion ? m_cfg.nTopForLargeRCentral : m_cfg.nTopForLargeRForward;
        std::size_t nTopForSmallR =
            useCentralRegion ? m_cfg.nTopForSmallRCentral : m_cfg.nTopForSmallRForward;
        nTopSeedConf = rM > rMaxSeedConf ? nTopForLargeR : nTopForSmallR;
      }

      // Find valid tuples of (bottom, middle, top) candidates
      candidates.clear();
      kdTree.validTuples(bottomOptions, topOptions, spM, nTopSeedConf, candidates);

      // Process bottom-low-high and top-low-high combinations
      Acts::SpacePointContainer2::ConstSubset bottomSps =
          spacePoints.subset(candidates.bottom_lh_v).asConst();
      Acts::SpacePointContainer2::ConstSubset topSps =
          spacePoints.subset(candidates.top_lh_v).asConst();
      data.seedFinder->createSeedsFromGroup(seederCache, *data.bottomDoubletFinder, *data.topDoubletFinder,
                                            *data.tripletFinder, seedFilter, spacePoints, bottomSps, spM,
                                            topSps, actsSeeds);

      bottomSps = spacePoints.subset(candidates.bottom_hl_v).asConst();
      topSps    = spacePoints.subset(candidates.top_hl_v).asConst();
      data.seedFinder->createSeedsFromGroup(seederCache, *data.bottomDoubletFinder, *data.topDoubletFinder,
                                            *data.tripletFinder, seedFilter, spacePoints, bottomSps, spM,
                                            topSps, actsSeeds);
    }

    debug("Created {} track seeds from {} space points", actsSeeds.size(), spacePoints.size());

    // Convert Acts seeds to EDM4eic format
    for (auto actsSeed : actsSeeds) {
      const auto& spIndices = actsSeed.spacePointIndices();

      // Get the three space points (bottom, middle, top)
      std::array<std::array<float, 3>, 3> positions;
      for (std::size_t k = 0; k < 3; ++k) {
        const std::uint32_t hitIdx = spacePoints.at(spIndices[k]).copyFromIndex();
        const auto hit             = (*trk_hits)[hitIdx];
        positions[k] = {hit.getPosition()[0], hit.getPosition()[1], hit.getPosition()[2]};
      }

      // Estimate track parameters from seed
      auto trackParams =
          estimateTrackParamsFromSeed(positions, actsSeed.vertexZ(), m_cfg.beamPosX, m_cfg.beamPosY,
                                      m_cfg.bFieldInZ, m_geoSvc, m_cfg);
      if (!trackParams.has_value()) {
        debug("Failed to estimate track parameters from seed");
        continue;
      }
      trk_params->push_back(trackParams.value());

      // Add seed to collection
      auto trk_seed = trk_seeds->create();
      trk_seed.setPerigee({0.F, 0.F, 0.F});
#if EDM4EIC_VERSION_MAJOR > 8 || (EDM4EIC_VERSION_MAJOR == 8 && EDM4EIC_VERSION_MINOR > 5)
      trk_seed.setQuality(actsSeed.quality());
#endif
      trk_seed.setParams(trackParams.value());
      for (std::size_t k = 0; k < 3; ++k) {
        const std::uint32_t hitIdx = spacePoints.at(spIndices[k]).copyFromIndex();
        trk_seed.addToHits((*trk_hits)[hitIdx]);
      }
    }
  }
#endif

#if TRACKSEEDING_HAS_ORTHOGONAL
  if (m_resolvedMethod == TrackSeedingConfig::SeedingMethod::Orthogonal) {
    // Get Orthogonal data from variant or direct member
#if TRACKSEEDING_HAS_SEEDING2 && TRACKSEEDING_HAS_ORTHOGONAL
    const auto& data = std::get<OrthogonalData>(m_seedingData);
#else
    const auto& data = m_seedingData;
#endif

    // ========== Orthogonal Processing ==========
    std::vector<const eicrecon::SpacePoint*> spacePoints = getSpacePoints(*trk_hits);

    Acts::SeedFinderOrthogonal<proxy_type> finder(data.seedFinderConfig);

    Acts::SpacePointContainerConfig spConfig;
    Acts::SpacePointContainerOptions spOptions;
    spOptions.beamPos = {m_cfg.beamPosX, m_cfg.beamPosY};

    SpacePointContainerType container(spacePoints);
    Acts::SpacePointContainer<decltype(container), Acts::detail::RefHolder> spContainer(
        spConfig, spOptions, container);

    std::vector<Acts::Seed<proxy_type>> seeds =
        finder.createSeeds(data.seedFinderOptions, spContainer);

    // need to convert here from seed of proxies to seed of sps
    for (const auto& seed : seeds) {
      const auto& sps = seed.sp();

      auto seedToAdd = Acts::Seed<eicrecon::SpacePoint>(*sps[0]->externalSpacePoint(),
                                                        *sps[1]->externalSpacePoint(),
                                                        *sps[2]->externalSpacePoint());
      seedToAdd.setVertexZ(seed.z());
      seedToAdd.setQuality(seed.seedQuality());

      // Estimate track parameters
      auto trackParams = estimateTrackParamsFromSeed(seedToAdd);
      if (!trackParams.has_value()) {
        debug("Failed to estimate track parameters from seed");
        continue;
      }
      trk_params->push_back(trackParams.value());

      // Add seed to collection
      auto trk_seed = trk_seeds->create();
      trk_seed.setPerigee({0.F, 0.F, 0.F});
#if EDM4EIC_VERSION_MAJOR > 8 || (EDM4EIC_VERSION_MAJOR == 8 && EDM4EIC_VERSION_MINOR > 5)
      trk_seed.setQuality(seedToAdd.seedQuality());
#endif
      trk_seed.setParams(trackParams.value());
      trk_seed.addToHits(*sps[0]->externalSpacePoint());
      trk_seed.addToHits(*sps[1]->externalSpacePoint());
      trk_seed.addToHits(*sps[2]->externalSpacePoint());
    }

    for (auto& sp : spacePoints) {
      delete sp;
    }
  }
#endif
}

// Shared core physics calculation for track parameter estimation
std::optional<edm4eic::MutableTrackParameters> TrackSeeding::computeTrackParametersFromFit(
    const std::vector<std::pair<float, float>>& xyPositions,
    const std::vector<std::pair<float, float>>& rzPositions, float vertexZ, float bFieldInZ,
    const std::shared_ptr<const ActsGeometryProvider>& geoSvc, const TrackSeedingConfig& cfg) {
  // Make mutable copies for fitting functions
  auto xyPosCopy = xyPositions;
  auto rzPosCopy = rzPositions;

  auto RX0Y0 = circleFit(xyPosCopy);
  float R    = std::get<0>(RX0Y0);
  float X0   = std::get<1>(RX0Y0);
  float Y0   = std::get<2>(RX0Y0);
  if (!(std::isfinite(R) && std::isfinite(std::abs(X0)) && std::isfinite(std::abs(Y0)))) {
    // avoid float overflow for hits on a line
    return {};
  }
  if (std::hypot(X0, Y0) < std::numeric_limits<decltype(std::hypot(X0, Y0))>::epsilon() ||
      !std::isfinite(std::hypot(X0, Y0))) {
    return {};
  }

  auto slopeZ0     = lineFit(rzPosCopy);
  const auto xypos = findPCA(RX0Y0);

  // Determine charge
  int charge = determineCharge(xyPosCopy, xypos, RX0Y0);

  float theta = std::atan(1.F / std::get<0>(slopeZ0));
  // normalize to 0<theta<pi
  if (theta < 0) {
    theta += static_cast<float>(M_PI);
  }
  float eta    = -std::log(std::tan(theta / 2.F));
  float pt     = R * bFieldInZ; // pt[GeV] = R[mm] * B[GeV/mm]
  float p      = pt * std::cosh(eta);
  float qOverP = static_cast<float>(charge) / p;

  // Calculate phi at xypos
  auto xpos  = xypos.first;
  auto ypos  = xypos.second;
  auto vxpos = -1. * charge * (ypos - Y0);
  auto vypos = charge * (xpos - X0);
  auto phi   = std::atan2(vypos, vxpos);

  auto perigee = Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3(0, 0, 0));
  Acts::Vector3 global(xypos.first, xypos.second, vertexZ);

  // Compute local position at PCA
  Acts::Vector3 direction(std::sin(theta) * std::cos(phi), std::sin(theta) * std::sin(phi),
                          std::cos(theta));
  auto local = perigee->globalToLocal(geoSvc->getActsGeometryContext(), global, direction);
  if (!local.ok()) {
    return {};
  }
  Acts::Vector2 localpos = local.value();

  auto trackparam = edm4eic::MutableTrackParameters();
  trackparam.setType(-1); // type --> seed(-1)
  trackparam.setLoc(
      {static_cast<float>(localpos(0)), static_cast<float>(localpos(1))}); // 2d location on surface
  trackparam.setPhi(static_cast<float>(phi));                              // phi [rad]
  trackparam.setTheta(theta);                                              // theta [rad]
  trackparam.setQOverP(qOverP);                                            // Q/p [e/GeV]
  trackparam.setTime(10);                                                  // time in ns
  edm4eic::Cov6f cov;
  cov(0, 0) = cfg.locaError / Acts::UnitConstants::mm;    // loc0
  cov(1, 1) = cfg.locbError / Acts::UnitConstants::mm;    // loc1
  cov(2, 2) = cfg.phiError / Acts::UnitConstants::rad;    // phi
  cov(3, 3) = cfg.thetaError / Acts::UnitConstants::rad;  // theta
  cov(4, 4) = cfg.qOverPError * Acts::UnitConstants::GeV; // qOverP
  cov(5, 5) = cfg.timeError / Acts::UnitConstants::ns;    // time
  trackparam.setCovariance(cov);
  return trackparam;
}

#if TRACKSEEDING_HAS_SEEDING2
std::optional<edm4eic::MutableTrackParameters> TrackSeeding::estimateTrackParamsFromSeed(
    const std::array<std::array<float, 3>, 3>& spPositions, float vertexZ,
    float beamPosX [[maybe_unused]], float beamPosY [[maybe_unused]], float bFieldInZ,
    const std::shared_ptr<const ActsGeometryProvider>& geoSvc, const TrackSeedingConfig& cfg) {
  // Note: beamPosX/beamPosY not currently used in track parameter estimation
  // but passed through for potential future improvements
  std::vector<std::pair<float, float>> xyPositions;
  std::vector<std::pair<float, float>> rzPositions;
  xyPositions.reserve(3);
  rzPositions.reserve(3);
  for (const auto& pos : spPositions) {
    xyPositions.emplace_back(pos[0], pos[1]);
    rzPositions.emplace_back(std::hypot(pos[0], pos[1]), pos[2]);
  }

  return computeTrackParametersFromFit(xyPositions, rzPositions, vertexZ, bFieldInZ, geoSvc, cfg);
}
#endif

#if TRACKSEEDING_HAS_ORTHOGONAL
std::vector<const eicrecon::SpacePoint*>
TrackSeeding::getSpacePoints(const edm4eic::TrackerHitCollection& trk_hits) {
  std::vector<const eicrecon::SpacePoint*> spacepoints;

  for (const auto hit : trk_hits) {
    const eicrecon::SpacePoint* sp = new SpacePoint(hit);
    spacepoints.push_back(sp);
  }

  return spacepoints;
}

std::optional<edm4eic::MutableTrackParameters>
TrackSeeding::estimateTrackParamsFromSeed(const Acts::Seed<SpacePoint>& seed) const {
  std::vector<std::pair<float, float>> xyHitPositions;
  std::vector<std::pair<float, float>> rzHitPositions;
  for (const auto& spptr : seed.sp()) {
    xyHitPositions.emplace_back(spptr->x(), spptr->y());
    rzHitPositions.emplace_back(spptr->r(), spptr->z());
  }

  return computeTrackParametersFromFit(xyHitPositions, rzHitPositions, seed.z(), m_cfg.bFieldInZ,
                                       m_geoSvc, m_cfg);
}
#endif

std::pair<float, float> TrackSeeding::findPCA(std::tuple<float, float, float>& circleParams) {
  const float R  = std::get<0>(circleParams);
  const float X0 = std::get<1>(circleParams);
  const float Y0 = std::get<2>(circleParams);

  // Calculate point on circle closest to origin
  const double R0 = std::hypot(X0, Y0);

  const double xmin = X0 * (1. - R / R0);
  const double ymin = Y0 * (1. - R / R0);

  return std::make_pair(xmin, ymin);
}

int TrackSeeding::determineCharge(std::vector<std::pair<float, float>>& positions,
                                  const std::pair<float, float>& PCA,
                                  std::tuple<float, float, float>& RX0Y0) {
  const auto& firstpos = positions.at(0);
  auto hit_x           = firstpos.first;
  auto hit_y           = firstpos.second;

  auto xpos = PCA.first;
  auto ypos = PCA.second;

  float X0 = std::get<1>(RX0Y0);
  float Y0 = std::get<2>(RX0Y0);

  Acts::Vector3 B_z(0, 0, 1);
  Acts::Vector3 radial(X0 - xpos, Y0 - ypos, 0);
  Acts::Vector3 hit(hit_x - xpos, hit_y - ypos, 0);

  auto cross = radial.cross(hit);

  float dot = cross.dot(B_z);

  return copysign(1., -dot);
}

/**
   * Circle fit to a given set of data points (in 2D)
   * This is an algebraic fit, due to Taubin, based on the journal article
   * G. Taubin, "Estimation Of Planar Curves, Surfaces And Nonplanar
   * Space Curves Defined By Implicit Equations, With
   * Applications To Edge And Range Image Segmentation",
   * IEEE Trans. PAMI, Vol. 13, pages 1115-1138, (1991)
   * It works well whether data points are sampled along an entire circle or along a small arc.
   * It still has a small bias and its statistical accuracy is slightly lower than that of the geometric fit (minimizing geometric distances),
   * It provides a very good initial guess for a subsequent geometric fit.
   * Nikolai Chernov  (September 2012)
   */
std::tuple<float, float, float>
TrackSeeding::circleFit(std::vector<std::pair<float, float>>& positions) {
  // Compute x- and y- sample means
  double meanX  = 0;
  double meanY  = 0;
  double weight = 0;

  for (const auto& [x, y] : positions) {
    meanX += x;
    meanY += y;
    ++weight;
  }
  meanX /= weight;
  meanY /= weight;

  //     computing moments

  double Mxx = 0;
  double Myy = 0;
  double Mxy = 0;
  double Mxz = 0;
  double Myz = 0;
  double Mzz = 0;

  for (auto& [x, y] : positions) {
    double Xi = x - meanX; //  centered x-coordinates
    double Yi = y - meanY; //  centered y-coordinates
    double Zi = std::pow(Xi, 2) + std::pow(Yi, 2);

    Mxy += Xi * Yi;
    Mxx += Xi * Xi;
    Myy += Yi * Yi;
    Mxz += Xi * Zi;
    Myz += Yi * Zi;
    Mzz += Zi * Zi;
  }
  Mxx /= weight;
  Myy /= weight;
  Mxy /= weight;
  Mxz /= weight;
  Myz /= weight;
  Mzz /= weight;

  //  computing coefficients of the characteristic polynomial
  const double Mz     = Mxx + Myy;
  const double Cov_xy = Mxx * Myy - Mxy * Mxy;
  const double Var_z  = Mzz - Mz * Mz;
  const double A3     = 4 * Mz;
  const double A2     = -3 * Mz * Mz - Mzz;
  const double A1     = Var_z * Mz + 4 * Cov_xy * Mz - Mxz * Mxz - Myz * Myz;
  const double A0  = Mxz * (Mxz * Myy - Myz * Mxy) + Myz * (Myz * Mxx - Mxz * Mxy) - Var_z * Cov_xy;
  const double A22 = A2 + A2;
  const double A33 = A3 + A3 + A3;

  //    finding the root of the characteristic polynomial
  //    using Newton's method starting at x=0
  //    (it is guaranteed to converge to the right root)
  static constexpr int iter_max = 99;
  double x                      = 0;
  double y                      = A0;

  // usually, 4-6 iterations are enough
  for (int iter = 0; iter < iter_max; ++iter) {
    const double Dy   = A1 + x * (A22 + A33 * x);
    const double xnew = x - y / Dy;
    if ((xnew == x) || (!std::isfinite(xnew))) {
      break;
    }

    const double ynew = A0 + xnew * (A1 + xnew * (A2 + xnew * A3));
    if (std::abs(ynew) >= std::abs(y)) {
      break;
    }

    x = xnew;
    y = ynew;
  }

  //  computing parameters of the fitting circle
  const double DET     = std::pow(x, 2) - x * Mz + Cov_xy;
  const double Xcenter = (Mxz * (Myy - x) - Myz * Mxy) / DET / 2;
  const double Ycenter = (Myz * (Mxx - x) - Mxz * Mxy) / DET / 2;

  float X0 = Xcenter + meanX;
  float Y0 = Ycenter + meanY;
  float R  = std::sqrt(std::pow(Xcenter, 2) + std::pow(Ycenter, 2) + Mz);

  //  assembling the output
  return std::make_tuple(R, X0, Y0);
}

std::tuple<float, float> TrackSeeding::lineFit(std::vector<std::pair<float, float>>& positions) {
  double xsum  = 0;
  double x2sum = 0;
  double ysum  = 0;
  double xysum = 0;
  for (const auto& [r, z] : positions) {
    xsum  = xsum + r;
    ysum  = ysum + z;
    x2sum = x2sum + std::pow(r, 2);
    xysum = xysum + static_cast<double>(r) * static_cast<double>(z);
  }

  const auto npts          = positions.size();
  const double denominator = (x2sum * npts - std::pow(xsum, 2));
  const float a            = (xysum * npts - xsum * ysum) / denominator;
  const float b            = (x2sum * ysum - xsum * xysum) / denominator;
  return std::make_tuple(a, b);
}

} // namespace eicrecon
