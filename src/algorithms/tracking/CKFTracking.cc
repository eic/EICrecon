// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2025 Whitney Armstrong, Wouter Deconinck, Dmitry Romanov, Shujie Li, Dmitry Kalinkin

#include "CKFTracking.h"

#include <Acts/Definitions/Algebra.hpp>
#include <Acts/Definitions/TrackParametrization.hpp>
#include <Acts/Definitions/Units.hpp>
#include <Acts/Utilities/MathHelpers.hpp>
#if Acts_VERSION_MAJOR >= 46
#else
#include <Acts/EventData/GenericBoundTrackParameters.hpp>
#endif
#include <Acts/EventData/MeasurementHelpers.hpp>
#include <Acts/EventData/ParticleHypothesis.hpp>
#include <Acts/EventData/ProxyAccessor.hpp>
#include <Acts/EventData/SourceLink.hpp>
#include <Acts/EventData/TrackContainer.hpp>
#include <Acts/EventData/TrackProxy.hpp>
#include <Acts/EventData/TrackStatePropMask.hpp>
#include <Acts/EventData/VectorMultiTrajectory.hpp>
#include <Acts/EventData/VectorTrackContainer.hpp>
#include <Acts/Geometry/GeometryContext.hpp>
#include <Acts/Geometry/GeometryHierarchyMap.hpp>
#include <Acts/Geometry/GeometryIdentifier.hpp>
#include <Acts/Propagator/ActorList.hpp>
#include <Acts/Propagator/EigenStepper.hpp>
#include <Acts/Propagator/MaterialInteractor.hpp>
#include <Acts/Propagator/Navigator.hpp>
#include <Acts/Propagator/Propagator.hpp>
#include <Acts/Propagator/PropagatorOptions.hpp>
#include <Acts/Propagator/StandardAborters.hpp>
#include <Acts/Surfaces/PerigeeSurface.hpp>
#include <Acts/Surfaces/Surface.hpp>
#include <Acts/TrackFinding/CombinatorialKalmanFilterExtensions.hpp>
#include <Acts/TrackFinding/TrackStateCreator.hpp>
#include <Acts/TrackFitting/GainMatrixUpdater.hpp>
#include <Acts/Utilities/CalibrationContext.hpp>
#include <Acts/Utilities/Logger.hpp>
#include <Acts/Utilities/TrackHelpers.hpp>
#include <ActsExamples/EventData/GeometryContainers.hpp>
#include <ActsExamples/EventData/IndexSourceLink.hpp>
#include <ActsExamples/EventData/Track.hpp>
#include <boost/container/vector.hpp>
#include <edm4eic/Cov3f.h>
#include <edm4eic/Cov6f.h>
#include <edm4eic/Measurement2DCollection.h>
#include <edm4eic/TrackParametersCollection.h>
#include <edm4eic/TrackSeedCollection.h>
#include <edm4eic/unit_system.h>
#include <edm4hep/Vector2f.h>
#include <spdlog/common.h>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <Eigen/LU> // IWYU pragma: keep
#include <algorithm>
#include <any>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <format>
#include <functional>
#include <string>
#include <system_error>
#include <tuple>
#include <utility>
// IWYU pragma: no_include <Acts/Utilities/detail/ContextType.hpp>
// IWYU pragma: no_include <Acts/Utilities/detail/ContainerIterator.hpp>

#include "ActsGeometryProvider.h"
#include "extensions/edm4eic/EDM4eicToActs.h"
#include "extensions/spdlog/SpdlogFormatters.h" // IWYU pragma: keep
#include "extensions/spdlog/SpdlogToActs.h"

namespace {

/// Calibrator that reads directly from edm4eic::Measurement2DCollection.
/// Also owns the geometry-ordered IndexSourceLink multiset built from the same collection.
class EDM4eicMeasurementSourceLinkCalibrator {
public:
  using TrackStateProxy = eicrecon::CKFTracking::PodioTrackContainer::TrackStateProxy;

