// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2025 Whitney Armstrong, Wouter Deconinck, Dmitry Romanov, Shujie Li, Dmitry Kalinkin

#include "CKFTracking.h"

#include <Acts/Definitions/Algebra.hpp>
#include <Acts/Definitions/Common.hpp>
#include <Acts/Definitions/Direction.hpp>
#include <Acts/Definitions/TrackParametrization.hpp>
#include <Acts/Definitions/Units.hpp>
#include <Acts/EventData/GenericBoundTrackParameters.hpp>
#include <Acts/EventData/MeasurementHelpers.hpp>
#include <Acts/EventData/TrackStatePropMask.hpp>
#include <Acts/EventData/Types.hpp>
#include <Acts/Geometry/GeometryHierarchyMap.hpp>
#include <fmt/format.h>
#include <algorithm>
#include <any>
#include <array>
#include <cstddef>
#include <functional>
#include <set>
#include <stdexcept>
#include <string>
#include <system_error>
#include <tuple>
#include <utility>
#if Acts_VERSION_MAJOR >= 39
#include <Acts/TrackFinding/CombinatorialKalmanFilterExtensions.hpp>
#endif
#if (Acts_VERSION_MAJOR >= 37) && (Acts_VERSION_MAJOR < 43)
#include <Acts/Utilities/Iterator.hpp>
#endif
#include <Acts/EventData/ParticleHypothesis.hpp>
#include <Acts/EventData/ProxyAccessor.hpp>
#include <Acts/EventData/SourceLink.hpp>
#include <Acts/EventData/TrackContainer.hpp>
#include <Acts/EventData/TrackProxy.hpp>
#include <Acts/EventData/VectorMultiTrajectory.hpp>
#include <Acts/EventData/VectorTrackContainer.hpp>
#include <Acts/Geometry/GeometryIdentifier.hpp>
#if Acts_VERSION_MAJOR >= 37
#include <Acts/Propagator/ActorList.hpp>
#else
#include <Acts/Propagator/AbortList.hpp>
#include <Acts/Propagator/ActionList.hpp>
#endif
#include <Acts/Propagator/EigenStepper.hpp>
#include <Acts/Propagator/MaterialInteractor.hpp>
#include <Acts/Propagator/Navigator.hpp>
#include <Acts/Propagator/Propagator.hpp>
#include <Acts/Propagator/PropagatorOptions.hpp>
#include <Acts/Propagator/StandardAborters.hpp>
#include <Acts/Surfaces/PerigeeSurface.hpp>
#include <Acts/Surfaces/Surface.hpp>
#if Acts_VERSION_MAJOR >= 39
#include <Acts/TrackFinding/TrackStateCreator.hpp>
#endif
#include <Acts/TrackFitting/GainMatrixUpdater.hpp>
#include <Acts/Utilities/Logger.hpp>
#include <Acts/Utilities/TrackHelpers.hpp>
#include <ActsExamples/EventData/IndexSourceLink.hpp>
#include <ActsExamples/EventData/Measurement.hpp>
#include <ActsExamples/EventData/MeasurementCalibration.hpp>
#include <ActsExamples/EventData/Track.hpp>
#include <boost/container/vector.hpp>
#include <edm4eic/Cov3f.h>
#include <edm4eic/Cov6f.h>
#include <edm4eic/Measurement2DCollection.h>
#include <edm4eic/TrackParametersCollection.h>
#include <edm4hep/Vector2f.h>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <Eigen/LU> // IWYU pragma: keep
// IWYU pragma: no_include <Acts/Utilities/detail/ContextType.hpp>
// IWYU pragma: no_include <Acts/Utilities/detail/ContainerIterator.hpp>

#include "ActsGeometryProvider.h"
#include "extensions/edm4eic/EDM4eicToActs.h"
#include "extensions/spdlog/SpdlogFormatters.h" // IWYU pragma: keep
#include "extensions/spdlog/SpdlogToActs.h"

