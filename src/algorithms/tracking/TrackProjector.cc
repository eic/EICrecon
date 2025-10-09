// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2025 wfan, Whitney Armstrong, Sylvester Joosten, Dmitry Kalinkin

#include <Acts/Definitions/TrackParametrization.hpp>
#include <Acts/EventData/MultiTrajectoryHelpers.hpp>
#if Acts_VERSION_MAJOR >= 34
#include <Acts/EventData/TransformationHelpers.hpp>
#endif
#include <Acts/Geometry/GeometryIdentifier.hpp>
#include <Acts/Utilities/UnitVectors.hpp>
#include <ActsExamples/EventData/Trajectories.hpp>
#include <algorithms/service.h>
#include <edm4eic/Cov2f.h>
#include <edm4eic/Cov3f.h>
#include <edm4eic/TrackParametersCollection.h>
#include <edm4eic/TrackPoint.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <edm4hep/Vector2f.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <fmt/core.h>
#include <fmt/ostream.h>
#include <any>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <gsl/pointers>
#include <iterator>

#include "TrackProjector.h"
#include "algorithms/interfaces/ActsSvc.h"
#include "extensions/spdlog/SpdlogFormatters.h" // IWYU pragma: keep

#if FMT_VERSION >= 90000
template <> struct fmt::formatter<Acts::GeometryIdentifier> : fmt::ostream_formatter {};
#endif // FMT_VERSION >= 90000

namespace eicrecon {

void TrackProjector::init() {
  auto& serviceSvc = algorithms::ServiceSvc::instance();
  m_geo_provider   = serviceSvc.service<algorithms::ActsSvc>("ActsSvc")->acts_geometry_provider();
}

void TrackProjector::process(const Input& input, const Output& output) const {
  const auto [acts_trajectories, tracks] = input;
  auto [track_segments]                  = output;

  debug("Track projector event process. Num of input trajectories: {}",
        std::size(acts_trajectories));

  // Loop over the trajectories
  for (std::size_t i = 0; const auto& traj : acts_trajectories) {
    // Get the entry index for the single trajectory
    // The trajectory entry indices and the multiTrajectory
    const auto& mj        = traj->multiTrajectory();
    const auto& trackTips = traj->tips();
    debug("------ Trajectory ------");
    debug("  Num of elements in trackTips {}", trackTips.size());

    // Skip empty
    if (trackTips.empty()) {
      debug("  Empty multiTrajectory.");
      continue;
    }
    const auto& trackTip = trackTips.front();

    // Collect the trajectory summary info
    auto trajState      = Acts::MultiTrajectoryHelpers::trajectoryState(mj, trackTip);
    int m_nMeasurements = trajState.nMeasurements;
    int m_nStates       = trajState.nStates;
    int m_nCalibrated   = 0;
    debug("  Num measurement in trajectory {}", m_nMeasurements);
    debug("  Num state in trajectory {}", m_nStates);

    auto track_segment = track_segments->create();

    // corresponding track
    if (tracks->size() == acts_trajectories.size()) {
      trace("track segment connected to track {}", i);
      track_segment.setTrack((*tracks)[i]);
      ++i;
    }

    // visit the track points
    mj.visitBackwards(trackTip, [&](auto&& trackstate) {
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
          m_geo_provider->getActsGeometryContext(),
          {boundParams[Acts::eBoundLoc0], boundParams[Acts::eBoundLoc1]},
          Acts::makeDirectionFromPhiTheta(boundParams[Acts::eBoundPhi],
                                          boundParams[Acts::eBoundTheta]));

#if Acts_VERSION_MAJOR >= 34
      auto freeParams = Acts::transformBoundToFreeParameters(
          trackstate.referenceSurface(), m_geo_provider->getActsGeometryContext(), boundParams);
      auto jacobian = trackstate.referenceSurface().boundToFreeJacobian(
          m_geo_provider->getActsGeometryContext(), freeParams.template segment<3>(Acts::eFreePos0),
          freeParams.template segment<3>(Acts::eFreeDir0));
#else
                auto jacobian = trackstate.referenceSurface().boundToFreeJacobian(
                        m_geo_provider->getActsGeometryContext(),
                        boundParams
                );
#endif
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
    });

    debug("  Num calibrated state in trajectory {}", m_nCalibrated);
    debug("------ end of trajectory process ------");
  }

  debug("END OF Track projector event process");
}

} // namespace eicrecon
