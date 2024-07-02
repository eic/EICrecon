// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 wfan, Whitney Armstrong, Sylvester Joosten

#include <Acts/Definitions/TrackParametrization.hpp>
#include <Acts/EventData/MultiTrajectoryHelpers.hpp>
#include <Acts/Geometry/GeometryIdentifier.hpp>
#include <Acts/Utilities/UnitVectors.hpp>
#include <ActsExamples/EventData/Trajectories.hpp>
#include <edm4eic/Cov2f.h>
#include <edm4eic/Cov3f.h>
#include <edm4eic/TrackParametersCollection.h>
#include <edm4eic/TrackPoint.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <fmt/core.h>
#include <fmt/ostream.h>
#include <spdlog/logger.h>
#include <stdint.h>
#include <cmath>
#include <iterator>
#include <utility>

#include "TrackProjector.h"
#include "extensions/spdlog/SpdlogFormatters.h" // IWYU pragma: keep

#if FMT_VERSION >= 90000
template<> struct fmt::formatter<Acts::GeometryIdentifier> : fmt::ostream_formatter {};
#endif // FMT_VERSION >= 90000

namespace eicrecon {

    void
    TrackProjector::init(std::shared_ptr<const ActsGeometryProvider> geo_svc, std::shared_ptr<spdlog::logger> logger) {
        m_log = logger;
        m_geo_provider = geo_svc;
    }


