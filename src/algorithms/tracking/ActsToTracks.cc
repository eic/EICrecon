// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 - 2025 Whitney Armstrong, Wouter Deconinck, Dmitry Romanov, Shujie Li, Dmitry Kalinkin

#include <Acts/Definitions/TrackParametrization.hpp>
#include <Acts/EventData/MultiTrajectoryHelpers.hpp>
#include <Acts/EventData/ParticleHypothesis.hpp>
#include <Acts/EventData/TrackStateType.hpp>
#include <ActsExamples/EventData/IndexSourceLink.hpp>
#include <edm4eic/Cov6f.h>
#include <edm4eic/RawTrackerHit.h>
#include <edm4eic/TrackerHit.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/SimTrackerHit.h>
#include <edm4hep/Vector2f.h>
#include <edm4hep/Vector3f.h>
#include <fmt/core.h>
#include <podio/ObjectID.h>
#include <podio/RelationRange.h>
#include <Eigen/Core>
#include <any>
#include <array>
#include <cmath>
#include <cstddef>
#include <gsl/pointers>
#include <map>
#include <numeric>
#include <optional>

#include "ActsToTracks.h"
#include "extensions/edm4eic/EDM4eicToActs.h"

namespace eicrecon {

void ActsToTracks::init() {}

void ActsToTracks::process(const Input& input, const Output& output) const {
  const auto [meas2Ds, acts_trajectories, raw_hit_assocs]     = input;
  auto [trajectories, track_parameters, tracks, tracks_assoc] = output;

  // Loop over trajectories
  for (const auto traj : acts_trajectories) {
    // The trajectory entry indices and the multiTrajectory
    const auto& trackTips = traj->tips();
    const auto& mj        = traj->multiTrajectory();
    if (trackTips.empty()) {
      warning("Empty multiTrajectory.");
      continue;
    }

    // Loop over all trajectories in a multiTrajectory
    for (auto trackTip : trackTips) {
      // Collect the trajectory summary info
      auto trajectoryState = Acts::MultiTrajectoryHelpers::trajectoryState(mj, trackTip);

      // Check if the reco track has fitted track parameters
      if (not traj->hasTrackParameters(trackTip)) {
        warning("No fitted track parameters for trajectory with entry index = {}", trackTip);
        continue;
      }

      // Create trajectory
      auto trajectory = trajectories->create();
      trajectory.setNMeasurements(trajectoryState.nMeasurements);
      trajectory.setNStates(trajectoryState.nStates);
      trajectory.setNOutliers(trajectoryState.nOutliers);
      trajectory.setNHoles(trajectoryState.nHoles);
      trajectory.setNSharedHits(trajectoryState.nSharedHits);

      debug("trajectory state, measurement, outlier, hole: {} {} {} {}", trajectoryState.nStates,
            trajectoryState.nMeasurements, trajectoryState.nOutliers, trajectoryState.nHoles);

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
      pars.setLoc({static_cast<float>(parameter[Acts::eBoundLoc0]),
                   static_cast<float>(parameter[Acts::eBoundLoc1])});
      pars.setTheta(static_cast<float>(parameter[Acts::eBoundTheta]));
      pars.setPhi(static_cast<float>(parameter[Acts::eBoundPhi]));
      pars.setQOverP(static_cast<float>(parameter[Acts::eBoundQOverP]));
      pars.setTime(static_cast<float>(parameter[Acts::eBoundTime]));
      edm4eic::Cov6f cov;
      for (std::size_t i = 0; const auto& [a, x] : edm4eic_indexed_units) {
        for (std::size_t j = 0; const auto& [b, y] : edm4eic_indexed_units) {
          // FIXME why not pars.getCovariance()(i,j) = covariance(a,b) / x / y;
          cov(i, j) = covariance(a, b) / x / y;
          ++j;
        }
        ++i;
      }
      pars.setCovariance(cov);

      trajectory.addToTrackParameters(pars);

      // Fill tracks
      auto track = tracks->create();
      track.setType( // Flag that defines the type of track
          pars.getType());
      track.setPosition( // Track 3-position at the vertex
          edm4hep::Vector3f());
      track.setMomentum( // Track 3-momentum at the vertex [GeV]
          edm4hep::Vector3f());
      track.setPositionMomentumCovariance( // Covariance matrix in basis [x,y,z,px,py,pz]
          edm4eic::Cov6f());
      track.setTime( // Track time at the vertex [ns]
          static_cast<float>(parameter[Acts::eBoundTime]));
      track.setTimeError( // Error on the track vertex time
          sqrt(static_cast<float>(covariance(Acts::eBoundTime, Acts::eBoundTime))));
      track.setCharge( // Particle charge
          std::copysign(1., parameter[Acts::eBoundQOverP]));
      track.setChi2(trajectoryState.chi2Sum); // Total chi2
      track.setNdf(trajectoryState.NDF);      // Number of degrees of freedom
      track.setPdg(                           // PDG particle ID hypothesis
          boundParam.particleHypothesis().absolutePdg());
      track.setTrajectory(trajectory); // Trajectory of this track

      // Determine track association with MCParticle, weighted by number of used measurements
      std::map<edm4hep::MCParticle, double> mcparticle_weight_by_hit_count;

      // save measurement2d to good measurements or outliers according to srclink index
      // fix me: ideally, this should be integrated into multitrajectoryhelper
      // fix me: should say "OutlierMeasurements" instead of "OutlierHits" etc
      mj.visitBackwards(trackTip, [&](const auto& state) {
        auto geoID     = state.referenceSurface().geometryId().value();
        auto typeFlags = state.typeFlags();

        // find the associated hit (2D measurement) with state sourcelink index
        // fix me: calibrated or not?
        if (state.hasUncalibratedSourceLink()) {

          std::size_t srclink_index = state.getUncalibratedSourceLink()
                                          .template get<ActsExamples::IndexSourceLink>()
                                          .index();

          // no hit on this state/surface, skip
          if (typeFlags.test(Acts::TrackStateFlag::HoleFlag)) {
            debug("No hit found on geo id={}", geoID);

          } else {
            auto meas2D = (*meas2Ds)[srclink_index];
            if (typeFlags.test(Acts::TrackStateFlag::OutlierFlag)) {
              trajectory.addToOutliers_deprecated(meas2D);
              debug("Outlier on geo id={}, index={}, loc={},{}", geoID, srclink_index,
                    meas2D.getLoc().a, meas2D.getLoc().b);
            } else if (typeFlags.test(Acts::TrackStateFlag::MeasurementFlag)) {
              track.addToMeasurements(meas2D);
              trajectory.addToMeasurements_deprecated(meas2D);
              debug("Measurement on geo id={}, index={}, loc={},{}", geoID, srclink_index,
                    meas2D.getLoc().a, meas2D.getLoc().b);

              // Determine track associations if hit associations provided
              // FIXME: not able to check whether optional inputs were provided
              //if (raw_hit_assocs->has_value()) {
              for (const auto& hit : meas2D.getHits()) {
                auto raw_hit = hit.getRawHit();
                for (const auto raw_hit_assoc : *raw_hit_assocs) {
                  if (raw_hit_assoc.getRawHit() == raw_hit) {
                    auto sim_hit = raw_hit_assoc.getSimHit();
                    auto mc_particle = sim_hit.getParticle();
                    mcparticle_weight_by_hit_count[mc_particle]++;
                  }
                }
              }
              //}
            }
          }
        }
      });

      // Store track associations if hit associations provided
      // FIXME: not able to check whether optional inputs were provided
      //if (raw_hit_assocs->has_value()) {
      double total_weight = std::accumulate(
          mcparticle_weight_by_hit_count.begin(), mcparticle_weight_by_hit_count.end(), 0,
          [](const double sum, const auto& i) { return sum + i.second; });
      for (const auto& [mcparticle, weight] : mcparticle_weight_by_hit_count) {
        auto track_assoc = tracks_assoc->create();
        track_assoc.setRec(track);
        track_assoc.setSim(mcparticle);
        double normalized_weight = weight / total_weight;
        track_assoc.setWeight(normalized_weight);
        debug("track {}: mcparticle {} weight {}", track.id().index, mcparticle.id().index,
              normalized_weight);
      }
      //}
    }
  }
}

} // namespace eicrecon
