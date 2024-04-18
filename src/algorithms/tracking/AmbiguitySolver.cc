// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Minjung Kim, Barak Schmookler,, Wouter Deconinck, Dmitry Romanov, Shujie Li
/*
 * Reco Track Filtering Based on Greedy ambiguity resolution solver adopted from ACTS
 * 
 *
 *  Author: Minjung Kim (LBL, minjung.kim@lbl.gov) 
*/
#include "AmbiguitySolver.h"

#include "Acts/AmbiguityResolution/GreedyAmbiguityResolution.hpp"
#include "AmbiguitySolverConfig.h"
#if EDM4EIC_VERSION_MAJOR >= 5
#include <edm4eic/Cov6f.h>
#endif
#include <Acts/Definitions/TrackParametrization.hpp>
#include <Acts/Definitions/Units.hpp>
#include <Acts/EventData/MultiTrajectoryHelpers.hpp>
#include <Acts/EventData/ParticleHypothesis.hpp>
#include <Acts/EventData/SourceLink.hpp>
#include <Acts/EventData/TrackContainer.hpp>
#include <Acts/EventData/TrackStatePropMask.hpp>
#include <Acts/EventData/TrackStateType.hpp>
#include <Acts/EventData/VectorMultiTrajectory.hpp>
#include <Acts/EventData/VectorTrackContainer.hpp>
#include <ActsExamples/EventData/IndexSourceLink.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/container/vector.hpp>
#include <edm4eic/Cov6f.h>
#include <edm4eic/EDM4eicVersion.h>
#include <edm4eic/Measurement2DCollection.h>
#include <edm4eic/TrackParametersCollection.h>
#include <edm4eic/TrajectoryCollection.h>
#include <edm4hep/Vector2f.h>
#include <edm4hep/Vector3f.h>
#include <fmt/core.h>
#include <Eigen/Core>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <optional>
#include <utility>

#include "Acts/Utilities/Logger.hpp"
#include "extensions/spdlog/SpdlogFormatters.h" // IWYU pragma: keep
#include "extensions/spdlog/SpdlogToActs.h"

