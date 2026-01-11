// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022, 2023 Wenqing Fan, Barak Schmookler, Whitney Armstrong, Sylvester Joosten, Dmitry Romanov, Christopher Dilks, Wouter Deconinck

#include <Acts/Definitions/Algebra.hpp>
#include <Acts/Definitions/Common.hpp>
#include <Acts/Definitions/Direction.hpp>
#include <Acts/Definitions/TrackParametrization.hpp>
#include <Acts/Definitions/Units.hpp>
#include <Acts/EventData/GenericBoundTrackParameters.hpp>
#include <Acts/EventData/MultiTrajectoryHelpers.hpp>
#include <Acts/EventData/VectorMultiTrajectory.hpp>
#include <Acts/Geometry/GeometryIdentifier.hpp>
#include <Acts/Geometry/TrackingGeometry.hpp>
#include <Acts/MagneticField/MagneticFieldProvider.hpp>
#include <Acts/Material/MaterialInteraction.hpp>
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
#include <Acts/Propagator/PropagatorResult.hpp>
#include <Acts/Surfaces/CylinderBounds.hpp>
#include <Acts/Surfaces/CylinderSurface.hpp>
#include <Acts/Surfaces/DiscSurface.hpp>
#include <Acts/Surfaces/RadialBounds.hpp>
#include <Acts/Utilities/Logger.hpp>
#include <DD4hep/Handle.h>
#include <Evaluator/DD4hepUnits.h>
#include <edm4eic/Cov2f.h>
#include <edm4eic/Cov3f.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <algorithm>
#include <any>
#include <cmath>
#include <cstdint>
#include <functional>
#include <iterator>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <tuple>
#include <typeinfo>
#include <utility>
#include <variant>

#include "algorithms/tracking/ActsGeometryProvider.h"
#include "algorithms/tracking/TrackPropagation.h"
#include "algorithms/tracking/TrackPropagationConfig.h"
#include "extensions/spdlog/SpdlogToActs.h"