  EDM4eicMeasurementSourceLinkCalibrator(const edm4eic::Measurement2DCollection* meas2Ds,
                                         bool useTime)
      : m_meas2Ds(meas2Ds), m_useTime(useTime) {
    for (std::size_t index = 0; index < meas2Ds->size(); ++index) {
      m_orderedSourceLinks.emplace(Acts::GeometryIdentifier{(*meas2Ds)[index].getSurface()}, index);
    }
  }

  const ActsExamples::GeometryIdMultiset<ActsExamples::IndexSourceLink>&
  orderedSourceLinks() const {
    return m_orderedSourceLinks;
  }

  void calibrate(const Acts::GeometryContext& /*gctx*/, const Acts::CalibrationContext& /*cctx*/,
                 const Acts::SourceLink& sourceLink,
                 TrackStateProxy trackState) const {
    trackState.setUncalibratedSourceLink(Acts::SourceLink{sourceLink});
    const auto& idxSourceLink = sourceLink.get<ActsExamples::IndexSourceLink>();
    const auto& meas2D        = (*m_meas2Ds)[idxSourceLink.index()];

    if (m_useTime) {
      calibrateImpl<3>(meas2D, trackState);
    } else {
      calibrateImpl<2>(meas2D, trackState);
    }
  }

private:
  /// Fill the calibrated measurement with loc0, loc1 (N = 2) and additionally time (N = 3)
  template <std::size_t N>
  static void calibrateImpl(const edm4eic::Measurement2D& meas2D,
                            TrackStateProxy& trackState) {
#if Acts_VERSION_MAJOR > 45 || (Acts_VERSION_MAJOR == 45 && Acts_VERSION_MINOR >= 2)
    Acts::Vector<N> loc       = Acts::Vector<N>::Zero();
    Acts::SquareMatrix<N> cov = Acts::SquareMatrix<N>::Zero();
#else
    Acts::ActsVector<N> loc       = Acts::ActsVector<N>::Zero();
    Acts::ActsSquareMatrix<N> cov = Acts::ActsSquareMatrix<N>::Zero();
#endif
    // Bound parameters that make up the calibrated measurement, in storage order
    constexpr auto indices = []() {
      std::array<uint8_t, N> idx{static_cast<uint8_t>(Acts::eBoundLoc0),
                                 static_cast<uint8_t>(Acts::eBoundLoc1)};
      if constexpr (N == 3) {
        idx[2] = static_cast<uint8_t>(Acts::eBoundTime);
      }
      return idx;
    }();
    // Position of a bound parameter within the compact calibrated vector
    constexpr auto position = [indices](Acts::BoundIndices bound) {
      return static_cast<std::size_t>(std::find(indices.begin(), indices.end(), bound) -
                                      indices.begin());
    };
    constexpr std::size_t iLoc0 = position(Acts::eBoundLoc0);
    constexpr std::size_t iLoc1 = position(Acts::eBoundLoc1);

    constexpr auto mm  = Acts::UnitConstants::mm / edm4eic::unit::mm;
    constexpr auto mm2 = mm * mm;
    loc[iLoc0]         = meas2D.getLoc().a * mm;
    loc[iLoc1]         = meas2D.getLoc().b * mm;
    cov(iLoc0, iLoc0)  = meas2D.getCovariance().xx * mm2;
    cov(iLoc1, iLoc1)  = meas2D.getCovariance().yy * mm2;
    cov(iLoc0, iLoc1)  = meas2D.getCovariance().xy * mm2;
    cov(iLoc1, iLoc0)  = meas2D.getCovariance().xy * mm2;

    if constexpr (N == 3) {
      constexpr std::size_t iTime = position(Acts::eBoundTime);
      constexpr auto ns           = Acts::UnitConstants::ns / edm4eic::unit::ns;
      constexpr auto ns2          = ns * ns;
      // The time variance is stored in the zz component of the measurement covariance
      loc[iTime]        = meas2D.getTime() * ns;
      cov(iTime, iTime) = meas2D.getCovariance().zz * ns2;
      cov(iLoc0, iTime) = meas2D.getCovariance().xz * mm * ns;
      cov(iTime, iLoc0) = cov(iLoc0, iTime);
      cov(iLoc1, iTime) = meas2D.getCovariance().yz * mm * ns;
      cov(iTime, iLoc1) = cov(iLoc1, iTime);
    }

    trackState.allocateCalibrated(loc, cov);
    trackState.setProjectorSubspaceIndices(indices);
  }