namespace eicrecon {
using namespace Acts::UnitLiterals;

#if EDM4EIC_VERSION_MAJOR >= 5
// This array relates the Acts and EDM4eic covariance matrices, including
// the unit conversion to get from Acts units into EDM4eic units.
//
// Note: std::map is not constexpr, so we use a constexpr std::array
// std::array initialization need double braces since arrays are aggregates
// ref: https://en.cppreference.com/w/cpp/language/aggregate_initialization
static constexpr std::array<std::pair<Acts::BoundIndices, double>, 6> edm4eic_indexed_units{
    {{Acts::eBoundLoc0, Acts::UnitConstants::mm},
     {Acts::eBoundLoc1, Acts::UnitConstants::mm},
     {Acts::eBoundPhi, 1.},
     {Acts::eBoundTheta, 1.},
     {Acts::eBoundQOverP, 1. / Acts::UnitConstants::GeV},
     {Acts::eBoundTime, Acts::UnitConstants::ns}}};
#endif
Acts::GreedyAmbiguityResolution::Config
transformConfig(const eicrecon::AmbiguitySolverConfig& cfg) {
  Acts::GreedyAmbiguityResolution::Config result;
  result.maximumSharedHits = cfg.m_maximumSharedHits;
  result.maximumIterations = cfg.m_maximumIterations;
  result.nMeasurementsMin  = cfg.m_nMeasurementsMin;
  return result;
}

std::size_t sourceLinkHash(const Acts::SourceLink& a) {
  return static_cast<std::size_t>(a.get<ActsExamples::IndexSourceLink>().index());
}

bool sourceLinkEquality(const Acts::SourceLink& a, const Acts::SourceLink& b) {
  return a.get<ActsExamples::IndexSourceLink>().index() ==
         b.get<ActsExamples::IndexSourceLink>().index();
}

AmbiguitySolver::AmbiguitySolver() {}

void AmbiguitySolver::init(std::shared_ptr<spdlog::logger> log) {

  m_log         = log;
  m_acts_logger = eicrecon::getSpdlogLogger("AmbiguitySolver", m_log);
  m_acts_cfg    = transformConfig(m_cfg);
  m_core        = std::make_unique<Acts::GreedyAmbiguityResolution>(m_acts_cfg, logger().clone());
}

std::tuple<
    std::unique_ptr<edm4eic::TrajectoryCollection>,
    std::unique_ptr<edm4eic::TrackParametersCollection>, std::unique_ptr<edm4eic::TrackCollection>,
    std::vector<ActsExamples::ConstTrackContainer*>, std::vector<ActsExamples::Trajectories*>>
AmbiguitySolver::process(std::vector<const ActsExamples::ConstTrackContainer*> input_container,
                         std::vector<const ActsExamples::Trajectories*> input_traj,
                         const edm4eic::Measurement2DCollection& meas2Ds) {
  // Assuming ActsExamples::ConstTrackContainer is compatible with Acts::ConstVectorTrackContainer
  // Create track container
  std::vector<ActsExamples::Trajectories*> output_trajectories;
  std::vector<ActsExamples::ConstTrackContainer*> output_tracks;
  auto track_parameters = std::make_unique<edm4eic::TrackParametersCollection>();
  auto trajectories     = std::make_unique<edm4eic::TrajectoryCollection>();
  auto tracks           = std::make_unique<edm4eic::TrackCollection>();

  std::vector<int> idxlist;

  for (const auto& input_trks : input_container) {

    Acts::GreedyAmbiguityResolution::State state;
    m_core->computeInitialState(*input_trks, state, &sourceLinkHash, &sourceLinkEquality);
    m_core->resolve(state);

    ActsExamples::TrackContainer solvedTracks{std::make_shared<Acts::VectorTrackContainer>(),
                                              std::make_shared<Acts::VectorMultiTrajectory>()};
    solvedTracks.ensureDynamicColumns(*input_trks);

    for (auto iTrack : state.selectedTracks) {
      auto destProxy = solvedTracks.getTrack(solvedTracks.addTrack());
      auto srcProxy  = input_trks->getTrack(state.trackTips.at(iTrack));
      idxlist.push_back(srcProxy.tipIndex());
      destProxy.copyFrom(srcProxy, false);
      destProxy.tipIndex() = srcProxy.tipIndex();

      for (auto* traj : input_traj) {
        const auto& trackTips = traj->tips();
        if (trackTips.empty()) {
          m_log->warn("Empty multiTrajectory.");
          continue;
        }
        if (std::find(trackTips.begin(), trackTips.end(), destProxy.tipIndex()) !=
            trackTips.end()) {
          output_trajectories.push_back(new ActsExamples::Trajectories(*traj));
        }
      }
    }
    output_tracks.push_back(new ActsExamples::ConstTrackContainer(
        std::make_shared<Acts::ConstVectorTrackContainer>(std::move(solvedTracks.container())),
        input_trks->trackStateContainerHolder()));
  }

  // Loop over trajectories
  for (const auto* traj : output_trajectories) {
    // The trajectory entry indices and the multiTrajectory
    const auto& trackTips = traj->tips();
    const auto& mj        = traj->multiTrajectory();
    if (trackTips.empty()) {
      m_log->warn("Empty multiTrajectory.");
      continue;
    }

    // Loop over all trajectories in a multiTrajectory
    // FIXME: we only retain the first trackTips entry
    for (auto trackTip : trackTips) {
      if (std::find(idxlist.begin(), idxlist.end(), trackTip) == idxlist.end())
        continue;
      // Collect the trajectory summary info
      auto trajectoryState = Acts::MultiTrajectoryHelpers::trajectoryState(mj, trackTip);

      // Check if the reco track has fitted track parameters
      if (not traj->hasTrackParameters(trackTip)) {
        m_log->warn("No fitted track parameters for trajectory with entry index = {}", trackTip);
        continue;
      }

      // Create trajectory
      auto trajectory = trajectories->create();
#if EDM4EIC_VERSION_MAJOR < 5
      trajectory.setChi2(trajectoryState.chi2Sum);
      trajectory.setNdf(trajectoryState.NDF);
#endif
      trajectory.setNMeasurements(trajectoryState.nMeasurements);
      trajectory.setNStates(trajectoryState.nStates);
      trajectory.setNOutliers(trajectoryState.nOutliers);
      trajectory.setNHoles(trajectoryState.nHoles);
      trajectory.setNSharedHits(trajectoryState.nSharedHits);

      m_log->debug("trajectory state, measurement, outlier, hole: {} {} {} {}",
                   trajectoryState.nStates, trajectoryState.nMeasurements,
                   trajectoryState.nOutliers, trajectoryState.nHoles);

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
#if EDM4EIC_VERSION_MAJOR >= 5
      edm4eic::Cov6f cov;
      for (size_t i = 0; const auto& [a, x] : edm4eic_indexed_units) {
        for (size_t j = 0; const auto& [b, y] : edm4eic_indexed_units) {
          // FIXME why not pars.getCovariance()(i,j) = covariance(a,b) / x / y;
          cov(i, j) = covariance(a, b) / x / y;
        }
      }
      pars.setCovariance(cov);
#else
      pars.setCharge(static_cast<float>(boundParam.charge()));
      pars.setLocError({static_cast<float>(covariance(Acts::eBoundLoc0, Acts::eBoundLoc0)),
                        static_cast<float>(covariance(Acts::eBoundLoc1, Acts::eBoundLoc1)),
                        static_cast<float>(covariance(Acts::eBoundLoc0, Acts::eBoundLoc1))});
      pars.setMomentumError({static_cast<float>(covariance(Acts::eBoundTheta, Acts::eBoundTheta)),
                             static_cast<float>(covariance(Acts::eBoundPhi, Acts::eBoundPhi)),
                             static_cast<float>(covariance(Acts::eBoundQOverP, Acts::eBoundQOverP)),
                             static_cast<float>(covariance(Acts::eBoundTheta, Acts::eBoundPhi)),
                             static_cast<float>(covariance(Acts::eBoundTheta, Acts::eBoundQOverP)),
                             static_cast<float>(covariance(Acts::eBoundPhi, Acts::eBoundQOverP))});
      pars.setTimeError(sqrt(static_cast<float>(covariance(Acts::eBoundTime, Acts::eBoundTime))));
#endif

      trajectory.addToTrackParameters(pars);

// Fill tracks
#if EDM4EIC_VERSION_MAJOR >= 5
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
#endif
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
            m_log->debug("No hit found on geo id={}", geoID);

          } else {
            auto meas2D = meas2Ds[srclink_index];
            if (typeFlags.test(Acts::TrackStateFlag::MeasurementFlag)) {
#if EDM4EIC_VERSION_MAJOR >= 5
              track.addToMeasurements(meas2D);
              trajectory.addToMeasurements_deprecated(meas2D);
#else
                            trajectory.addToMeasurementHits(meas2D);
#endif
              m_log->debug("Measurement on geo id={}, index={}, loc={},{}", geoID, srclink_index,
                           meas2D.getLoc().a, meas2D.getLoc().b);

            } else if (typeFlags.test(Acts::TrackStateFlag::OutlierFlag)) {
#if EDM4EIC_VERSION_MAJOR >= 5
              trajectory.addToOutliers_deprecated(meas2D);
#else
                            trajectory.addToOutlierHits(meas2D);
#endif
              m_log->debug("Outlier on geo id={}, index={}, loc={},{}", geoID, srclink_index,
                           meas2D.getLoc().a, meas2D.getLoc().b);
            }
          }
        }
      });
    }
  }

  return std::make_tuple(std::move(trajectories), std::move(track_parameters), std::move(tracks),
                         std::move(output_tracks), std::move(output_trajectories));
}

} // namespace eicrecon
