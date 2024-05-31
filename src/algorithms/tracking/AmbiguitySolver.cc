// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Minjung Kim
#include "AmbiguitySolver.h"
#include "AmbiguitySolverConfig.h"

#include <Acts/AmbiguityResolution/GreedyAmbiguityResolution.hpp>
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
#include <ActsExamples/EventData/Track.hpp>
#include <ActsExamples/EventData/Trajectories.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/container/vector.hpp>
#include <edm4eic/Cov6f.h>
#include <edm4eic/Measurement2DCollection.h>
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

Acts::GreedyAmbiguityResolution::Config
transformConfig(const eicrecon::AmbiguitySolverConfig& cfg) {
  Acts::GreedyAmbiguityResolution::Config result;
  result.maximumSharedHits = cfg.maximum_shared_hits;
  result.maximumIterations = cfg.maximum_iterations;
  result.nMeasurementsMin  = cfg.n_measurements_min;
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


std::tuple<std::vector<ActsExamples::ConstTrackContainer*>, std::vector<ActsExamples::Trajectories*>>
AmbiguitySolver::process(std::vector<const ActsExamples::ConstTrackContainer*> input_container,
                         std::vector<const ActsExamples::Trajectories*> input_traj,
                         const edm4eic::Measurement2DCollection& meas2Ds) {

  // Assuming ActsExamples::ConstTrackContainer is compatible with Acts::ConstVectorTrackContainer
  // Create track container
  std::vector<ActsExamples::Trajectories*> output_trajectories;
  std::vector<ActsExamples::ConstTrackContainer*> output_tracks;

  auto& input_trks = input_container.front();
  Acts::GreedyAmbiguityResolution::State state;
  m_core->computeInitialState(*input_trks, state, &sourceLinkHash, &sourceLinkEquality);
  m_core->resolve(state);

  ActsExamples::TrackContainer solvedTracks{std::make_shared<Acts::VectorTrackContainer>(),
                                            std::make_shared<Acts::VectorMultiTrajectory>()};
  solvedTracks.ensureDynamicColumns(*input_trks);

  for (auto iTrack : state.selectedTracks) {

	auto destProxy = solvedTracks.getTrack(solvedTracks.addTrack());
      	auto srcProxy  = input_trks->getTrack(state.trackTips.at(iTrack));
      	destProxy.copyFrom(srcProxy, false);
      	destProxy.tipIndex() = srcProxy.tipIndex();

   }
 
   output_tracks.push_back(new ActsExamples::ConstTrackContainer(
	std::make_shared<Acts::ConstVectorTrackContainer>(std::move(solvedTracks.container())),
        input_trks->trackStateContainerHolder()));
  
   //Make output trajectories
   ActsExamples::Trajectories::IndexedParameters parameters;
   std::vector<Acts::MultiTrajectoryTraits::IndexType> tips;

   for (const auto& track : *(output_tracks.front())) {

	tips.clear();
        parameters.clear();	

	tips.push_back(track.tipIndex());
	parameters.emplace(
           std::pair{track.tipIndex(),
                    ActsExamples::TrackParameters{track.referenceSurface().getSharedPtr(),
                                                  track.parameters(), track.covariance(),
                                                  track.particleHypothesis()}});

	output_trajectories.push_back(new ActsExamples::Trajectories(
             ((*output_tracks.front())).trackStateContainer(),
             tips, parameters));

   }

  return std::make_tuple(std::move(output_tracks), std::move(output_trajectories));
}

} // namespace eicrecon