  const edm4eic::Measurement2DCollection* m_meas2Ds;
  bool m_useTime;
  ActsExamples::GeometryIdMultiset<ActsExamples::IndexSourceLink> m_orderedSourceLinks;
};

/// Per-branch stopper for the CombinatorialKalmanFilter.
///
/// ACTS's default branch stopper never stops a branch, so a diverging branch is
/// never abandoned. Once a fit goes bad the covariance inflates, the Kalman
/// gain on q/p becomes ill-conditioned, and each further update throws q/p
/// further off. The covariance eventually overflows double precision and the
/// chi2 becomes NaN, which crashes the MeasurementSelector (seen in the
/// far-forward B0 tracker).
///
/// The divergence is visible in the filtered variance of q/p long before it
/// becomes fatal: a well-behaved fit never exceeds the variance asserted by the
/// seed, while a diverging one exceeds it by many orders of magnitude, and
/// ultimately turns negative or non-finite as positive-definiteness is lost.
/// Testing that variance therefore catches the divergence at the first bad
/// update, while the chi2 still looks harmless, and costs a single matrix
/// element with no history to inspect.
class CKFBranchStopper {
public:
  using TrackProxy      = eicrecon::CKFTracking::PodioTrackContainer::TrackProxy;
  using TrackStateProxy = eicrecon::CKFTracking::PodioTrackContainer::TrackStateProxy;
  using Result          = Acts::CombinatorialKalmanFilterBranchStopperResult;
  using LogFunc         = std::function<void(const std::string&)>;

  CKFBranchStopper(const eicrecon::CKFTrackingConfig& cfg, LogFunc log)
      : m_cfg(cfg), m_log(std::move(log)) {}

