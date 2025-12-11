// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2025 wfan, Whitney Armstrong, Sylvester Joosten, Dmitry Kalinkin

#include <Acts/Definitions/TrackParametrization.hpp>
#include <Acts/EventData/MultiTrajectoryHelpers.hpp>
#include <Acts/EventData/TrackContainer.hpp>
#include <Acts/EventData/TrackProxy.hpp>
#include <Acts/EventData/TransformationHelpers.hpp>
#include <Acts/EventData/VectorMultiTrajectory.hpp>
#include <Acts/Geometry/GeometryIdentifier.hpp>
#include <Acts/Surfaces/Surface.hpp>
#include <Acts/Utilities/UnitVectors.hpp>
#include <ActsExamples/EventData/Track.hpp>
#include <algorithms/service.h>
#include <edm4eic/Cov2f.h>
#include <edm4eic/Cov3f.h>
#include <edm4eic/TrackParametersCollection.h>
#include <edm4eic/TrackPoint.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <edm4hep/Vector2f.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <Eigen/Core>
#include <any>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <gsl/pointers>

#include "TrackProjector.h"
#include "algorithms/interfaces/ActsSvc.h"
#include "extensions/spdlog/SpdlogFormatters.h" // IWYU pragma: keep

template <> struct fmt::formatter<Acts::GeometryIdentifier> : fmt::ostream_formatter {};

