// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 wfan, Whitney Armstrong, Sylvester Joosten

#include <algorithm>

#include "DDRec/CellIDPositionConverter.h"
#include "DDRec/SurfaceManager.h"
#include "DDRec/Surface.h"

#include "Acts/EventData/MultiTrajectory.hpp"
#include "Acts/EventData/MultiTrajectoryHelpers.hpp"

// Event Model related classes
#include "edm4eic/TrackerHitCollection.h"
#include "edm4eic/TrackParametersCollection.h"
#include "edm4eic/TrajectoryCollection.h"
#include "edm4eic/TrackSegmentCollection.h"
#include "JugTrack/IndexSourceLink.hpp"
#include "JugTrack/Track.hpp"
#include "JugTrack/Trajectories.hpp"

#include "Acts/Utilities/Helpers.hpp"
#include "Acts/Geometry/GeometryIdentifier.hpp"
#include "Acts/MagneticField/ConstantBField.hpp"
#include "Acts/MagneticField/InterpolatedBFieldMap.hpp"
#include "Acts/MagneticField/SharedBField.hpp"
#include "Acts/Propagator/EigenStepper.hpp"
#include "Acts/Surfaces/PerigeeSurface.hpp"

#include "edm4eic/vector_utils.h"

#include <algorithms/interfaces/WithPodConfig.h>
#include <spdlog/logger.h>
#include <spdlog/fmt/ostr.h>
#include "TrackProjectorConfig.h"
#include "TrackProjector.h"

#include <cmath>

namespace eicrecon {

    void
    TrackProjector::init(std::shared_ptr<const ActsGeometryProvider> geo_svc, std::shared_ptr<spdlog::logger> logger) {
        m_log = logger;
        m_geo_provider = geo_svc;
    }


    std::vector<edm4eic::TrackSegment *> TrackProjector::execute(std::vector<const Jug::Trajectories *> trajectories) {

        // create output collections
        std::vector<edm4eic::TrackSegment *> track_segments;
        m_log->debug("Track projector evnet process. Num of input trajectories: {}", std::size(trajectories));

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
            auto &trackTip = trackTips.front();

            // Collect the trajectory summary info
            auto trajState = Acts::MultiTrajectoryHelpers::trajectoryState(mj, trackTip);
            int m_nMeasurements = trajState.nMeasurements;
            int m_nStates = trajState.nStates;
            int m_nCalibrated = 0;
            m_log->debug("  Num measurement in trajectory {}", m_nMeasurements);
            m_log->debug("  Num state in trajectory {}", m_nStates);

            edm4eic::MutableTrackSegment track_segment;

            // visit the track points
            mj.visitBackwards(trackTip, [&](auto &&trackstate) {
                // get volume info
                auto geoID = trackstate.referenceSurface().geometryId();
                auto volume = geoID.volume();
                auto layer = geoID.layer();
                if (trackstate.hasCalibrated()) {
                    m_nCalibrated++;
                }

                // get track state parameters and their covariances
                const auto &parameter = trackstate.predicted();
                const auto &covariance = trackstate.predictedCovariance();

                // convert local to global
                auto global = trackstate.referenceSurface().localToGlobal(
                        m_geo_provider->getActsGeometryContext(),
                        {parameter[Acts::eBoundLoc0], parameter[Acts::eBoundLoc1]},
                        {0, 0, 0}
                );
                // global position
                const decltype(edm4eic::TrackPoint::position) position{
                        static_cast<float>(global.x()),
                        static_cast<float>(global.y()),
                        static_cast<float>(global.z())
                };

                // local position
                const decltype(edm4eic::TrackParametersData::loc) loc{
                        static_cast<float>(parameter[Acts::eBoundLoc0]),
                        static_cast<float>(parameter[Acts::eBoundLoc1])
                };
                const decltype(edm4eic::TrackParametersData::locError) locError{
                        static_cast<float>(covariance(Acts::eBoundLoc0, Acts::eBoundLoc0)),
                        static_cast<float>(covariance(Acts::eBoundLoc1, Acts::eBoundLoc1)),
                        static_cast<float>(covariance(Acts::eBoundLoc0, Acts::eBoundLoc1))
                };
                const decltype(edm4eic::TrackPoint::positionError) positionError{0, 0, 0};
                const decltype(edm4eic::TrackPoint::momentum) momentum = edm4eic::sphericalToVector(
                        static_cast<float>(1.0 / std::abs(parameter[Acts::eBoundQOverP])),
                        static_cast<float>(parameter[Acts::eBoundTheta]),
                        static_cast<float>(parameter[Acts::eBoundPhi])
                );
                const decltype(edm4eic::TrackPoint::momentumError) momentumError{
                        static_cast<float>(covariance(Acts::eBoundTheta, Acts::eBoundTheta)),
                        static_cast<float>(covariance(Acts::eBoundPhi, Acts::eBoundPhi)),
                        static_cast<float>(covariance(Acts::eBoundQOverP, Acts::eBoundQOverP)),
                        static_cast<float>(covariance(Acts::eBoundTheta, Acts::eBoundPhi)),
                        static_cast<float>(covariance(Acts::eBoundTheta, Acts::eBoundQOverP)),
                        static_cast<float>(covariance(Acts::eBoundPhi, Acts::eBoundQOverP))
                };
                const float time{static_cast<float>(parameter(Acts::eBoundTime))};
                const float timeError{sqrt(static_cast<float>(covariance(Acts::eBoundTime, Acts::eBoundTime)))};
                const float theta(parameter[Acts::eBoundTheta]);
                const float phi(parameter[Acts::eBoundPhi]);
                const decltype(edm4eic::TrackPoint::directionError) directionError{
                        static_cast<float>(covariance(Acts::eBoundTheta, Acts::eBoundTheta)),
                        static_cast<float>(covariance(Acts::eBoundPhi, Acts::eBoundPhi)),
                        static_cast<float>(covariance(Acts::eBoundTheta, Acts::eBoundPhi))
                };
                const float pathLength = static_cast<float>(trackstate.pathLength());
                const float pathLengthError = 0;

                // Store track point
                track_segment.addToPoints({
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
                //m_log->debug("parameter[eBoundLoc0] = {}", parameter[Acts::eBoundLoc0]);
                //m_log->debug("parameter[eBoundLoc1] = {}", parameter[Acts::eBoundLoc1]);
                //m_log->debug("parameter[eBoundPhi] = {}", parameter[Acts::eBoundPhi]);
                //m_log->debug("parameter[eBoundTheta] = {}", parameter[Acts::eBoundTheta]);
                //m_log->debug("parameter[eBoundQOverP] = {}", parameter[Acts::eBoundQOverP]);
                //m_log->debug("parameter[eBoundTime] = {}", parameter[Acts::eBoundTime]);
                //m_log->debug("predicted variables: {}", trackstate.predicted());
            });

            m_log->debug("  Num calibrated state in trajectory {}", m_nCalibrated);
            m_log->debug("------ end of trajectory process ------");

            // Add to output collection
            track_segments.push_back(new edm4eic::TrackSegment(track_segment));
        }

        m_log->debug("END OF Track projector evnet process");
        return track_segments;
    }


} // namespace Jug::Reco