  Result operator()(const TrackProxy& track, const TrackStateProxy& trackState) const {
#if Acts_VERSION_MAJOR >= 45
    if (!trackState.typeFlags().hasMeasurement() || !trackState.hasFiltered()) {
#else
    if (!trackState.typeFlags().test(Acts::TrackStateFlag::MeasurementFlag) ||
        !trackState.hasFiltered()) {
#endif
      return Result::Continue;
    }

    const double varQOverP =
        trackState.filteredCovariance()(Acts::eBoundQOverP, Acts::eBoundQOverP);
    // Catches an inflated variance, and also a negative or NaN one, which mean
    // the covariance has lost positive-definiteness altogether.
    if (!(varQOverP > 0.) || varQOverP > m_cfg.maxQOverPVariance) {
      if (m_log) {
        m_log(std::format("Dropping diverged CKF branch with {} measurement(s): "
                          "filtered var(q/p) = {:g}, limit {:g}",
                          track.nMeasurements(), varQOverP, m_cfg.maxQOverPVariance));
      }
      return Result::StopAndDrop;
    }
    return Result::Continue;
  }

private:
  const eicrecon::CKFTrackingConfig& m_cfg;
  LogFunc m_log;
};

} // anonymous namespace

namespace eicrecon {

using namespace Acts::UnitLiterals;

void CKFTracking::init() {
  m_acts_logger = Acts::getDefaultLogger(
      "CKF", eicrecon::SpdlogToActsLevel(static_cast<spdlog::level::level_enum>(this->level())));

  // eta bins, chi2 and #sourclinks per surface cutoffs
  m_sourcelinkSelectorCfg = {
      {Acts::GeometryIdentifier(),
       {.etaBins               = m_cfg.etaBins,
        .chi2CutOff            = m_cfg.chi2CutOff,
        .numMeasurementsCutOff = {m_cfg.numMeasurementsCutOff.begin(),
                                  m_cfg.numMeasurementsCutOff.end()}}},
  };
  m_trackFinderFunc = CKFTracking::makeCKFTrackingFunction(
      m_geoSvc->trackingGeometry(), m_geoSvc->getFieldProvider(), acts_logger());
}

void CKFTracking::process(const Input& input, const Output& output) const {
  const auto [init_trk_seeds, meas2Ds] = input;
  auto [output_track_states, output_track_parameters, output_track_jacobians, output_tracks] =
      output;

  // If measurements or initial track parameters are empty, return early
  if (meas2Ds->empty() || init_trk_seeds->empty()) {
    return;
  }

  // Time-enabled calibration requires a valid time uncertainty on every measurement
  if (m_cfg.useTime) {
    bool invalid = false;
    for (const auto& meas2D : *meas2Ds) {
      const float var_t = meas2D.getCovariance().zz;
      if (!std::isfinite(var_t) || var_t <= 0) {
        error("UseTime is enabled but measurement on surface {} has invalid time variance {}; "
              "skipping tracking for this event",
              meas2D.getSurface(), var_t);
        invalid = true;
        break;
      }
    }
    if (invalid) {
      return;
    }
  }

  ActsExamples::TrackParametersContainer acts_init_trk_params;
  for (const auto& track_seed : *init_trk_seeds) {

    const auto& track_parameter = track_seed.getParams();

    Acts::BoundVector params;
    params(Acts::eBoundLoc0) =
        track_parameter.getLoc().a * Acts::UnitConstants::mm; // cylinder radius
    params(Acts::eBoundLoc1) =
        track_parameter.getLoc().b * Acts::UnitConstants::mm; // cylinder length
    params(Acts::eBoundPhi)    = track_parameter.getPhi();
    params(Acts::eBoundTheta)  = track_parameter.getTheta();
    params(Acts::eBoundQOverP) = track_parameter.getQOverP() / Acts::UnitConstants::GeV;
    params(Acts::eBoundTime)   = track_parameter.getTime() * Acts::UnitConstants::ns;

#if Acts_VERSION_MAJOR > 45 || (Acts_VERSION_MAJOR == 45 && Acts_VERSION_MINOR >= 1)
    Acts::BoundMatrix cov = Acts::BoundMatrix::Zero();
#else
    Acts::BoundSquareMatrix cov = Acts::BoundSquareMatrix::Zero();
#endif
    for (std::size_t i = 0; const auto& [a, x] : edm4eic_indexed_units) {
      for (std::size_t j = 0; const auto& [b, y] : edm4eic_indexed_units) {
        cov(a, b) = track_parameter.getCovariance()(i, j) * x * y;
        ++j;
      }
      ++i;
    }

    // Construct a perigee surface as the target surface
    auto pSurface = Acts::Surface::makeShared<const Acts::PerigeeSurface>(Acts::Vector3(0, 0, 0));

    // Create parameters
    acts_init_trk_params.emplace_back(pSurface, params, cov, Acts::ParticleHypothesis::pion());
  }

  //// Construct a perigee surface as the target surface
  auto pSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3{0., 0., 0.});

  // Convert algorithm log level to Acts log level for local logger
  const auto spdlog_level = static_cast<spdlog::level::level_enum>(this->level());
  const auto acts_level   = eicrecon::SpdlogToActsLevel(spdlog_level);
  ACTS_LOCAL_LOGGER(Acts::getDefaultLogger("CKF", acts_level));

