// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 - 2026, Joe Osborn, Dmitry Romanov, Wouter Deconinck

#include "TrackSeeding.h"

#include <Acts/Definitions/Algebra.hpp>
#include <Acts/Definitions/TrackParametrization.hpp>
#include <Acts/Definitions/Units.hpp>
#if Acts_VERSION_MAJOR >= 46
#include <Acts/EventData/BoundTrackParameters.hpp>
#else
#include <Acts/EventData/TrackParameters.hpp>
#endif
#include <Acts/EventData/ParticleHypothesis.hpp>
#include <Acts/Geometry/GeometryContext.hpp>
#include <Acts/MagneticField/ConstantBField.hpp>
#include <Acts/MagneticField/MagneticFieldContext.hpp>
#include <Acts/MagneticField/MagneticFieldProvider.hpp>
#include <Acts/Propagator/ActorList.hpp>
#include <Acts/Propagator/PropagatorOptions.hpp>
#include <Acts/Propagator/PropagatorResult.hpp>
#if __has_include(<Acts/Propagator/DefaultActor.hpp>)
#include <Acts/Propagator/DefaultActor.hpp>
#else
#include <Acts/Propagator/StandardAborters.hpp>
#endif
#include <Acts/Seeding/EstimateTrackParamsFromSeed.hpp>
#include <Acts/Seeding/SeedConfirmationRangeConfig.hpp>
#include <Acts/Surfaces/PerigeeSurface.hpp>
#include <Acts/Surfaces/Surface.hpp>
#include <Acts/Utilities/Result.hpp>
#include <edm4eic/Cov6f.h>
#include <edm4eic/CovDiag3f.h>
#include <edm4eic/EDM4eicVersion.h>
#include <edm4eic/unit_system.h>
#include <edm4hep/Vector2f.h>
#include <edm4hep/Vector3f.h>
#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
#include <Eigen/Geometry>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <system_error>
#include <tuple>
#include <vector>

#include "extensions/spdlog/SpdlogToActs.h"

// Acts version-specific includes
#if TRACKSEEDING_HAS_SEEDING
// Modern Seeding API includes
#include <Acts/Definitions/Direction.hpp>
#include <Acts/EventData/SeedContainer.hpp>
#include <Acts/EventData/SeedProxy.hpp>
#include <Acts/EventData/SpacePointColumnProxy.hpp>
#include <Acts/EventData/SpacePointColumns.hpp>
#include <Acts/EventData/SpacePointContainer.hpp>
#include <Acts/EventData/SpacePointProxy.hpp>
#include <Acts/EventData/Types.hpp>
#include <Acts/Geometry/Extent.hpp>
#include <Acts/Seeding/BroadTripletSeedFilter.hpp>
#include <Acts/Seeding/CylindricalSpacePointKDTree.hpp>
#include <Acts/Seeding/DoubletSeedFinder.hpp>
#include <Acts/Seeding/TripletSeedFinder.hpp>
#include <Acts/Seeding/TripletSeeder.hpp>
#include <Acts/Seeding/detail/CandidatesForMiddleSp.hpp>
#include <Acts/Utilities/AxisDefinitions.hpp>
#include <Acts/Utilities/Logger.hpp>

namespace Acts {
using SeedContainer2                             = Acts::SeedContainer;
using SpacePointIndex2                           = Acts::SpacePointIndex;
template <bool read_only> using SeedProxy2       = Acts::SeedProxy<read_only>;
using SpacePointContainer2                       = Acts::SpacePointContainer;
template <bool read_only> using SpacePointProxy2 = Acts::SpacePointProxy<read_only>;
using ConstSpacePointProxy2                      = Acts::ConstSpacePointProxy;
using TripletCandidate2                          = Acts::TripletCandidate;
} // namespace Acts
#endif

