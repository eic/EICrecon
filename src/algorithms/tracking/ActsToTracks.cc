// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 - 2025 Whitney Armstrong, Wouter Deconinck, Dmitry Romanov, Shujie Li, Dmitry Kalinkin

#include <Acts/Definitions/TrackParametrization.hpp>
#include <Acts/EventData/MultiTrajectoryHelpers.hpp>
#include <Acts/EventData/ParticleHypothesis.hpp>
#include <Acts/EventData/SourceLink.hpp>
#include <Acts/EventData/TrackContainer.hpp>
#include <Acts/EventData/TrackProxy.hpp>
#include <Acts/EventData/TrackStateType.hpp>
#include <Acts/EventData/VectorMultiTrajectory.hpp>
#include <Acts/Geometry/GeometryIdentifier.hpp>
#include <Acts/Surfaces/Surface.hpp>
#include <ActsExamples/EventData/IndexSourceLink.hpp>
#include <ActsExamples/EventData/Track.hpp>
#include <edm4eic/Cov6f.h>
#include <edm4eic/RawTrackerHit.h>
#include <edm4eic/TrackerHit.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/SimTrackerHit.h>
#include <edm4hep/Vector2f.h>
#include <edm4hep/Vector3f.h>
#include <podio/ObjectID.h>
#include <podio/RelationRange.h>
#include <Eigen/Core>
#include <any>
#include <array>
#include <cmath>
#include <cstddef>
#include <gsl/pointers>
#include <map>
#include <memory>
#include <numeric>
#include <vector>

#include "ActsToTracks.h"
#include "extensions/edm4eic/EDM4eicToActs.h"

namespace eicrecon {

// Custom comparator for MCParticle that uses deterministic ObjectID-based comparison
// instead of podio's default memory-address-based comparison
namespace {
  struct MCParticleCompare {
    bool operator()(const edm4hep::MCParticle& p_a, const edm4hep::MCParticle& p_b) const {
      // Compare particles by ObjectID for deterministic ordering
      auto id_a = p_a.getObjectID();
      auto id_b = p_b.getObjectID();
      if (id_a.collectionID != id_b.collectionID) {
        return id_a.collectionID < id_b.collectionID;
      }
      return id_a.index < id_b.index;
    }
  };
} // namespace

void ActsToTracks::init() {}

void ActsToTracks::process(const Input& input, const Output& output) const {
  const auto [meas2Ds, acts_track_states, acts_tracks, raw_hit_assocs] = input;
  auto [trajectories, track_parameters, tracks, tracks_assoc]          = output;

  // Construct ActsExamples::ConstTrackContainer from underlying containers
  auto trackStateContainer = std::make_shared<Acts::ConstVectorMultiTrajectory>(*acts_track_states);
  auto trackContainer      = std::make_shared<Acts::ConstVectorTrackContainer>(*acts_tracks);
  ActsExamples::ConstTrackContainer acts_track_container(trackContainer, trackStateContainer);

  // Loop over tracks
  for (const auto& track : acts_track_container) {
    // Collect the trajectory summary info
    auto trajectoryState = Acts::MultiTrajectoryHelpers::trajectoryState(
        acts_track_container.trackStateContainer(), track.tipIndex());

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
    const auto& parameter  = track.parameters();
    const auto& covariance = track.covariance();

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
    auto track_out = tracks->create();
    track_out.setType( // Flag that defines the type of track
        pars.getType());
    track_out.setPosition( // Track 3-position at the vertex
        edm4hep::Vector3f());
    track_out.setMomentum( // Track 3-momentum at the vertex [GeV]
        edm4hep::Vector3f());
    track_out.setPositionMomentumCovariance( // Covariance matrix in basis [x,y,z,px,py,pz]
        edm4eic::Cov6f());
    track_out.setTime( // Track time at the vertex [ns]
        static_cast<float>(parameter[Acts::eBoundTime]));
    track_out.setTimeError( // Error on the track vertex time
        sqrt(static_cast<float>(covariance(Acts::eBoundTime, Acts::eBoundTime))));
    track_out.setCharge( // Particle charge
        std::copysign(1., parameter[Acts::eBoundQOverP]));
    track_out.setChi2(trajectoryState.chi2Sum); // Total chi2
    track_out.setNdf(trajectoryState.NDF);      // Number of degrees of freedom
    track_out.setPdg(                           // PDG particle ID hypothesis
        track.particleHypothesis().absolutePdg());
    track_out.setTrajectory(trajectory); // Trajectory of this track

    // Determine track association with MCParticle, weighted by number of used measurements
    std::map<edm4hep::MCParticle, double, MCParticleCompare> mcparticle_weight_by_hit_count;

    // save measurement2d to good measurements or outliers according to srclink index
    // fix me: ideally, this should be integrated into multitrajectoryhelper
    // fix me: should say "OutlierMeasurements" instead of "OutlierHits" etc
    for (const auto& state : track.trackStatesReversed()) {
      auto geoID     = state.referenceSurface().geometryId().value();
      auto typeFlags = state.typeFlags();

      // find the associated hit (2D measurement) with state sourcelink index
      // fix me: calibrated or not?
      if (state.hasUncalibratedSourceLink()) {

        std::size_t srclink_index =
            state.getUncalibratedSourceLink().template get<ActsExamples::IndexSourceLink>().index();

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
            track_out.addToMeasurements(meas2D);
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
                  auto sim_hit     = raw_hit_assoc.getSimHit();
                  auto mc_particle = sim_hit.getParticle();
                  mcparticle_weight_by_hit_count[mc_particle]++;
                }
              }
            }
            //}
          }
        }
      }
    }

    // Store track associations if hit associations provided
    // FIXME: not able to check whether optional inputs were provided
    //if (raw_hit_assocs->has_value()) {
    double total_weight = std::accumulate(
        mcparticle_weight_by_hit_count.begin(), mcparticle_weight_by_hit_count.end(), 0,
        [](const double sum, const auto& i) { return sum + i.second; });
    for (const auto& [mcparticle, weight] : mcparticle_weight_by_hit_count) {
      auto track_assoc = tracks_assoc->create();
      track_assoc.setRec(track_out);
      track_assoc.setSim(mcparticle);
      double normalized_weight = weight / total_weight;
      track_assoc.setWeight(normalized_weight);
      debug("track {}: mcparticle {} weight {}", track_out.id().index, mcparticle.id().index,
            normalized_weight);
    }
    //}
  }
}

} // namespace eicrecon