namespace eicrecon {

void TrackProjector::init() { m_acts_detector = algorithms::ActsSvc::instance().detector(); }

void TrackProjector::process(const Input& input, const Output& output) const {
  const auto [track_states, tracks_container, tracks] = input;
  auto [track_segments]                               = output;

  // Construct ConstTrackContainer from underlying containers
  auto trackStateContainer = std::make_shared<Acts::ConstVectorMultiTrajectory>(*track_states);
  auto trackContainer      = std::make_shared<Acts::ConstVectorTrackContainer>(*tracks_container);
  ActsExamples::ConstTrackContainer acts_tracks(trackContainer, trackStateContainer);

  debug("Track projector event process. Num of input tracks: {}", acts_tracks.size());

  // Loop over the tracks
  std::size_t i = 0;
  for (const auto& track : acts_tracks) {
    auto tipIndex = track.tipIndex();
    debug("------ Track ------");
    debug("  Tip index {}", tipIndex);

    // Collect the trajectory summary info
    auto trajState =
        Acts::MultiTrajectoryHelpers::trajectoryState(acts_tracks.trackStateContainer(), tipIndex);
    int m_nMeasurements = trajState.nMeasurements;
    int m_nStates       = trajState.nStates;
    int m_nCalibrated   = 0;
    debug("  Num measurement in trajectory {}", m_nMeasurements);
    debug("  Num state in trajectory {}", m_nStates);

    auto track_segment = track_segments->create();

    // corresponding track
    if (tracks->size() == acts_tracks.size()) {
      trace("track segment connected to track {}", i);
      track_segment.setTrack((*tracks)[i]);
      ++i;
    }

    // visit the track points
    for (const auto& trackstate : track.trackStatesReversed()) {
      // get volume info
      auto geoID  = trackstate.referenceSurface().geometryId();
      auto volume = geoID.volume();
      auto layer  = geoID.layer();

      if (trackstate.hasCalibrated()) {
        m_nCalibrated++;
      }

      // get track state bound parameters and their boundCovs
      const auto& boundParams = trackstate.predicted();
      const auto& boundCov    = trackstate.predictedCovariance();

      // convert local to global
      auto global = trackstate.referenceSurface().localToGlobal(
          m_acts_detector->getActsGeometryContext(),
          {boundParams[Acts::eBoundLoc0], boundParams[Acts::eBoundLoc1]},
          Acts::makeDirectionFromPhiTheta(boundParams[Acts::eBoundPhi],
                                          boundParams[Acts::eBoundTheta]));

      auto freeParams = Acts::transformBoundToFreeParameters(
          trackstate.referenceSurface(), m_acts_detector->getActsGeometryContext(), boundParams);
      auto jacobian = trackstate.referenceSurface().boundToFreeJacobian(
          m_acts_detector->getActsGeometryContext(),
          freeParams.template segment<3>(Acts::eFreePos0),
          freeParams.template segment<3>(Acts::eFreeDir0));
      auto freeCov = jacobian * boundCov * jacobian.transpose();

      // global position
      const decltype(edm4eic::TrackPoint::position) position{static_cast<float>(global.x()),
                                                             static_cast<float>(global.y()),
                                                             static_cast<float>(global.z())};

      // local position
      const decltype(edm4eic::TrackParametersData::loc) loc{
          static_cast<float>(boundParams[Acts::eBoundLoc0]),
          static_cast<float>(boundParams[Acts::eBoundLoc1])};
      const edm4eic::Cov2f locError{
          static_cast<float>(boundCov(Acts::eBoundLoc0, Acts::eBoundLoc0)),
          static_cast<float>(boundCov(Acts::eBoundLoc1, Acts::eBoundLoc1)),
          static_cast<float>(boundCov(Acts::eBoundLoc0, Acts::eBoundLoc1))};
      const decltype(edm4eic::TrackPoint::positionError) positionError{
          static_cast<float>(freeCov(Acts::eFreePos0, Acts::eFreePos0)),
          static_cast<float>(freeCov(Acts::eFreePos1, Acts::eFreePos1)),
          static_cast<float>(freeCov(Acts::eFreePos2, Acts::eFreePos2)),
          static_cast<float>(freeCov(Acts::eFreePos0, Acts::eFreePos1)),
          static_cast<float>(freeCov(Acts::eFreePos0, Acts::eFreePos2)),
          static_cast<float>(freeCov(Acts::eFreePos1, Acts::eFreePos2)),
      };

      // momentum
      const decltype(edm4eic::TrackPoint::momentum) momentum = edm4hep::utils::sphericalToVector(
          static_cast<float>(1.0 / std::abs(boundParams[Acts::eBoundQOverP])),
          static_cast<float>(boundParams[Acts::eBoundTheta]),
          static_cast<float>(boundParams[Acts::eBoundPhi]));
      const decltype(edm4eic::TrackPoint::momentumError) momentumError{
          static_cast<float>(boundCov(Acts::eBoundTheta, Acts::eBoundTheta)),
          static_cast<float>(boundCov(Acts::eBoundPhi, Acts::eBoundPhi)),
          static_cast<float>(boundCov(Acts::eBoundQOverP, Acts::eBoundQOverP)),
          static_cast<float>(boundCov(Acts::eBoundTheta, Acts::eBoundPhi)),
          static_cast<float>(boundCov(Acts::eBoundTheta, Acts::eBoundQOverP)),
          static_cast<float>(boundCov(Acts::eBoundPhi, Acts::eBoundQOverP))};
      const float time{static_cast<float>(boundParams(Acts::eBoundTime))};
      const float timeError{static_cast<float>(sqrt(boundCov(Acts::eBoundTime, Acts::eBoundTime)))};
      const float theta(boundParams[Acts::eBoundTheta]);
      const float phi(boundParams[Acts::eBoundPhi]);
      const decltype(edm4eic::TrackPoint::directionError) directionError{
          static_cast<float>(boundCov(Acts::eBoundTheta, Acts::eBoundTheta)),
          static_cast<float>(boundCov(Acts::eBoundPhi, Acts::eBoundPhi)),
          static_cast<float>(boundCov(Acts::eBoundTheta, Acts::eBoundPhi))};
      const auto pathLength       = static_cast<float>(trackstate.pathLength());
      const float pathLengthError = 0;

      uint64_t surface = trackstate.referenceSurface().geometryId().value();
      uint32_t system  = 0;

      // Store track point
      track_segment.addToPoints({.surface         = surface,
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

      debug("  ******************************");
      debug("    position: {}", position);
      debug("    positionError: {}", positionError);
      debug("    momentum: {}", momentum);
      debug("    momentumError: {}", momentumError);
      debug("    time: {}", time);
      debug("    timeError: {}", timeError);
      debug("    theta: {}", theta);
      debug("    phi: {}", phi);
      debug("    directionError: {}", directionError);
      debug("    pathLength: {}", pathLength);
      debug("    pathLengthError: {}", pathLengthError);
      debug("    geoID = {}", geoID);
      debug("    volume = {}, layer = {}", volume, layer);
      debug("    pathlength = {}", pathLength);
      debug("    hasCalibrated = {}", trackstate.hasCalibrated());
      debug("  ******************************");

      // Local position on the reference surface.
      //debug("boundParams[eBoundLoc0] = {}", boundParams[Acts::eBoundLoc0]);
      //debug("boundParams[eBoundLoc1] = {}", boundParams[Acts::eBoundLoc1]);
      //debug("boundParams[eBoundPhi] = {}", boundParams[Acts::eBoundPhi]);
      //debug("boundParams[eBoundTheta] = {}", boundParams[Acts::eBoundTheta]);
      //debug("boundParams[eBoundQOverP] = {}", boundParams[Acts::eBoundQOverP]);
      //debug("boundParams[eBoundTime] = {}", boundParams[Acts::eBoundTime]);
      //debug("predicted variables: {}", trackstate.predicted());
    }

    debug("  Num calibrated state in track {}", m_nCalibrated);
    debug("------ end of track process ------");
  }

  debug("END OF Track projector event process");
}

} // namespace eicrecon