#if TRACKSEEDING_HAS_SEEDING2
// Modern Seeding2 API includes
#include <Acts/Definitions/Direction.hpp>
#include <Acts/EventData/SeedContainer2.hpp>
#include <Acts/EventData/SeedProxy2.hpp>
#include <Acts/EventData/SpacePointColumnProxy2.hpp>
#include <Acts/EventData/SpacePointColumns.hpp>
#include <Acts/EventData/SpacePointContainer2.hpp>
#include <Acts/EventData/SpacePointProxy2.hpp>
#include <Acts/EventData/Types.hpp>
#include <Acts/Geometry/Extent.hpp>
#include <Acts/Seeding2/BroadTripletSeedFilter.hpp>
#include <Acts/Seeding2/CylindricalSpacePointKDTree.hpp>
#include <Acts/Seeding2/DoubletSeedFinder.hpp>
#include <Acts/Seeding2/TripletSeedFinder.hpp>
#include <Acts/Seeding2/TripletSeeder.hpp>
#include <Acts/Seeding2/detail/CandidatesForMiddleSp2.hpp>
#include <Acts/Utilities/AxisDefinitions.hpp>
#include <Acts/Utilities/Logger.hpp>
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

namespace eicrecon {

#if TRACKSEEDING_HAS_SEEDING2 || TRACKSEEDING_HAS_SEEDING
namespace {

  /// Triplet-level cut on the r-z alignment of the two seed segments.
  ///
  /// Rejects a (bottom, middle, top) triplet when the polar angles of the
  /// bottom->middle and middle->top segments, theta = atan2(dr, dz), differ by more
  /// than deltaThetaMax. This is the same quantity that Acts::SeedFinderOrthogonal
  /// cuts at a hard-coded 5 mrad (|tanLM - tanMT| > 0.005 in filterCandidates); the
  /// Seeding2 TripletSeedFinder has no equivalent and relies only on the
  /// pT-dependent multiple-scattering window, which is 4 to 13 times looser for
  /// pT < 2 GeV with the ePIC configuration.
  class SegmentDeltaThetaCuts final : public Acts::ITripletSeedCuts {
  public:
    explicit SegmentDeltaThetaCuts(float deltaThetaMax) : m_deltaThetaMax(deltaThetaMax) {}

    float seedWeight(const Acts::ConstSpacePointProxy2& /*bottom*/,
                     const Acts::ConstSpacePointProxy2& /*middle*/,
                     const Acts::ConstSpacePointProxy2& /*top*/) const override {
      return 0.F;
    }

    bool singleSeedCut(float /*weight*/, const Acts::ConstSpacePointProxy2& bottom,
                       const Acts::ConstSpacePointProxy2& middle,
                       const Acts::ConstSpacePointProxy2& top) const override {
      // zr() = {z, r}
      const float zB      = bottom.zr()[0];
      const float rB      = bottom.zr()[1];
      const float zM      = middle.zr()[0];
      const float rM      = middle.zr()[1];
      const float zT      = top.zr()[0];
      const float rT      = top.zr()[1];
      const float thetaBM = std::atan2(rM - rB, zM - zB);
      const float thetaMT = std::atan2(rT - rM, zT - zM);
      return std::abs(thetaBM - thetaMT) <= m_deltaThetaMax;
    }

    void cutPerMiddleSp(std::span<Acts::TripletCandidate2>& /*seedCandidates*/) const override {}

