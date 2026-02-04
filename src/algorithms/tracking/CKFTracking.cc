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
#include <Acts/Geometry/GeometryContext.hpp>
#include <Acts/Geometry/GeometryHierarchyMap.hpp>
#include <Acts/MagneticField/MagneticFieldContext.hpp>
#include <Acts/Utilities/CalibrationContext.hpp>
#include <spdlog/common.h>
#include <algorithm>
#include <any>
#include <array>
#include <cstddef>
#include <functional>
#include <gsl/pointers>
#include <new>
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
#include <edm4eic/TrackSeedCollection.h>
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
  const auto [init_trk_seeds, meas2Ds]      = input;
  auto [output_track_states, output_tracks] = output;

  // If measurements or initial track parameters are empty, return early
  if (meas2Ds->empty() || init_trk_seeds->empty()) {
    return;
  }

  // create sourcelink and measurement containers
  auto measurements = std::make_shared<ActsExamples::MeasurementContainer>();

  // need list here for stable addresses
#if Acts_VERSION_MAJOR < 37 || (Acts_VERSION_MAJOR == 37 && Acts_VERSION_MINOR < 1)
  std::list<ActsExamples::IndexSourceLink> sourceLinkStorage;
  ActsExamples::IndexSourceLinkContainer src_links;
  src_links.reserve(meas2Ds->size());
  std::size_t hit_index = 0;
#endif

  for (const auto& meas2D : *meas2Ds) {

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
#else
    auto measurement = ActsExamples::makeVariableSizeMeasurement(
        Acts::SourceLink{sourceLink}, loc, cov, Acts::eBoundLoc0, Acts::eBoundLoc1);
    measurements->emplace_back(std::move(measurement));
#endif

#if Acts_VERSION_MAJOR < 37 || (Acts_VERSION_MAJOR == 37 && Acts_VERSION_MINOR < 1)
    hit_index++;
#endif
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

  // Convert algorithm log level to Acts log level for local logger
  const auto spdlog_level = static_cast<spdlog::level::level_enum>(this->level());
  const auto acts_level   = eicrecon::SpdlogToActsLevel(spdlog_level);
  ACTS_LOCAL_LOGGER(Acts::getDefaultLogger("CKF", acts_level));

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
                                            acts_logger().cloneWithSuffix("Navigator")),
                            acts_logger().cloneWithSuffix("Propagator"));
  ExtrapolatorOptions extrapolationOptions(m_geoctx, m_fieldctx);

  // Create track container
  auto trackContainer      = std::make_shared<Acts::VectorTrackContainer>();
  auto trackStateContainer = std::make_shared<Acts::VectorMultiTrajectory>();
  ActsExamples::TrackContainer acts_tracks(trackContainer, trackStateContainer);

  // Create temporary track container
  auto trackContainerTemp      = std::make_shared<Acts::VectorTrackContainer>();
  auto trackStateContainerTemp = std::make_shared<Acts::VectorMultiTrajectory>();
  ActsExamples::TrackContainer acts_tracks_temp(trackContainerTemp, trackStateContainerTemp);

  // Add seed number column
  acts_tracks.addColumn<unsigned int>("seed");
  acts_tracks_temp.addColumn<unsigned int>("seed");
  Acts::ProxyAccessor<unsigned int> seedNumber("seed");

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

      auto smoothingResult = Acts::smoothTrack(m_geoctx, track, acts_logger());
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

      seedNumber(track) = iseed;

      // Copy accepted track into main track container
      auto acts_tracks_proxy = acts_tracks.makeTrack();
      acts_tracks_proxy.copyFrom(track);
    }
  }

  // Move track states and track container to const containers as naked pointers
  auto constTrackStateContainer =
      std::make_shared<Acts::ConstVectorMultiTrajectory>(std::move(*trackStateContainer));

  auto constTrackContainer =
      std::make_shared<Acts::ConstVectorTrackContainer>(std::move(*trackContainer));

  // Output pointers are double-pointers, dereference once and use placement new
  new (*output_track_states) Acts::ConstVectorMultiTrajectory(*constTrackStateContainer);
  new (*output_tracks) Acts::ConstVectorTrackContainer(*constTrackContainer);
}

} // namespace eicrecon
