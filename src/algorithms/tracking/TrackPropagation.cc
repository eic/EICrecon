// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Wenqing Fan, Barak Schmookler, Whitney Armstrong, Sylvester Joosten, Dmitry Romanov

#include <cmath>
#include <algorithm>

#include "TrackPropagation.h"

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
#include "JugTrack/TrackingResultTrajectory.hpp"

#include "Acts/Utilities/Helpers.hpp"
#include "Acts/Geometry/GeometryIdentifier.hpp"
#include "Acts/MagneticField/ConstantBField.hpp"
#include "Acts/MagneticField/InterpolatedBFieldMap.hpp"
#include "Acts/MagneticField/SharedBField.hpp"
#include "Acts/Propagator/EigenStepper.hpp"
#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "Acts/Surfaces/DiscSurface.hpp"
#include "Acts/Surfaces/RadialBounds.hpp"

#include "ActsGeometryProvider.h"

#include "edm4eic/vector_utils.h"


#include <Acts/Geometry/TrackingGeometry.hpp>

#include "TrackPropagation.h"


namespace eicrecon {


    void TrackPropagation::init(std::shared_ptr<const ActsGeometryProvider> geo_svc,
                                std::shared_ptr<spdlog::logger> logger) {

        auto transform = Acts::Transform3::Identity();

        // make a reference disk to mimic electron-endcap HCal
        const auto hcalEndcapNZ = -3322.;
        const auto hcalEndcapNMinR = 83.01;
        const auto hcalEndcapNMaxR = 950;
        auto hcalEndcapNBounds = std::make_shared<Acts::RadialBounds>(hcalEndcapNMinR, hcalEndcapNMaxR);
        auto hcalEndcapNTrf = transform * Acts::Translation3(Acts::Vector3(0, 0, hcalEndcapNZ));
        hcalEndcapNSurf = Acts::Surface::makeShared<Acts::DiscSurface>(hcalEndcapNTrf, hcalEndcapNBounds);
        m_geoSvc = geo_svc;
        m_log = logger;
        m_log->debug("Initialized");
    }

    ActsTrackPropagationResult TrackPropagation::propagateTrack(const Acts::BoundTrackParameters &params,
                                                                const std::shared_ptr<const Acts::Surface> &targetSurf) {

        m_log->debug("  Propagating track to surface # {}", typeid(targetSurf->type()).name());

        std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry = m_geoSvc->trackingGeometry();
        std::shared_ptr<const Acts::MagneticFieldProvider> magneticField = m_geoSvc->getFieldProvider();
        using Stepper = Acts::EigenStepper<>;
        using Propagator = Acts::Propagator<Stepper>;
        Stepper stepper(magneticField);
        Propagator propagator(stepper);
        // Acts::Logging::Level logLevel = Acts::Logging::FATAL
        Acts::Logging::Level logLevel = Acts::Logging::INFO;

        ACTS_LOCAL_LOGGER(Acts::getDefaultLogger("ProjectTrack Logger", logLevel));

        Acts::PropagatorOptions<> options(m_geoContext, m_fieldContext, Acts::LoggerWrapper{logger()});

        auto result = propagator.propagate(params, *targetSurf, options);

        if (result.ok()) return std::move((*result).endParameters);
        return result.error();
    }



