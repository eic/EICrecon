// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Whitney Armstrong, Wouter Deconinck, Dmitry Romanov, Shujie Li, Dmitry Kalinkin

#include <Acts/EventData/MultiTrajectory.hpp>
#include <Acts/EventData/MultiTrajectoryHelpers.hpp>

#include "ActsToTracks.h"

namespace eicrecon {

// This array relates the Acts and EDM4eic covariance matrices, including
// the unit conversion to get from Acts units into EDM4eic units.
//
// Note: std::map is not constexpr, so we use a constexpr std::array
// std::array initialization need double braces since arrays are aggregates
// ref: https://en.cppreference.com/w/cpp/language/aggregate_initialization
static constexpr std::array<std::pair<Acts::BoundIndices, double>, 6> edm4eic_indexed_units{{
  {Acts::eBoundLoc0, Acts::UnitConstants::mm},
  {Acts::eBoundLoc1, Acts::UnitConstants::mm},
  {Acts::eBoundPhi, 1.},
  {Acts::eBoundTheta, 1.},
  {Acts::eBoundQOverP, 1. / Acts::UnitConstants::GeV},
  {Acts::eBoundTime, Acts::UnitConstants::ns}
}};

void ActsToTracks::init() {
}

void ActsToTracks::process(const Input& input, const Output& output) const {
  const auto [meas2Ds, acts_trajectories] = input;
  auto  [trajectories, track_parameters, tracks] = output;

        // Loop over trajectories
        for (const auto traj : acts_trajectories) {
          // The trajectory entry indices and the multiTrajectory
          const auto& trackTips = traj->tips();
          const auto& mj = traj->multiTrajectory();
          if (trackTips.empty()) {
            warning("Empty multiTrajectory.");
            continue;
          }

          // Loop over all trajectories in a multiTrajectory
          // FIXME: we only retain the first trackTips entry
          for (auto trackTip : decltype(trackTips){trackTips.front()}) {
            // Collect the trajectory summary info
            auto trajectoryState =
                Acts::MultiTrajectoryHelpers::trajectoryState(mj, trackTip);

            // Check if the reco track has fitted track parameters
            if (not traj->hasTrackParameters(trackTip)) {
              warning(
                  "No fitted track parameters for trajectory with entry index = {}",
                  trackTip);
              continue;
            }

            // Create trajectory
            auto trajectory = trajectories->create();
            trajectory.setNMeasurements(trajectoryState.nMeasurements);
            trajectory.setNStates(trajectoryState.nStates);
            trajectory.setNOutliers(trajectoryState.nOutliers);
            trajectory.setNHoles(trajectoryState.nHoles);
            trajectory.setNSharedHits(trajectoryState.nSharedHits);

            debug("trajectory state, measurement, outlier, hole: {} {} {} {}",
                trajectoryState.nStates,
                trajectoryState.nMeasurements,
                trajectoryState.nOutliers,
                trajectoryState.nHoles);

            for (const auto& measurementChi2 : trajectoryState.measurementChi2) {
                trajectory.addToMeasurementChi2(measurementChi2);
            }

            for (const auto& outlierChi2 : trajectoryState.outlierChi2) {
                trajectory.addToOutlierChi2(outlierChi2);
            }

            // Get the fitted track parameter
            const auto& boundParam = traj->trackParameters(trackTip);
            const auto& parameter  = boundParam.parameters();
            const auto& covariance = *boundParam.covariance();

            auto pars = track_parameters->create();
            pars.setType(0); // type: track head --> 0
            pars.setLoc({
                  static_cast<float>(parameter[Acts::eBoundLoc0]),
                  static_cast<float>(parameter[Acts::eBoundLoc1])
              });
            pars.setTheta(static_cast<float>(parameter[Acts::eBoundTheta]));
            pars.setPhi(static_cast<float>(parameter[Acts::eBoundPhi]));
            pars.setQOverP(static_cast<float>(parameter[Acts::eBoundQOverP]));
            pars.setTime(static_cast<float>(parameter[Acts::eBoundTime]));
            edm4eic::Cov6f cov;
            for (size_t i = 0; const auto& [a, x] : edm4eic_indexed_units) {
              for (size_t j = 0; const auto& [b, y] : edm4eic_indexed_units) {
                // FIXME why not pars.getCovariance()(i,j) = covariance(a,b) / x / y;
                cov(i,j) = covariance(a,b) / x / y;
              }
            }
            pars.setCovariance(cov);

            trajectory.addToTrackParameters(pars);

            // Fill tracks
            auto track = tracks->create();
            track.setType(                             // Flag that defines the type of track
              pars.getType()
            );
            track.setPosition(                         // Track 3-position at the vertex
              edm4hep::Vector3f()
            );
            track.setMomentum(                         // Track 3-momentum at the vertex [GeV]
              edm4hep::Vector3f()
            );
            track.setPositionMomentumCovariance(       // Covariance matrix in basis [x,y,z,px,py,pz]
              edm4eic::Cov6f()
            );
            track.setTime(                             // Track time at the vertex [ns]
              static_cast<float>(parameter[Acts::eBoundTime])
            );
            track.setTimeError(                        // Error on the track vertex time
              sqrt(static_cast<float>(covariance(Acts::eBoundTime, Acts::eBoundTime)))
            );
            track.setCharge(                           // Particle charge
              std::copysign(1., parameter[Acts::eBoundQOverP])
            );
            track.setChi2(trajectoryState.chi2Sum);    // Total chi2
            track.setNdf(trajectoryState.NDF);         // Number of degrees of freedom
            track.setPdg(                              // PDG particle ID hypothesis
              boundParam.particleHypothesis().absolutePdg()
            );
            track.setTrajectory(trajectory);           // Trajectory of this track

            // save measurement2d to good measurements or outliers according to srclink index
            // fix me: ideally, this should be integrated into multitrajectoryhelper
            // fix me: should say "OutlierMeasurements" instead of "OutlierHits" etc
            mj.visitBackwards(trackTip, [&](const auto& state) {

                auto geoID = state.referenceSurface().geometryId().value();
                auto typeFlags = state.typeFlags();

                // find the associated hit (2D measurement) with state sourcelink index
                // fix me: calibrated or not?
                if (state.hasUncalibratedSourceLink()) {

                    std::size_t srclink_index = state.getUncalibratedSourceLink().template get<ActsExamples::IndexSourceLink>().index();

                    // no hit on this state/surface, skip
                    if (typeFlags.test(Acts::TrackStateFlag::HoleFlag)) {
                        debug("No hit found on geo id={}", geoID);

                    } else {
                        auto meas2D = (*meas2Ds) [srclink_index];
                        if (typeFlags.test(Acts::TrackStateFlag::MeasurementFlag)) {
                          track.addToMeasurements(meas2D);
                          trajectory.addToMeasurements_deprecated(meas2D);
                          debug("Measurement on geo id={}, index={}, loc={},{}",
                                geoID, srclink_index, meas2D.getLoc().a, meas2D.getLoc().b);

                        }
                        else if (typeFlags.test(Acts::TrackStateFlag::OutlierFlag)) {
                          trajectory.addToOutliers_deprecated(meas2D);
                          debug("Outlier on geo id={}, index={}, loc={},{}",
                                geoID, srclink_index, meas2D.getLoc().a, meas2D.getLoc().b);

                        }
                    }
                }

            });

          }
        }
}

}