    std::unique_ptr<edm4eic::TrackSegmentCollection> TrackProjector::execute(std::vector<const ActsExamples::Trajectories *> trajectories) {

        // create output collections
        auto track_segments = std::make_unique<edm4eic::TrackSegmentCollection>();
        m_log->debug("Track projector event process. Num of input trajectories: {}", std::size(trajectories));

        // Loop over the trajectories
        for (const auto &traj: trajectories) {
            // Get the entry index for the single trajectory
            // The trajectory entry indices and the multiTrajectory
            const auto &mj = traj->multiTrajectory();
            const auto &trackTips = traj->tips();
            m_log->debug("------ Trajectory ------");
            m_log->debug("  Num of elements in trackTips {}", trackTips.size());

            // Skip empty
            if (trackTips.empty()) {
                m_log->debug("  Empty multiTrajectory.");
                continue;
            }
            const auto &trackTip = trackTips.front();

            // Collect the trajectory summary info
            auto trajState = Acts::MultiTrajectoryHelpers::trajectoryState(mj, trackTip);
            int m_nMeasurements = trajState.nMeasurements;
            int m_nStates = trajState.nStates;
            int m_nCalibrated = 0;
            m_log->debug("  Num measurement in trajectory {}", m_nMeasurements);
            m_log->debug("  Num state in trajectory {}", m_nStates);

            auto track_segment = track_segments->create();

            // visit the track points
            mj.visitBackwards(trackTip, [&](auto &&trackstate) {
                // get volume info
                auto geoID = trackstate.referenceSurface().geometryId();
                auto volume = geoID.volume();
                auto layer = geoID.layer();

                if (trackstate.hasCalibrated()) {
                    m_nCalibrated++;
                }

                // get track state bound parameters and their boundCovs
                const auto &boundParams = trackstate.predicted();
                const auto &boundCov = trackstate.predictedCovariance();

                // convert local to global
                auto global = trackstate.referenceSurface().localToGlobal(
                        m_geo_provider->getActsGeometryContext(),
                        {boundParams[Acts::eBoundLoc0], boundParams[Acts::eBoundLoc1]},
                        Acts::makeDirectionFromPhiTheta(
                            boundParams[Acts::eBoundPhi],
                            boundParams[Acts::eBoundTheta]
                        )
                );

#if Acts_VERSION_MAJOR >= 34
                auto freeParams = Acts::transformBoundToFreeParameters(
                        trackstate.referenceSurface(),
                        m_geo_provider->getActsGeometryContext(),
                        boundParams);
                auto jacobian = trackstate.referenceSurface().boundToFreeJacobian(
                        m_geo_provider->getActsGeometryContext(),
                        freeParams.template segment<3>(Acts::eFreePos0),
                        freeParams.template segment<3>(Acts::eFreeDir0)
                );
#else
                auto jacobian = trackstate.referenceSurface().boundToFreeJacobian(
                        m_geo_provider->getActsGeometryContext(),
                        boundParams
                );
#endif
                auto freeCov = jacobian * boundCov * jacobian.transpose();

                // global position
                const decltype(edm4eic::TrackPoint::position) position{
                        static_cast<float>(global.x()),
                        static_cast<float>(global.y()),
                        static_cast<float>(global.z())
                };

                // local position
                const decltype(edm4eic::TrackParametersData::loc) loc{
                        static_cast<float>(boundParams[Acts::eBoundLoc0]),
                        static_cast<float>(boundParams[Acts::eBoundLoc1])
                };
                const edm4eic::Cov2f locError{
                        static_cast<float>(boundCov(Acts::eBoundLoc0, Acts::eBoundLoc0)),
                        static_cast<float>(boundCov(Acts::eBoundLoc1, Acts::eBoundLoc1)),
                        static_cast<float>(boundCov(Acts::eBoundLoc0, Acts::eBoundLoc1))
                };
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
                        static_cast<float>(boundParams[Acts::eBoundPhi])
                );
                const decltype(edm4eic::TrackPoint::momentumError) momentumError{
                        static_cast<float>(boundCov(Acts::eBoundTheta, Acts::eBoundTheta)),
                        static_cast<float>(boundCov(Acts::eBoundPhi, Acts::eBoundPhi)),
                        static_cast<float>(boundCov(Acts::eBoundQOverP, Acts::eBoundQOverP)),
                        static_cast<float>(boundCov(Acts::eBoundTheta, Acts::eBoundPhi)),
                        static_cast<float>(boundCov(Acts::eBoundTheta, Acts::eBoundQOverP)),
                        static_cast<float>(boundCov(Acts::eBoundPhi, Acts::eBoundQOverP))
                };
                const float time{static_cast<float>(boundParams(Acts::eBoundTime))};
                const float timeError{sqrt(static_cast<float>(boundCov(Acts::eBoundTime, Acts::eBoundTime)))};
                const float theta(boundParams[Acts::eBoundTheta]);
                const float phi(boundParams[Acts::eBoundPhi]);
                const decltype(edm4eic::TrackPoint::directionError) directionError{
                        static_cast<float>(boundCov(Acts::eBoundTheta, Acts::eBoundTheta)),
                        static_cast<float>(boundCov(Acts::eBoundPhi, Acts::eBoundPhi)),
                        static_cast<float>(boundCov(Acts::eBoundTheta, Acts::eBoundPhi))
                };
                const float pathLength = static_cast<float>(trackstate.pathLength());
                const float pathLengthError = 0;

                uint64_t surface = trackstate.referenceSurface().geometryId().value();
                uint32_t system = 0;

                // Store track point
                track_segment.addToPoints({
                                                  surface,
                                                  system,
                                                  position,
                                                  positionError,
                                                  momentum,
                                                  momentumError,
                                                  time,
                                                  timeError,
                                                  theta,
                                                  phi,
                                                  directionError,
                                                  pathLength,
                                                  pathLengthError
                                          });


                m_log->debug("  ******************************");
                m_log->debug("    position: {}", position);
                m_log->debug("    positionError: {}", positionError);
                m_log->debug("    momentum: {}", momentum);
                m_log->debug("    momentumError: {}", momentumError);
                m_log->debug("    time: {}", time);
                m_log->debug("    timeError: {}", timeError);
                m_log->debug("    theta: {}", theta);
                m_log->debug("    phi: {}", phi);
                m_log->debug("    directionError: {}", directionError);
                m_log->debug("    pathLength: {}", pathLength);
                m_log->debug("    pathLengthError: {}", pathLengthError);
                m_log->debug("    geoID = {}", geoID);
                m_log->debug("    volume = {}, layer = {}", volume, layer);
                m_log->debug("    pathlength = {}", pathLength);
                m_log->debug("    hasCalibrated = {}", trackstate.hasCalibrated());
                m_log->debug("  ******************************");

                // Local position on the reference surface.
                //m_log->debug("boundParams[eBoundLoc0] = {}", boundParams[Acts::eBoundLoc0]);
                //m_log->debug("boundParams[eBoundLoc1] = {}", boundParams[Acts::eBoundLoc1]);
                //m_log->debug("boundParams[eBoundPhi] = {}", boundParams[Acts::eBoundPhi]);
                //m_log->debug("boundParams[eBoundTheta] = {}", boundParams[Acts::eBoundTheta]);
                //m_log->debug("boundParams[eBoundQOverP] = {}", boundParams[Acts::eBoundQOverP]);
                //m_log->debug("boundParams[eBoundTime] = {}", boundParams[Acts::eBoundTime]);
                //m_log->debug("predicted variables: {}", trackstate.predicted());
            });

            m_log->debug("  Num calibrated state in trajectory {}", m_nCalibrated);
            m_log->debug("------ end of trajectory process ------");
        }

        m_log->debug("END OF Track projector event process");
        return std::move(track_segments);
    }


} // namespace eicrecon::Reco