namespace eicrecon {

template <typename... L> struct multilambda : L... {
  using L::operator()...;
  constexpr multilambda(L... lambda) : L(std::move(lambda))... {}
};

void TrackPropagation::init(const dd4hep::Detector* detector,
                            std::shared_ptr<const ActsGeometryProvider> geo_svc,
                            std::shared_ptr<spdlog::logger> logger) {
  m_geoSvc = geo_svc;
  m_log    = logger;

  std::map<uint32_t, std::size_t> system_id_layers;

  multilambda _toDouble = {
      [](const std::string& v) { return dd4hep::_toDouble(v); },
      [](const double& v) { return v; },
  };

  auto _toActsSurface =
      [&_toDouble, &detector, &system_id_layers](
          const std::variant<CylinderSurfaceConfig, DiscSurfaceConfig> surface_variant)
      -> std::shared_ptr<Acts::Surface> {
    if (std::holds_alternative<CylinderSurfaceConfig>(surface_variant)) {
      CylinderSurfaceConfig surface = std::get<CylinderSurfaceConfig>(surface_variant);
      const double rmin =
          std::visit(_toDouble, surface.rmin) / dd4hep::mm * Acts::UnitConstants::mm;
      const double zmin =
          std::visit(_toDouble, surface.zmin) / dd4hep::mm * Acts::UnitConstants::mm;
      const double zmax =
          std::visit(_toDouble, surface.zmax) / dd4hep::mm * Acts::UnitConstants::mm;
      const uint32_t system_id = detector->constant<uint32_t>(surface.id);
      auto bounds              = std::make_shared<Acts::CylinderBounds>(rmin, (zmax - zmin) / 2);
      auto t                   = Acts::Translation3(Acts::Vector3(0, 0, (zmax + zmin) / 2));
      auto tf                  = Acts::Transform3(t);
      auto acts_surface        = Acts::Surface::makeShared<Acts::CylinderSurface>(tf, bounds);
#if Acts_VERSION_MAJOR >= 40
      acts_surface->assignGeometryId(
          Acts::GeometryIdentifier().withExtra(system_id).withLayer(++system_id_layers[system_id]));
#else
      acts_surface->assignGeometryId(
          Acts::GeometryIdentifier().setExtra(system_id).setLayer(++system_id_layers[system_id]));
#endif
      return acts_surface;
    }
    if (std::holds_alternative<DiscSurfaceConfig>(surface_variant)) {
      DiscSurfaceConfig surface = std::get<DiscSurfaceConfig>(surface_variant);
      const double zmin =
          std::visit(_toDouble, surface.zmin) / dd4hep::mm * Acts::UnitConstants::mm;
      const double rmin =
          std::visit(_toDouble, surface.rmin) / dd4hep::mm * Acts::UnitConstants::mm;
      const double rmax =
          std::visit(_toDouble, surface.rmax) / dd4hep::mm * Acts::UnitConstants::mm;
      const uint32_t system_id = detector->constant<uint32_t>(surface.id);
      auto bounds              = std::make_shared<Acts::RadialBounds>(rmin, rmax);
      auto t                   = Acts::Translation3(Acts::Vector3(0, 0, zmin));
      auto tf                  = Acts::Transform3(t);
      auto acts_surface        = Acts::Surface::makeShared<Acts::DiscSurface>(tf, bounds);
#if Acts_VERSION_MAJOR >= 40
      acts_surface->assignGeometryId(
          Acts::GeometryIdentifier().withExtra(system_id).withLayer(++system_id_layers[system_id]));
#else
      acts_surface->assignGeometryId(
          Acts::GeometryIdentifier().setExtra(system_id).setLayer(++system_id_layers[system_id]));
#endif
      return acts_surface;
    }
    throw std::domain_error("Unknown surface type");
  };
  m_target_surfaces.resize(m_cfg.target_surfaces.size());
  std::ranges::transform(m_cfg.target_surfaces, m_target_surfaces.begin(), _toActsSurface);
  m_filter_surfaces.resize(m_cfg.filter_surfaces.size());
  std::ranges::transform(m_cfg.filter_surfaces, m_filter_surfaces.begin(), _toActsSurface);

  m_log->trace("Initialized");
}

void TrackPropagation::propagateToSurfaceList(
    const std::tuple<const edm4eic::TrackCollection&,
                     const std::vector<const ActsExamples::ConstTrackContainer*>>
        input,
    const std::tuple<edm4eic::TrackSegmentCollection*> output) const {
  const auto [tracks, acts_tracks] = input;
  auto [track_segments]            = output;

  // logging
  m_log->trace("Propagate tracks: --------------------");
  m_log->trace("number of tracks: {}", tracks.size());
  m_log->trace("number of acts_tracks: {}", acts_tracks.size());

  if (acts_tracks.empty()) {
    return;
  }

  const auto& constTracks = *acts_tracks.front();

  // loop over input tracks
  std::size_t i = 0;
  for (const auto& track : constTracks) {

    // check if this track can be propagated to any filter surface
    bool track_reaches_filter_surface{false};
    for (const auto& filter_surface : m_filter_surfaces) {
      auto point = propagate(edm4eic::Track{}, track, constTracks, filter_surface);
      if (point) {
        track_reaches_filter_surface = true;
        break;
      }
    }
    if (!track_reaches_filter_surface) {
      ++i;
      continue;
    }

    // start a mutable TrackSegment
    auto track_segment = track_segments->create();

    // corresponding track
    if (tracks.size() == constTracks.size()) {
      m_log->trace("track segment connected to track {}", i);
      track_segment.setTrack(tracks[i]);
      ++i;
    }

    // zero measurements of segment length
    decltype(edm4eic::TrackSegmentData::length) length            = 0;
    decltype(edm4eic::TrackSegmentData::lengthError) length_error = 0;

    // loop over projection-target surfaces
    for (const auto& target_surface : m_target_surfaces) {

      // project the track to this surface
      auto point = propagate(edm4eic::Track{}, track, constTracks, target_surface);
      if (!point) {
        m_log->trace("<> Failed to propagate track to this plane");
        continue;
      }

      // logging
      m_log->trace("<> track: x=( {:>10.2f} {:>10.2f} {:>10.2f} )", point->position.x,
                   point->position.y, point->position.z);
      m_log->trace("               p=( {:>10.2f} {:>10.2f} {:>10.2f} )", point->momentum.x,
                   point->momentum.y, point->momentum.z);

      // track point cut
      if (!m_cfg.track_point_cut(*point)) {
        m_log->trace("                 => REJECTED by trackPointCut");
        if (m_cfg.skip_track_on_track_point_cut_failure) {
          break;
        }
        continue;
      }

      // update the `TrackSegment` length
      // FIXME: `length` and `length_error` are currently not used by any callers, and may not be correctly calculated here
      if (track_segment.points_size() > 0) {
        auto pos0 = point->position;
        auto pos1 = std::prev(track_segment.points_end())->position;
        auto dist = edm4hep::utils::magnitude(pos0 - pos1);
        length += dist;
        m_log->trace("               dist to previous point: {}", dist);
      }

      // add the `TrackPoint` to the `TrackSegment`
      track_segment.addToPoints(*point);

    } // end `targetSurfaces` loop

    // set final length and length error
    track_segment.setLength(length);
    track_segment.setLengthError(length_error);

  } // end loop over input tracks
}

std::unique_ptr<edm4eic::TrackPoint>
TrackPropagation::propagate(const edm4eic::Track& /* track */,
                            const ActsExamples::ConstTrackProxy& acts_track,
                            const ActsExamples::ConstTrackContainer& trackContainer,
                            const std::shared_ptr<const Acts::Surface>& targetSurf) const {

  auto tipIndex = acts_track.tipIndex();

  m_log->trace("  Propagating track with tip index {}", tipIndex);

  // Collect the trajectory summary info
  auto trajState =
      Acts::MultiTrajectoryHelpers::trajectoryState(trackContainer.trackStateContainer(), tipIndex);
  int m_nMeasurements = trajState.nMeasurements;
  int m_nStates       = trajState.nStates;

  m_log->trace("  Num measurement in trajectory: {}", m_nMeasurements);
  m_log->trace("  Num states in trajectory     : {}", m_nStates);

  // Get track state at last measurement surface
  // For last measurement surface, filtered and smoothed results are equivalent
  auto trackState        = trackContainer.trackStateContainer().getTrackState(tipIndex);
  auto initSurface       = trackState.referenceSurface().getSharedPtr();
  const auto& initParams = trackState.filtered();
  const auto& initCov    = trackState.filteredCovariance();

  Acts::BoundTrackParameters initBoundParams(initSurface, initParams, initCov,
                                             acts_track.particleHypothesis());

  // Get pathlength of last track state with respect to perigee surface
  const auto initPathLength = trackState.pathLength();

  m_log->trace("    TrackPropagation. Propagating to surface # {}",
               typeid(targetSurf->type()).name());

  std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry   = m_geoSvc->trackingGeometry();
  std::shared_ptr<const Acts::MagneticFieldProvider> magneticField = m_geoSvc->getFieldProvider();

  ACTS_LOCAL_LOGGER(eicrecon::getSpdlogLogger("PROP", m_log));

  using Propagator = Acts::Propagator<Acts::EigenStepper<>, Acts::Navigator>;
#if Acts_VERSION_MAJOR >= 37
  using PropagatorOptions = Propagator::template Options<Acts::ActorList<Acts::MaterialInteractor>>;
#else
  using PropagatorOptions =
      Propagator::template Options<Acts::ActionList<Acts::MaterialInteractor>>;
#endif
  Propagator propagator(Acts::EigenStepper<>(magneticField),
                        Acts::Navigator({.trackingGeometry = m_geoSvc->trackingGeometry()},
                                        logger().cloneWithSuffix("Navigator")),
                        logger().cloneWithSuffix("Propagator"));
  PropagatorOptions propagationOptions(m_geoContext, m_fieldContext);

  auto result = propagator.propagate(initBoundParams, *targetSurf, propagationOptions);

  // check propagation result
  if (!result.ok()) {
    m_log->trace("    propagation failed (!result.ok())");
    return nullptr;
  }
  m_log->trace("    propagation result is OK");

  // Pulling results to convenient variables
  auto trackStateParams  = *((*result).endParameters);
  const auto& parameter  = trackStateParams.parameters();
  const auto& covariance = *trackStateParams.covariance();

  // Path length
  const float pathLength      = initPathLength + (*result).pathLength;
  const float pathLengthError = 0;
  m_log->trace("    path len = {}", pathLength);

  // Position:
  auto projectionPos = trackStateParams.position(m_geoContext);
  const decltype(edm4eic::TrackPoint::position) position{static_cast<float>(projectionPos(0)),
                                                         static_cast<float>(projectionPos(1)),
                                                         static_cast<float>(projectionPos(2))};
  const decltype(edm4eic::TrackPoint::positionError) positionError{0, 0, 0};
  m_log->trace("    pos x = {}", position.x);
  m_log->trace("    pos y = {}", position.y);
  m_log->trace("    pos z = {}", position.z);

  // Momentum
  const decltype(edm4eic::TrackPoint::momentum) momentum = edm4hep::utils::sphericalToVector(
      static_cast<float>(1.0 / std::abs(parameter[Acts::eBoundQOverP])),
      static_cast<float>(parameter[Acts::eBoundTheta]),
      static_cast<float>(parameter[Acts::eBoundPhi]));
  const decltype(edm4eic::TrackPoint::momentumError) momentumError{
      static_cast<float>(covariance(Acts::eBoundTheta, Acts::eBoundTheta)),
      static_cast<float>(covariance(Acts::eBoundPhi, Acts::eBoundPhi)),
      static_cast<float>(covariance(Acts::eBoundQOverP, Acts::eBoundQOverP)),
      static_cast<float>(covariance(Acts::eBoundTheta, Acts::eBoundPhi)),
      static_cast<float>(covariance(Acts::eBoundTheta, Acts::eBoundQOverP)),
      static_cast<float>(covariance(Acts::eBoundPhi, Acts::eBoundQOverP))};

  // time
  const float time{static_cast<float>(parameter(Acts::eBoundTime))};
  const float timeError{static_cast<float>(sqrt(covariance(Acts::eBoundTime, Acts::eBoundTime)))};

  // Direction
  const float theta(parameter[Acts::eBoundTheta]);
  const float phi(parameter[Acts::eBoundPhi]);
  const decltype(edm4eic::TrackPoint::directionError) directionError{
      static_cast<float>(covariance(Acts::eBoundTheta, Acts::eBoundTheta)),
      static_cast<float>(covariance(Acts::eBoundPhi, Acts::eBoundPhi)),
      static_cast<float>(covariance(Acts::eBoundTheta, Acts::eBoundPhi))};

  // >oO debug print
  m_log->trace("    loc 0   = {:.4f}", parameter[Acts::eBoundLoc0]);
  m_log->trace("    loc 1   = {:.4f}", parameter[Acts::eBoundLoc1]);
  m_log->trace("    phi     = {:.4f}", parameter[Acts::eBoundPhi]);
  m_log->trace("    theta   = {:.4f}", parameter[Acts::eBoundTheta]);
  m_log->trace("    q/p     = {:.4f}", parameter[Acts::eBoundQOverP]);
  m_log->trace("    p       = {:.4f}", 1.0 / parameter[Acts::eBoundQOverP]);
  m_log->trace("    err phi = {:.4f}", sqrt(covariance(Acts::eBoundPhi, Acts::eBoundPhi)));
  m_log->trace("    err th  = {:.4f}", sqrt(covariance(Acts::eBoundTheta, Acts::eBoundTheta)));
  m_log->trace("    err q/p = {:.4f}", sqrt(covariance(Acts::eBoundQOverP, Acts::eBoundQOverP)));
  m_log->trace("    chi2    = {:.4f}", trajState.chi2Sum);
  m_log->trace("    loc err = {:.4f}",
               static_cast<float>(covariance(Acts::eBoundLoc0, Acts::eBoundLoc0)));
  m_log->trace("    loc err = {:.4f}",
               static_cast<float>(covariance(Acts::eBoundLoc1, Acts::eBoundLoc1)));
  m_log->trace("    loc err = {:.4f}",
               static_cast<float>(covariance(Acts::eBoundLoc0, Acts::eBoundLoc1)));

  uint64_t surface = targetSurf->geometryId().value();
  uint32_t system  = 0; // default value...will be set in TrackPropagation factory

  return std::make_unique<edm4eic::TrackPoint>(
      edm4eic::TrackPoint{.surface         = surface,
                          .system          = system,
                          .position        = position,
                          .positionError   = positionError,
                          .momentum        = momentum,
                          .momentumError   = momentumError,
                          .time            = time,
                          .timeError       = timeError,
                          .theta           = theta,
                          .phi             = phi,
                          .directionError  = directionError,
                          .pathlength      = pathLength,
                          .pathlengthError = pathLengthError});
}

} // namespace eicrecon