  // Get run-scoped contexts from service
  const auto& gctx = m_geoSvc->getActsGeometryContext();
  const auto& mctx = m_geoSvc->getActsMagneticFieldContext();
  const auto& cctx = m_geoSvc->getActsCalibrationContext();

  // Create conversion helper for Podio backend
  PodioGeometryIdConversionHelper helper(gctx, m_geoSvc->trackingGeometry());

  // Create track container with ActsPodioEdm backend using externally owned collections
  auto trackStateContainer =
      std::make_shared<ActsPlugins::MutablePodioTrackStateContainer<Acts::RefHolder>>(
          helper, Acts::RefHolder{*output_track_states}, Acts::RefHolder{*output_track_parameters},
          Acts::RefHolder{*output_track_jacobians});
  auto trackContainer = std::make_shared<ActsPlugins::MutablePodioTrackContainer<Acts::RefHolder>>(
      helper, Acts::RefHolder{*output_tracks});

  // Create Acts TrackContainer with Podio backends using shared_ptr holder
  PodioTrackContainer acts_tracks(trackContainer, trackStateContainer);

  Acts::PropagatorPlainOptions pOptions(gctx, mctx);
  pOptions.maxSteps = 10000;

  EDM4eicMeasurementSourceLinkCalibrator calibratorImpl{meas2Ds, m_cfg.useTime};
  Acts::GainMatrixUpdater kfUpdater;
  Acts::MeasurementSelector measSel{m_sourcelinkSelectorCfg};

  Acts::CombinatorialKalmanFilterExtensions<PodioTrackContainer> extensions;
  extensions.updater.connect<&Acts::GainMatrixUpdater::operator()<
      typename PodioTrackContainer::TrackStateContainerBackend>>(&kfUpdater);

  ActsExamples::IndexSourceLinkAccessor slAccessor;
  slAccessor.container = &calibratorImpl.orderedSourceLinks();
  using TrackStateCreatorType =
      Acts::TrackStateCreator<ActsExamples::IndexSourceLinkAccessor::Iterator, PodioTrackContainer>;
  TrackStateCreatorType trackStateCreator;
  trackStateCreator.sourceLinkAccessor
      .template connect<&ActsExamples::IndexSourceLinkAccessor::range>(&slAccessor);
  trackStateCreator.calibrator.template connect<&EDM4eicMeasurementSourceLinkCalibrator::calibrate>(
      &calibratorImpl);
  trackStateCreator.measurementSelector.template connect<&Acts::MeasurementSelector::select<
      ActsPlugins::MutablePodioTrackStateContainer<Acts::RefHolder>>>(&measSel);

  extensions.createTrackStates.template connect<&TrackStateCreatorType::createTrackStates>(
      &trackStateCreator);

  // Per-branch stopping (ACTS default never stops a branch).
  CKFBranchStopper branchStopper{m_cfg, [this](const std::string& msg) { debug("{}", msg); }};
  extensions.branchStopper.connect<&CKFBranchStopper::operator()>(&branchStopper);

  // Set the CombinatorialKalmanFilter options
  CKFTracking::TrackFinderOptions options(gctx, mctx, cctx, extensions, pOptions);

  using Extrapolator        = Acts::Propagator<Acts::EigenStepper<>, Acts::Navigator>;
  using ExtrapolatorOptions = Extrapolator::template Options<
      Acts::ActorList<Acts::MaterialInteractor, Acts::EndOfWorldReached>>;
  Extrapolator extrapolator(Acts::EigenStepper<>(m_BField),
                            Acts::Navigator({.trackingGeometry = m_geoSvc->trackingGeometry()},
                                            acts_logger().cloneWithSuffix("Navigator")),
                            acts_logger().cloneWithSuffix("Propagator"));
  ExtrapolatorOptions extrapolationOptions(gctx, mctx);

