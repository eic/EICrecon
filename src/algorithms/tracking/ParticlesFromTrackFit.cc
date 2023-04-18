#include "ParticlesFromTrackFit.h"
#include <algorithm>


#include "DDRec/CellIDPositionConverter.h"
#include "DDRec/SurfaceManager.h"
#include "DDRec/Surface.h"



#include "Acts/EventData/MultiTrajectory.hpp"
#include "Acts/EventData/MultiTrajectoryHelpers.hpp"

// Event Model related classes
#include "edm4eic/ReconstructedParticleCollection.h"
#include "edm4eic/TrackerHitCollection.h"
#include "edm4eic/TrackParametersCollection.h"
#include "JugTrack/IndexSourceLink.hpp"
#include "JugTrack/Track.hpp"


#include "Acts/Utilities/Helpers.hpp"

#include "edm4eic/vector_utils.h"

#include <cmath>

void eicrecon::Reco::ParticlesFromTrackFit::init(std::shared_ptr<spdlog::logger> log) {
    m_log = log;
}

ParticlesFromTrackFitResultNew eicrecon::Reco::ParticlesFromTrackFit::execute(const std::vector<const eicrecon::TrackingResultTrajectory *> &trajectories) {

    // create output collections
    auto rec_parts = std::make_unique<edm4eic::ReconstructedParticleCollection >();
    auto track_pars = std::make_unique<edm4eic::TrackParametersCollection>();

    m_log->debug("Trajectories size: {}", std::size(trajectories));

    // Loop over the trajectories
    for (const auto& traj : trajectories) {

        // Get the entry index for the single trajectory
        // The trajectory entry indices and the multiTrajectory
        const auto& mj        = traj->multiTrajectory();
        const auto& trackTips = traj->tips();
        if (trackTips.empty()) {
            m_log->debug("Empty multiTrajectory.");
            continue;
        }

        const auto& trackTip = trackTips.front();

        // Collect the trajectory summary info
        auto trajState       = Acts::MultiTrajectoryHelpers::trajectoryState(mj, trackTip);
        //int  m_nMeasurements = trajState.nMeasurements;
        //int  m_nStates       = trajState.nStates;

        // Get the fitted track parameter
        //
        if (traj->hasTrackParameters(trackTip)) {
            const auto& boundParam = traj->trackParameters(trackTip);
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

            edm4eic::TrackParameters pars{
                    0, // type: track head --> 0
                    loc,
                    locError,
                    static_cast<float>(parameter[Acts::eBoundTheta]),
                    static_cast<float>(parameter[Acts::eBoundPhi]),
                    static_cast<float>(parameter[Acts::eBoundQOverP]),
                    momentumError,
                    static_cast<float>(parameter[Acts::eBoundTime]),
                    timeError,
                    static_cast<float>(boundParam.charge())};
            track_pars->push_back(pars);
        }

        auto tsize = trackTips.size();
        m_log->debug("# fitted parameters : {}", tsize);

        if (tsize == 0) {
            continue;
        }

        mj.visitBackwards(tsize - 1, [&](auto&& trackstate) {
            // debug() << trackstate.hasPredicted() << endmsg;
            // debug() << trackstate.predicted() << endmsg;
            auto params = trackstate.predicted(); //<< endmsg;

            double p0 = (1.0 / params[Acts::eBoundQOverP]) / Acts::UnitConstants::GeV;
            m_log->debug("track predicted p = {} GeV", p0);
            if (std::abs(p0) > 500) {
                m_log->debug("skipping!");
                return;
            }

            auto rec_part = rec_parts->create();
            rec_part.setMomentum(
                    edm4eic::sphericalToVector(
                            1.0 / std::abs(params[Acts::eBoundQOverP]),
                            params[Acts::eBoundTheta],
                            params[Acts::eBoundPhi])
            );
            rec_part.setCharge(static_cast<int16_t>(std::copysign(1., params[Acts::eBoundQOverP])));
        });
    }

    return std::make_pair(std::move(rec_parts), std::move(track_pars));
}