namespace eicrecon {

using namespace Acts::UnitLiterals;

CKFTracking::CKFTracking() = default;

void CKFTracking::init(std::shared_ptr<const ActsGeometryProvider> geo_svc,
                       std::shared_ptr<spdlog::logger> log) {
  m_log         = log;
  m_acts_logger = eicrecon::getSpdlogLogger("CKF", m_log);

  m_geoSvc = geo_svc;

  m_BField   = m_geoSvc->getFieldProvider();
  m_fieldctx = Acts::MagneticFieldContext{};

  // eta bins, chi2 and #sourclinks per surface cutoffs
  m_sourcelinkSelectorCfg = {
      {Acts::GeometryIdentifier(),
       {.etaBins               = m_cfg.etaBins,
        .chi2CutOff            = m_cfg.chi2CutOff,
        .numMeasurementsCutOff = {m_cfg.numMeasurementsCutOff.begin(),
                                  m_cfg.numMeasurementsCutOff.end()}}},
  };
  m_trackFinderFunc =
      CKFTracking::makeCKFTrackingFunction(m_geoSvc->trackingGeometry(), m_BField, logger());
}

std::tuple<std::vector<Acts::ConstVectorMultiTrajectory*>,
           std::vector<Acts::ConstVectorTrackContainer*>>
CKFTracking::process(const edm4eic::TrackParametersCollection& init_trk_params,
                     const edm4eic::Measurement2DCollection& meas2Ds) {

  // Create output collections
  std::vector<Acts::ConstVectorMultiTrajectory*> trajectories_v;
  std::vector<Acts::ConstVectorTrackContainer*> tracks_v;

  // If measurements or initial track parameters are empty, return early with empty container
  if (meas2Ds.empty() || init_trk_params.empty()) {
    auto emptyTrackStateContainer = std::make_shared<Acts::ConstVectorMultiTrajectory>();
    auto emptyTrackContainer      = std::make_shared<Acts::ConstVectorTrackContainer>();
    trajectories_v.push_back(new Acts::ConstVectorMultiTrajectory(*emptyTrackStateContainer));
    tracks_v.push_back(new Acts::ConstVectorTrackContainer(*emptyTrackContainer));
    return {trajectories_v, tracks_v};
  }

  // create sourcelink and measurement containers
  auto measurements = std::make_shared<ActsExamples::MeasurementContainer>();

  // need list here for stable addresses
#if Acts_VERSION_MAJOR < 37 || (Acts_VERSION_MAJOR == 37 && Acts_VERSION_MINOR < 1)
  std::list<ActsExamples::IndexSourceLink> sourceLinkStorage;
  ActsExamples::IndexSourceLinkContainer src_links;
  src_links.reserve(meas2Ds.size());
  std::size_t hit_index = 0;
#endif

  for (const auto& meas2D : meas2Ds) {

    Acts::GeometryIdentifier geoId{meas2D.getSurface()};

#if Acts_VERSION_MAJOR < 37 || (Acts_VERSION_MAJOR == 37 && Acts_VERSION_MINOR < 1)
    // --follow example from ACTS to create source links
    sourceLinkStorage.emplace_back(geoId, hit_index);
    ActsExamples::IndexSourceLink& sourceLink = sourceLinkStorage.back();
    // Add to output containers:
    // index map and source link container are geometry-ordered.
    // since the input is also geometry-ordered, new items can
    // be added at the end.
    src_links.insert(src_links.end(), sourceLink);
#endif
    // ---
    // Create ACTS measurements

    Acts::ActsVector<2> loc = Acts::Vector2::Zero();
    loc[Acts::eBoundLoc0]   = meas2D.getLoc().a;
    loc[Acts::eBoundLoc1]   = meas2D.getLoc().b;

    Acts::ActsSquareMatrix<2> cov = Acts::ActsSquareMatrix<2>::Zero();
    cov(0, 0)                     = meas2D.getCovariance().xx;
    cov(1, 1)                     = meas2D.getCovariance().yy;
    cov(0, 1)                     = meas2D.getCovariance().xy;
    cov(1, 0)                     = meas2D.getCovariance().xy;

#if Acts_VERSION_MAJOR > 37 || (Acts_VERSION_MAJOR == 37 && Acts_VERSION_MINOR >= 1)
    std::array<Acts::BoundIndices, 2> indices{Acts::eBoundLoc0, Acts::eBoundLoc1};
    Acts::visit_measurement(
        indices.size(), [&](auto dim) -> ActsExamples::VariableBoundMeasurementProxy {
          if constexpr (dim == indices.size()) {
            return ActsExamples::VariableBoundMeasurementProxy{
                measurements->emplaceMeasurement<dim>(geoId, indices, loc, cov)};
          } else {
            throw std::runtime_error("Dimension not supported in measurement creation");
          }
        });
#elif Acts_VERSION_MAJOR == 37 && Acts_VERSION_MINOR == 0
    std::array<Acts::BoundIndices, 2> indices{Acts::eBoundLoc0, Acts::eBoundLoc1};
    Acts::visit_measurement(
        indices.size(), [&](auto dim) -> ActsExamples::VariableBoundMeasurementProxy {
          if constexpr (dim == indices.size()) {
            return ActsExamples::VariableBoundMeasurementProxy{
                measurements->emplaceMeasurement<dim>(Acts::SourceLink{sourceLink}, indices, loc,
                                                      cov)};
          } else {
            throw std::runtime_error("Dimension not supported in measurement creation");
          }
        });
#elif Acts_VERSION_MAJOR == 36 && Acts_VERSION_MINOR >= 1
    auto measurement = ActsExamples::makeVariableSizeMeasurement(
        Acts::SourceLink{sourceLink}, loc, cov, Acts::eBoundLoc0, Acts::eBoundLoc1);
    measurements->emplace_back(std::move(measurement));
#elif Acts_VERSION_MAJOR == 36 && Acts_VERSION_MINOR == 0
    auto measurement = ActsExamples::makeFixedSizeMeasurement(
        Acts::SourceLink{sourceLink}, loc, cov, Acts::eBoundLoc0, Acts::eBoundLoc1);
    measurements->emplace_back(std::move(measurement));
#endif

#if Acts_VERSION_MAJOR < 37 || (Acts_VERSION_MAJOR == 37 && Acts_VERSION_MINOR < 1)
    hit_index++;
#endif
  }

  ActsExamples::TrackParametersContainer acts_init_trk_params;
  for (const auto& track_parameter : init_trk_params) {

    Acts::BoundVector params;
    params(Acts::eBoundLoc0) =
        track_parameter.getLoc().a * Acts::UnitConstants::mm; // cylinder radius
    params(Acts::eBoundLoc1) =
        track_parameter.getLoc().b * Acts::UnitConstants::mm; // cylinder length
    params(Acts::eBoundPhi)    = track_parameter.getPhi();
    params(Acts::eBoundTheta)  = track_parameter.getTheta();
    params(Acts::eBoundQOverP) = track_parameter.getQOverP() / Acts::UnitConstants::GeV;
    params(Acts::eBoundTime)   = track_parameter.getTime() * Acts::UnitConstants::ns;

    Acts::BoundSquareMatrix cov = Acts::BoundSquareMatrix::Zero();
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

  ACTS_LOCAL_LOGGER(eicrecon::getSpdlogLogger("CKF", m_log, {"^No tracks found$"}));

  Acts::PropagatorPlainOptions pOptions(m_geoctx, m_fieldctx);
  pOptions.maxSteps = 10000;

  ActsExamples::PassThroughCalibrator pcalibrator;
  ActsExamples::MeasurementCalibratorAdapter calibrator(pcalibrator, *measurements);
  Acts::GainMatrixUpdater kfUpdater;
  Acts::MeasurementSelector measSel{m_sourcelinkSelectorCfg};

  Acts::CombinatorialKalmanFilterExtensions<ActsExamples::TrackContainer> extensions;
#if Acts_VERSION_MAJOR < 39
  extensions.calibrator.connect<&ActsExamples::MeasurementCalibratorAdapter::calibrate>(
      &calibrator);
#endif
  extensions.updater.connect<&Acts::GainMatrixUpdater::operator()<
      typename ActsExamples::TrackContainer::TrackStateContainerBackend>>(&kfUpdater);
#if Acts_VERSION_MAJOR < 39
  extensions.measurementSelector.connect<&Acts::MeasurementSelector::select<
      typename ActsExamples::TrackContainer::TrackStateContainerBackend>>(&measSel);
#endif

  ActsExamples::IndexSourceLinkAccessor slAccessor;
#if Acts_VERSION_MAJOR > 37 || (Acts_VERSION_MAJOR == 37 && Acts_VERSION_MINOR >= 1)
  slAccessor.container = &measurements->orderedIndices();
#else
  slAccessor.container = &src_links;
#endif
#if Acts_VERSION_MAJOR >= 39
  using TrackStateCreatorType =
      Acts::TrackStateCreator<ActsExamples::IndexSourceLinkAccessor::Iterator,
                              ActsExamples::TrackContainer>;
  TrackStateCreatorType trackStateCreator;
  trackStateCreator.sourceLinkAccessor
      .template connect<&ActsExamples::IndexSourceLinkAccessor::range>(&slAccessor);
  trackStateCreator.calibrator
      .template connect<&ActsExamples::MeasurementCalibratorAdapter::calibrate>(&calibrator);
  trackStateCreator.measurementSelector
      .template connect<&Acts::MeasurementSelector::select<Acts::VectorMultiTrajectory>>(&measSel);

  extensions.createTrackStates.template connect<&TrackStateCreatorType::createTrackStates>(
      &trackStateCreator);
#else
  Acts::SourceLinkAccessorDelegate<ActsExamples::IndexSourceLinkAccessor::Iterator>
      slAccessorDelegate;
  slAccessorDelegate.connect<&ActsExamples::IndexSourceLinkAccessor::range>(&slAccessor);
#endif

  // Set the CombinatorialKalmanFilter options
#if Acts_VERSION_MAJOR >= 39
  CKFTracking::TrackFinderOptions options(m_geoctx, m_fieldctx, m_calibctx, extensions, pOptions);
#else
  CKFTracking::TrackFinderOptions options(m_geoctx, m_fieldctx, m_calibctx, slAccessorDelegate,
                                          extensions, pOptions);
#endif

  using Extrapolator = Acts::Propagator<Acts::EigenStepper<>, Acts::Navigator>;
#if Acts_VERSION_MAJOR >= 37
  using ExtrapolatorOptions = Extrapolator::template Options<
      Acts::ActorList<Acts::MaterialInteractor, Acts::EndOfWorldReached>>;
#else
  using ExtrapolatorOptions =
      Extrapolator::template Options<Acts::ActionList<Acts::MaterialInteractor>,
                                     Acts::AbortList<Acts::EndOfWorldReached>>;
#endif
  Extrapolator extrapolator(Acts::EigenStepper<>(m_BField),
                            Acts::Navigator({.trackingGeometry = m_geoSvc->trackingGeometry()},
                                            logger().cloneWithSuffix("Navigator")),
                            logger().cloneWithSuffix("Propagator"));
  ExtrapolatorOptions extrapolationOptions(m_geoctx, m_fieldctx);

  // Create track container
  auto trackContainer      = std::make_shared<Acts::VectorTrackContainer>();
  auto trackStateContainer = std::make_shared<Acts::VectorMultiTrajectory>();
  ActsExamples::TrackContainer acts_tracks(trackContainer, trackStateContainer);

  // Add seed number column
  acts_tracks.addColumn<unsigned int>("seed");
  Acts::ProxyAccessor<unsigned int> seedNumber("seed");
  std::set<Acts::TrackIndexType> passed_tracks;

  // Loop over seeds
  for (std::size_t iseed = 0; iseed < acts_init_trk_params.size(); ++iseed) {
    auto result = (*m_trackFinderFunc)(acts_init_trk_params.at(iseed), options, acts_tracks);

    if (!result.ok()) {
      m_log->debug("Track finding failed for seed {} with error {}", iseed,
                   result.error().message());
      continue;
    }

    // Set seed number for all found tracks
    auto& tracksForSeed = result.value();
    for (auto& track : tracksForSeed) {
      // Check if track has at least one valid (non-outlier) measurement
      // (this check avoids errors inside smoothing and extrapolation)
      auto lastMeasurement = Acts::findLastMeasurementState(track);
      if (!lastMeasurement.ok()) {
        m_log->debug("Track {} for seed {} has no valid measurements, skipping", track.index(),
                     iseed);
        continue;
      }

      auto smoothingResult = Acts::smoothTrack(m_geoctx, track, logger());
      if (!smoothingResult.ok()) {
        m_log->debug("Smoothing for seed {} and track {} failed with error {}", iseed,
                     track.index(), smoothingResult.error().message());
        continue;
      }

      auto extrapolationResult = Acts::extrapolateTrackToReferenceSurface(
          track, *pSurface, extrapolator, extrapolationOptions,
          Acts::TrackExtrapolationStrategy::firstOrLast, logger());

      if (!extrapolationResult.ok()) {
        m_log->debug("Extrapolation for seed {} and track {} failed with error {}", iseed,
                     track.index(), extrapolationResult.error().message());
        continue;
      }

      passed_tracks.insert(track.index());
      seedNumber(track) = iseed;
    }
  }

  for (std::size_t track_index = acts_tracks.size(); (track_index--) != 0U;) {
    if (!passed_tracks.contains(track_index)) {
      // NOTE This does not remove track states corresponding to the
      // removed tracks. Doing so would require implementing some garbage
      // collection. We'll just assume no algorithm will access them
      // directly.
      acts_tracks.removeTrack(track_index);
#if Acts_VERSION_MAJOR == 36 && Acts_VERSION_MINOR < 1
      // Workaround an upstream bug in Acts::VectorTrackContainer::removeTrack_impl()
      // https://github.com/acts-project/acts/commit/94cf81f3f1109210b963977e0904516b949b1154
      trackContainer->m_particleHypothesis.erase(trackContainer->m_particleHypothesis.begin() +
                                                 track_index);
#endif
    }
  }

  // Move track states and track container to const containers
  auto constTrackStateContainer =
      std::make_shared<Acts::ConstVectorMultiTrajectory>(std::move(*trackStateContainer));

  auto constTrackContainer =
      std::make_shared<Acts::ConstVectorTrackContainer>(std::move(*trackContainer));

  trajectories_v.push_back(new Acts::ConstVectorMultiTrajectory(*constTrackStateContainer));
  tracks_v.push_back(new Acts::ConstVectorTrackContainer(*constTrackContainer));

  return {trajectories_v, tracks_v};
}

} // namespace eicrecon