    std::vector<edm4eic::TrackSegment *>
    TrackPropagation::execute(std::vector<const Jug::TrackingResultTrajectory *> trajectories) {
        // output collection
        std::vector<edm4eic::TrackSegment *> track_segments;
        m_log->debug("Track propagation evnet process. Num of input trajectories: {}", std::size(trajectories));

        // Loop over the trajectories
        for (size_t traj_index = 0; traj_index < trajectories.size(); traj_index++) {
            auto &traj = trajectories[traj_index];

            // Get the entry index for the single trajectory
            // The trajectory entry indices and the multiTrajectory
            const auto &mj = traj->multiTrajectory();
            const auto &trackTips = traj->tips();

            m_log->debug("  Trajectory object # {}", traj_index);
            m_log->debug("  Number of elements in trackTips {}", trackTips.size());

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

            m_log->debug("  Num measurement in trajectory: {}", m_nMeasurements);
            m_log->debug("  Num states in trajectory     : {}", m_nStates);

            // output track segment
            edm4eic::MutableTrackSegment track_segment;

            //=================================================
            //Track projection
            //Reference sPHENIX code: https://github.com/sPHENIX-Collaboration/coresoftware/blob/335e6da4ccacc8374cada993485fe81d82e74a4f/offline/packages/trackreco/PHActsTrackProjection.h
            //=================================================
            const auto &boundParam = traj->trackParameters(trackTip);

            // project track parameters to electron endcap hcal surface
            auto result = propagateTrack(boundParam, hcalEndcapNSurf);

            // check propagation result
            if (!result.ok()) {
                m_log->debug("  propagation failed (!result.ok())");

            } else {
                m_log->debug("  propagation result is OK");
                auto trackStateParams = **result;
                auto projectionPos = trackStateParams.position(m_geoContext);

                m_log->debug("X projection is {}", projectionPos(0));
                m_log->debug("Y projection is {}", projectionPos(1));
                m_log->debug("Z projection is {}", projectionPos(2));

                const decltype(edm4eic::TrackPoint::position) proj_position{
                        static_cast<float>(projectionPos(0)),
                        static_cast<float>(projectionPos(1)),
                        static_cast<float>(projectionPos(2))
                };
                const decltype(edm4eic::TrackPoint::positionError) proj_positionError{0, 0, 0};
                const decltype(edm4eic::TrackPoint::momentum) proj_momentum{0, 0, 0};
                const decltype(edm4eic::TrackPoint::momentumError) proj_momentumError{0, 0, 0};
                const float proj_time = 0;
                const float proj_timeError = 0;
                const float proj_theta = 0;
                const float proj_phi = 0;
                const decltype(edm4eic::TrackPoint::directionError) proj_directionError{0, 0, 0};
                const float proj_pathLength = 0;
                const float proj_pathLengthError = 0;

                // Example of extracting errors!
                const auto& parameter  = boundParam.parameters();
                const auto& covariance = *boundParam.covariance();
                if (m_log->level() <= spdlog::level::debug) {
                    m_log->debug("loc 0 = {}", parameter[Acts::eBoundLoc0]);
                    m_log->debug("loc 1 = {}", parameter[Acts::eBoundLoc1]);
                    m_log->debug("phi   = {}", parameter[Acts::eBoundPhi]);
                    m_log->debug("theta = {}", parameter[Acts::eBoundTheta]);
                    m_log->debug("q/p   = {}", parameter[Acts::eBoundQOverP]);
                    m_log->debug("p     = {}", 1.0 / parameter[Acts::eBoundQOverP]);

                    m_log->debug("err phi = {}", sqrt(covariance(Acts::eBoundPhi, Acts::eBoundPhi)));
                    m_log->debug("err th  = {}", sqrt(covariance(Acts::eBoundTheta, Acts::eBoundTheta)));
                    m_log->debug("err q/p = {}", sqrt(covariance(Acts::eBoundQOverP, Acts::eBoundQOverP)));
                    m_log->debug("   chi2 = {}", trajState.chi2Sum);

                    m_log->debug("err x = {}", static_cast<float>(covariance(Acts::eBoundLoc0, Acts::eBoundLoc0)));
                    m_log->debug("err y = {}", static_cast<float>(covariance(Acts::eBoundLoc1, Acts::eBoundLoc1)));
                    m_log->debug("err z = {}", static_cast<float>(covariance(Acts::eBoundLoc0, Acts::eBoundLoc1)));
                }

                const decltype(edm4eic::TrackParametersData::loc) loc {
                        static_cast<float>(parameter[Acts::eBoundLoc0]),
                        static_cast<float>(parameter[Acts::eBoundLoc1])
                };
                const decltype(edm4eic::TrackParametersData::momentumError) momentumError {
                        static_cast<float>(covariance(Acts::eBoundTheta, Acts::eBoundTheta)),
                        static_cast<float>(covariance(Acts::eBoundPhi, Acts::eBoundPhi)),
                        static_cast<float>(covariance(Acts::eBoundQOverP, Acts::eBoundQOverP)),
                        static_cast<float>(covariance(Acts::eBoundTheta, Acts::eBoundPhi)),
                        static_cast<float>(covariance(Acts::eBoundTheta, Acts::eBoundQOverP)),
                        static_cast<float>(covariance(Acts::eBoundPhi, Acts::eBoundQOverP))};
                const decltype(edm4eic::TrackParametersData::locError) locError {
                        static_cast<float>(covariance(Acts::eBoundLoc0, Acts::eBoundLoc0)),
                        static_cast<float>(covariance(Acts::eBoundLoc1, Acts::eBoundLoc1)),
                        static_cast<float>(covariance(Acts::eBoundLoc0, Acts::eBoundLoc1))};
                const float timeError{sqrt(static_cast<float>(covariance(Acts::eBoundTime, Acts::eBoundTime)))};


                // Store projection point
                track_segment.addToPoints({
                                                  proj_position,
                                                  proj_positionError,
                                                  proj_momentum,
                                                  proj_momentumError,
                                                  proj_time,
                                                  proj_timeError,
                                                  proj_theta,
                                                  proj_phi,
                                                  proj_directionError,
                                                  proj_pathLength,
                                                  proj_pathLengthError
                                          });

            }

            // Set associated track
            // track_segment.setTrack(traj);

            // Add to output collection
            track_segments.push_back(new edm4eic::TrackSegment(track_segment));
        }

        return track_segments;
    }


} // namespace eicrecon