  private:
    float m_deltaThetaMax;
  };

} // namespace
#endif

void TrackSeeding::init() {
  // Step 1: Resolve Auto to specific method based on Acts version
  m_resolvedMethod = m_cfg.seedingMethod;
  if (m_resolvedMethod == TrackSeedingConfig::SeedingMethod::Auto) {
#if TRACKSEEDING_HAS_SEEDING2 || TRACKSEEDING_HAS_SEEDING
    // Prefer Seeding2 (modern API) when available
    m_resolvedMethod = TrackSeedingConfig::SeedingMethod::Seeding2;
#elif TRACKSEEDING_HAS_ORTHOGONAL
    m_resolvedMethod = TrackSeedingConfig::SeedingMethod::Orthogonal;
#else
#error "No seeding method available - check Acts version compatibility"
#endif
  }

  // Step 2: Validate method availability
#if !(TRACKSEEDING_HAS_SEEDING2 || TRACKSEEDING_HAS_SEEDING)
  if (m_resolvedMethod == TrackSeedingConfig::SeedingMethod::Seeding2) {
    throw std::runtime_error("TrackSeeding: Seeding2 method not available in Acts " +
                             std::to_string(Acts_VERSION_MAJOR) + "." +
                             std::to_string(Acts_VERSION_MINOR) +
                             ". Use seedingMethod='auto' or 'orthogonal'.");
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
#if TRACKSEEDING_HAS_SEEDING2 || TRACKSEEDING_HAS_SEEDING
  if (m_resolvedMethod == TrackSeedingConfig::SeedingMethod::Seeding2) {
    // Initialize Seeding2
#if TRACKSEEDING_HAS_SEEDING2 && TRACKSEEDING_HAS_ORTHOGONAL
    m_seedingData = SeedingData{};
    auto& data    = std::get<SeedingData>(m_seedingData);
#else
    auto& data = m_seedingData;
#endif

    // Create spdlog logger with matching log level for Acts
    auto spdlog_logger = spdlog::default_logger()->clone(std::string(this->name()));
    spdlog_logger->set_level(static_cast<spdlog::level::level_enum>(this->level()));

    data.actsLogger = eicrecon::getSpdlogLogger(std::string(this->name()), spdlog_logger);

    data.filterConfig.deltaInvHelixDiameter = m_cfg.deltaInvHelixDiameter;
    data.filterConfig.deltaRMin             = m_cfg.deltaRMin;
    data.filterConfig.compatSeedWeight      = m_cfg.compatSeedWeight;
    data.filterConfig.impactWeightFactor    = m_cfg.impactWeightFactor;
    data.filterConfig.zOriginWeightFactor   = m_cfg.zOriginWeightFactor;
    data.filterConfig.maxSeedsPerSpM        = m_cfg.maxSeedsPerSpM;
    data.filterConfig.compatSeedLimit       = m_cfg.compatSeedLimit;
    data.filterConfig.seedWeightIncrement   = m_cfg.seedWeightIncrement;
    data.filterConfig.seedConfirmation      = m_cfg.seedConfirmation;

    // Segment alignment cut in r-z (reproduces the Orthogonal seeder's 5 mrad cut)
    if (m_cfg.deltaThetaMax > 0) {
      data.filterConfig.experimentCuts =
          std::make_shared<SegmentDeltaThetaCuts>(m_cfg.deltaThetaMax);
      debug("Seeding2: applying seed segment alignment cut |dTheta| <= {} rad",
            m_cfg.deltaThetaMax);
    } else {
      debug("Seeding2: seed segment alignment cut disabled (deltaThetaMax <= 0)");
    }

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
    bottomDoubletFinderConfig.deltaZMin                 = m_cfg.deltaZMin;
    bottomDoubletFinderConfig.deltaZMax                 = m_cfg.deltaZMax;
    bottomDoubletFinderConfig.impactMax                 = m_cfg.impactMax;
    bottomDoubletFinderConfig.interactionPointCut       = m_cfg.interactionPointCut;
    bottomDoubletFinderConfig.collisionRegionMin        = m_cfg.collisionRegionMin;
    bottomDoubletFinderConfig.collisionRegionMax        = m_cfg.collisionRegionMax;
    bottomDoubletFinderConfig.cotThetaMax               = m_cfg.cotThetaMax;
    bottomDoubletFinderConfig.minPt                     = m_cfg.minPt;
    bottomDoubletFinderConfig.helixCutTolerance         = m_cfg.helixCutTolerance;
    data.bottomDoubletFinder                            = Acts::DoubletSeedFinder::create(
        Acts::DoubletSeedFinder::DerivedConfig(bottomDoubletFinderConfig, m_cfg.bFieldInZ));

    // Configure top doublet finder
    Acts::DoubletSeedFinder::Config topDoubletFinderConfig = bottomDoubletFinderConfig;
    topDoubletFinderConfig.candidateDirection              = Acts::Direction::Forward();
    topDoubletFinderConfig.deltaRMin                       = m_cfg.deltaRMinTopSP;
    topDoubletFinderConfig.deltaRMax                       = m_cfg.deltaRMaxTopSP;
    data.topDoubletFinder                                  = Acts::DoubletSeedFinder::create(
        Acts::DoubletSeedFinder::DerivedConfig(topDoubletFinderConfig, m_cfg.bFieldInZ));

    // Configure triplet finder
    Acts::TripletSeedFinder::Config tripletFinderConfig;
    tripletFinderConfig.useStripInfo      = false;
    tripletFinderConfig.sortedByCotTheta  = true;
    tripletFinderConfig.minPt             = m_cfg.minPt;
    tripletFinderConfig.sigmaScattering   = m_cfg.sigmaScattering;
    tripletFinderConfig.radLengthPerSeed  = m_cfg.radLengthPerSeed;
    tripletFinderConfig.impactMax         = m_cfg.impactMax;
    tripletFinderConfig.helixCutTolerance = m_cfg.helixCutTolerance;
    data.tripletFinder                    = Acts::TripletSeedFinder::create(
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

    // Create spdlog logger with matching log level for Acts
    auto spdlog_logger = spdlog::default_logger()->clone(std::string(this->name()));
    spdlog_logger->set_level(static_cast<spdlog::level::level_enum>(this->level()));

    data.actsLogger = eicrecon::getSpdlogLogger(std::string(this->name()), spdlog_logger);

    if (std::abs(m_cfg.deltaThetaMax - 0.005F) > 1e-6F) {
      warning("deltaThetaMax = {} has no effect with the Orthogonal seeder, which applies a "
              "hard-coded 5 mrad segment alignment cut",
              m_cfg.deltaThetaMax);
    }

    data.seedFilterConfig.maxSeedsPerSpM        = m_cfg.maxSeedsPerSpM;
    data.seedFilterConfig.deltaRMin             = m_cfg.deltaRMin;
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

    data.seedFinderConfig = data.seedFinderConfig.calculateDerivedQuantities();
    data.seedFinderOptions =
        data.seedFinderOptions.calculateDerivedQuantities(data.seedFinderConfig);
  }
#endif

  // Initialize vacuum propagator for track parameter estimation
  std::shared_ptr<const Acts::MagneticFieldProvider> fieldProvider =
      m_geoSvc ? m_geoSvc->getFieldProvider() : nullptr;
  if (!fieldProvider) {
    fieldProvider = std::make_shared<Acts::ConstantBField>(Acts::Vector3(0, 0, m_cfg.bFieldInZ));
  }
  m_propagator =
      std::make_unique<SeedPropagator>(SeedStepper(fieldProvider), Acts::VoidNavigator{});
}

void TrackSeeding::process(const Input& input, const Output& output) const {
  const auto [trk_hits]        = input;
  auto [trk_seeds, trk_params] = output;

#if TRACKSEEDING_HAS_SEEDING2 || TRACKSEEDING_HAS_SEEDING
  if (m_resolvedMethod == TrackSeedingConfig::SeedingMethod::Seeding2) {
    // Get Seeding2 data from variant or direct member
#if TRACKSEEDING_HAS_SEEDING2 && TRACKSEEDING_HAS_ORTHOGONAL
    const auto& data = std::get<SeedingData>(m_seedingData);
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
        Acts::SpacePointColumns::VarianceR |
#if TRACKSEEDING_HAS_SEEDING
        Acts::SpacePointColumns::CopiedFromIndex
#else
        Acts::SpacePointColumns::CopyFromIndex
#endif
    );
    spacePoints.reserve(trk_hits->size());

    Acts::Experimental::CylindricalSpacePointKDTreeBuilder kdTreeBuilder;
    kdTreeBuilder.reserve(trk_hits->size());

    Acts::Extent rRangeSPExtent;

    for (std::uint32_t i = 0; i < trk_hits->size(); ++i) {
      const auto& hit = (*trk_hits)[i];
      const float hx  = hit.getPosition()[0];
      const float hy  = hit.getPosition()[1];
      const float hz  = hit.getPosition()[2];
      const float hr  = std::hypot(hx, hy);

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
#if TRACKSEEDING_HAS_SEEDING
      sp.copiedFromIndex() = i;
#else
      sp.copyFromIndex() = i;
#endif

      kdTreeBuilder.insert(spIdx, sp.phi(), hr, hz);
      rRangeSPExtent.extend({hx, hy, hz});
    }
    if (spacePoints.empty()) {
      return;
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

    const bool useVariableMiddleSPRange = m_cfg.useVariableMiddleSPRange;
    float rMiddleSPMin                  = 0.F;
    float rMiddleSPMax                  = 0.F;
    if (useVariableMiddleSPRange) {
      rMiddleSPMin = std::floor(rRangeSPExtent.min(Acts::AxisDirection::AxisR) / 2) * 2 +
                     m_cfg.deltaRMiddleMinSPRange;
      rMiddleSPMax = std::floor(rRangeSPExtent.max(Acts::AxisDirection::AxisR) / 2) * 2 -
                     m_cfg.deltaRMiddleMaxSPRange;
    }

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
      if (useVariableMiddleSPRange) {
        if (rM < rMiddleSPMin || rM > rMiddleSPMax) {
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
      kdTree.validTuples(topOptions, bottomOptions, spM, nTopSeedConf, candidates);

      // Process bottom-low-high and top-low-high combinations
      Acts::SpacePointContainer2::ConstSubset bottomSps =
          spacePoints.subset(candidates.bottom_lh_v).asConst();
      Acts::SpacePointContainer2::ConstSubset topSps =
          spacePoints.subset(candidates.top_lh_v).asConst();
      data.seedFinder->createSeedsFromGroup(seederCache, *data.bottomDoubletFinder,
                                            *data.topDoubletFinder, *data.tripletFinder, seedFilter,
                                            spacePoints, bottomSps, spM, topSps, actsSeeds);

      bottomSps = spacePoints.subset(candidates.bottom_hl_v).asConst();
      topSps    = spacePoints.subset(candidates.top_hl_v).asConst();
      data.seedFinder->createSeedsFromGroup(seederCache, *data.bottomDoubletFinder,
                                            *data.topDoubletFinder, *data.tripletFinder, seedFilter,
                                            spacePoints, bottomSps, spM, topSps, actsSeeds);
    }

    debug("Created {} track seeds from {} space points", actsSeeds.size(), spacePoints.size());

    // Convert Acts seeds to EDM4eic format
    for (const auto& actsSeed : actsSeeds) {
      const auto& spIndices = actsSeed.spacePointIndices();

      // Get the three space points (bottom, middle, top)
      std::array<std::array<float, 3>, 3> positions{};
      double t0 = 0.0;
      for (std::size_t k = 0; k < 3; ++k) {
#if TRACKSEEDING_HAS_SEEDING
        const std::uint32_t hitIdx = spacePoints.at(spIndices[k]).copiedFromIndex();
#else
        const std::uint32_t hitIdx = spacePoints.at(spIndices[k]).copyFromIndex();
#endif
        const auto hit  = (*trk_hits)[hitIdx];
        positions.at(k) = {hit.getPosition()[0], hit.getPosition()[1], hit.getPosition()[2]};
        if (k == 0) {
          t0 = static_cast<double>(hit.getTime()) * Acts::UnitConstants::ns;
        }
      }

      // Estimate track parameters from seed
      auto trackParams = estimateTrackParamsFromSeed(positions, t0);
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
#if TRACKSEEDING_HAS_SEEDING
        const std::uint32_t hitIdx = spacePoints.at(spIndices[k]).copiedFromIndex();
#else
        const std::uint32_t hitIdx = spacePoints.at(spIndices[k]).copyFromIndex();
#endif
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

    Acts::SeedFinderOrthogonal<proxy_type> finder(data.seedFinderConfig, data.actsLogger->clone());

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

std::optional<edm4eic::MutableTrackParameters>
TrackSeeding::estimateTrackParams(const Acts::Vector3& sp0, const Acts::Vector3& sp1,
                                  const Acts::Vector3& sp2, double t0) const {
  if (!m_propagator) {
    return {};
  }

#if Acts_VERSION_MAJOR >= 45
  Acts::GeometryContext gctx = m_geoSvc ? m_geoSvc->getActsGeometryContext()
                                        : Acts::GeometryContext::dangerouslyDefaultConstruct();
#else
  Acts::GeometryContext gctx =
      m_geoSvc ? m_geoSvc->getActsGeometryContext() : Acts::GeometryContext();
#endif
  Acts::MagneticFieldContext mctx =
      m_geoSvc ? m_geoSvc->getActsMagneticFieldContext() : Acts::MagneticFieldContext();

  // Determine magnetic field at sp0
  Acts::Vector3 bField(0, 0, m_cfg.bFieldInZ);
  if (m_geoSvc) {
    if (auto fieldProvider = m_geoSvc->getFieldProvider()) {
      Acts::MagneticFieldProvider::Cache fieldCache = fieldProvider->makeCache(mctx);
      auto fieldRes                                 = fieldProvider->getField(sp0, fieldCache);
      if (fieldRes.ok()) {
        bField = *fieldRes;
      }
    }
  }

  if (bField.norm() <= 0) {
    return {};
  }

  // Estimate free track parameters at sp0 using upstream Acts
#if Acts_VERSION_MAJOR >= 46 || Acts_VERSION_MAJOR == 45 && Acts_VERSION_MINOR >= 2
  Acts::FreeVector freeParams = Acts::estimateTrackParamsFromSeed(sp0, t0, sp1, sp2, bField);
#else
  Acts::FreeVector freeParams = Acts::estimateTrackParamsFromSeed(sp0, sp1, sp2, bField);
  freeParams[Acts::eFreeTime] = t0;
#endif

  if (!freeParams.allFinite() || freeParams[Acts::eFreeQOverP] == 0) {
    return {};
  }

  // Create curvilinear track parameters at sp0
  Acts::Vector3 direction = freeParams.segment<3>(Acts::eFreeDir0).normalized();
  auto startParams        = Acts::BoundTrackParameters::createCurvilinear(
      freeParams.segment<4>(Acts::eFreePos0), direction, freeParams[Acts::eFreeQOverP],
      std::nullopt, Acts::ParticleHypothesis::pion());

  // Target perigee surface at the origin
  auto perigeeSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3(0, 0, 0));

  // Propagate backward to the perigee surface.
  // ForcedSurfaceReached allows convergence to the line surface without
  // rejecting intersections that slightly overstep during the Runge-Kutta integration.
  SeedPropagator::Options options(gctx, mctx);
  options.direction = Acts::Direction::Backward();

  auto result = m_propagator->template propagate<decltype(options), Acts::ForcedSurfaceReached>(
      startParams, *perigeeSurface, options);
  if (!result.ok()) {
    debug("Propagation to perigee surface failed: {}", result.error().message());
    return {};
  }

  const auto& endParams   = result->endParameters.value();
  const auto& boundValues = endParams.parameters();

  auto loc0 = static_cast<float>(boundValues[Acts::eBoundLoc0] / Acts::UnitConstants::mm *
                                 edm4eic::unit::mm); // d0 [mm]
  auto loc1 = static_cast<float>(boundValues[Acts::eBoundLoc1] / Acts::UnitConstants::mm *
                                 edm4eic::unit::mm); // z0 [mm]
  auto phi =
      static_cast<float>(boundValues[Acts::eBoundPhi] / Acts::UnitConstants::rad); // phi [rad]
  auto theta =
      static_cast<float>(boundValues[Acts::eBoundTheta] / Acts::UnitConstants::rad); // theta [rad]
  auto qOverP = static_cast<float>(boundValues[Acts::eBoundQOverP] * Acts::UnitConstants::GeV /
                                   edm4eic::unit::GeV); // Q/p [e/GeV]
  auto time   = static_cast<float>(boundValues[Acts::eBoundTime] / Acts::UnitConstants::ns *
                                   edm4eic::unit::ns); // time [ns]

  if (!std::isfinite(loc0) || !std::isfinite(loc1) || !std::isfinite(phi) ||
      !std::isfinite(theta) || !std::isfinite(qOverP)) {
    return {};
  }

  auto trackparam = edm4eic::MutableTrackParameters();
  trackparam.setType(-1); // type --> seed(-1)
  trackparam.setLoc({loc0, loc1});
  trackparam.setPhi(phi);
  trackparam.setTheta(theta);
  trackparam.setQOverP(qOverP);
  trackparam.setTime(time);

  edm4eic::Cov6f cov;
  cov(0, 0) = std::pow(m_cfg.locaError / Acts::UnitConstants::mm * edm4eic::unit::mm,
                       2); // loc0 variance in mm^2
  cov(1, 1) = std::pow(m_cfg.locbError / Acts::UnitConstants::mm * edm4eic::unit::mm,
                       2); // loc1 variance in mm^2
  cov(2, 2) = std::pow(m_cfg.phiError / Acts::UnitConstants::rad,
                       2); // phi variance in rad^2
  cov(3, 3) = std::pow(m_cfg.thetaError / Acts::UnitConstants::rad,
                       2); // theta variance in rad^2
  cov(4, 4) = std::pow(m_cfg.qOverPError * Acts::UnitConstants::GeV / edm4eic::unit::GeV,
                       2); // qOverP variance in (e/GeV)^2
  cov(5, 5) = std::pow(m_cfg.timeError / Acts::UnitConstants::ns * edm4eic::unit::ns,
                       2); // time variance in ns^2
  trackparam.setCovariance(cov);

  return trackparam;
}

#if TRACKSEEDING_HAS_SEEDING2 || TRACKSEEDING_HAS_SEEDING
std::optional<edm4eic::MutableTrackParameters>
TrackSeeding::estimateTrackParamsFromSeed(const std::array<std::array<float, 3>, 3>& spPositions,
                                          double t0) const {
  Acts::Vector3 sp0(spPositions[0][0], spPositions[0][1], spPositions[0][2]);
  Acts::Vector3 sp1(spPositions[1][0], spPositions[1][1], spPositions[1][2]);
  Acts::Vector3 sp2(spPositions[2][0], spPositions[2][1], spPositions[2][2]);
  return estimateTrackParams(sp0, sp1, sp2, t0);
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
  const auto& sps = seed.sp();
  Acts::Vector3 sp0(sps[0]->x(), sps[0]->y(), sps[0]->z());
  sp0 *= Acts::UnitConstants::mm / edm4eic::unit::mm;
  Acts::Vector3 sp1(sps[1]->x(), sps[1]->y(), sps[1]->z());
  sp1 *= Acts::UnitConstants::mm / edm4eic::unit::mm;
  Acts::Vector3 sp2(sps[2]->x(), sps[2]->y(), sps[2]->z());
  sp2 *= Acts::UnitConstants::mm / edm4eic::unit::mm;
  double t0 = static_cast<double>(sps[0]->t()) / edm4eic::unit::ns * Acts::UnitConstants::ns;
  return estimateTrackParams(sp0, sp1, sp2, t0);
}
#endif

} // namespace eicrecon