  // Create temporary track containers for intermediate processing
  // Heap-allocate collections to ensure proper lifetime management across all copyFrom operations
  auto tempTrackStates = std::make_shared<ActsPodioEdm::TrackStateCollection>();
  auto tempTrackParams = std::make_shared<ActsPodioEdm::BoundParametersCollection>();
  auto tempTrackJacs   = std::make_shared<ActsPodioEdm::JacobianCollection>();
  auto tempTracks      = std::make_shared<ActsPodioEdm::TrackCollection>();

  auto trackStateContainerTemp =
      std::make_shared<ActsPlugins::MutablePodioTrackStateContainer<Acts::RefHolder>>(
          helper, Acts::RefHolder<ActsPodioEdm::TrackStateCollection>{*tempTrackStates},
          Acts::RefHolder<ActsPodioEdm::BoundParametersCollection>{*tempTrackParams},
          Acts::RefHolder<ActsPodioEdm::JacobianCollection>{*tempTrackJacs});
  auto trackContainerTemp =
      std::make_shared<ActsPlugins::MutablePodioTrackContainer<Acts::RefHolder>>(
          helper, Acts::RefHolder<ActsPodioEdm::TrackCollection>{*tempTracks});
  PodioTrackContainer acts_tracks_temp(trackContainerTemp, trackStateContainerTemp);

  // TODO: Add seed number tracking back later using PODIO links or associations
  // Dynamic columns in PODIO backend currently cause serialization issues
  // acts_tracks.addColumn<unsigned int>("seed");
  // acts_tracks_temp.addColumn<unsigned int>("seed");
  // Acts::ProxyAccessor<unsigned int> seedNumber("seed");

  // Loop over seeds
  for (std::size_t iseed = 0; iseed < acts_init_trk_params.size(); ++iseed) {

    // Clear trackContainerTemp and trackStateContainerTemp
    acts_tracks_temp.clear();

    // Run track finding for this seed
    auto result = (*m_trackFinderFunc)(acts_init_trk_params.at(iseed), options, acts_tracks_temp);

    if (!result.ok()) {
      debug("Track finding failed for seed {} with error {}", iseed, result.error().message());
      continue;
    }

    // Set seed number for all found tracks
    auto& tracksForSeed = result.value();
    for (auto& track : tracksForSeed) {
      // Check if track has at least one valid (non-outlier) measurement
      // (this check avoids errors inside smoothing and extrapolation)
      auto lastMeasurement = Acts::findLastMeasurementState(track);
      if (!lastMeasurement.ok()) {
        debug("Track {} for seed {} has no valid measurements, skipping", track.index(), iseed);
        continue;
      }

      if (track.nMeasurements() < m_cfg.numMeasurementsMin) {
        trace("Track {} for seed {} has fewer measurements than minimum of {}, skipping",
              track.index(), iseed, m_cfg.numMeasurementsMin);
        continue;
      }

      auto smoothingResult = Acts::smoothTrack(gctx, track, acts_logger());
      if (!smoothingResult.ok()) {
        debug("Smoothing for seed {} and track {} failed with error {}", iseed, track.index(),
              smoothingResult.error().message());
        continue;
      }

      auto extrapolationResult = Acts::extrapolateTrackToReferenceSurface(
          track, *pSurface, extrapolator, extrapolationOptions,
          Acts::TrackExtrapolationStrategy::firstOrLast, acts_logger());

      if (!extrapolationResult.ok()) {
        debug("Extrapolation for seed {} and track {} failed with error {}", iseed, track.index(),
              extrapolationResult.error().message());
        continue;
      }

      // TODO: Restore seed number tracking when dynamic columns are fixed
      // seedNumber(track) = iseed;

      // Copy accepted track into main track container
      auto acts_tracks_proxy = acts_tracks.makeTrack();
      acts_tracks_proxy.copyFrom(track);

      // Set the reference surface on the output track
      acts_tracks_proxy.setReferenceSurface(pSurface);
    }
  }
}

} // namespace eicrecon
