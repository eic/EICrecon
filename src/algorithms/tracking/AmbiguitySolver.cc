// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Minjung Kim, Barak Schmookler
#include "AmbiguitySolver.h"

#include <Acts/AmbiguityResolution/GreedyAmbiguityResolution.hpp>
#include <Acts/EventData/SourceLink.hpp>
#include <Acts/EventData/VectorMultiTrajectory.hpp>
#include <Acts/EventData/VectorTrackContainer.hpp>
#if (Acts_VERSION_MAJOR >= 37) && (Acts_VERSION_MAJOR < 43)
#include <Acts/Utilities/Iterator.hpp>
#endif
#include <ActsExamples/EventData/IndexSourceLink.hpp>
#include <ActsExamples/EventData/Track.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/container/vector.hpp>
#include <any>
#include <cstddef>
#include <string>
#include <utility>

#include "Acts/Utilities/Logger.hpp"
#include "AmbiguitySolverConfig.h"
#include "extensions/spdlog/SpdlogFormatters.h" // IWYU pragma: keep
#include "extensions/spdlog/SpdlogToActs.h"

namespace eicrecon {

Acts::GreedyAmbiguityResolution::Config
transformConfig(const eicrecon::AmbiguitySolverConfig& cfg) {
  Acts::GreedyAmbiguityResolution::Config result;
  result.maximumSharedHits = cfg.maximum_shared_hits;
  result.maximumIterations = cfg.maximum_iterations;
  result.nMeasurementsMin  = cfg.n_measurements_min;
  return result;
}

static std::size_t sourceLinkHash(const Acts::SourceLink& a) {
  return static_cast<std::size_t>(a.get<ActsExamples::IndexSourceLink>().index());
}

static bool sourceLinkEquality(const Acts::SourceLink& a, const Acts::SourceLink& b) {
  return a.get<ActsExamples::IndexSourceLink>().index() ==
         b.get<ActsExamples::IndexSourceLink>().index();
}

AmbiguitySolver::AmbiguitySolver() = default;

void AmbiguitySolver::init(std::shared_ptr<spdlog::logger> log) {

  m_log         = log;
  m_acts_logger = eicrecon::getSpdlogLogger("AmbiguitySolver", m_log);
  m_acts_cfg    = transformConfig(m_cfg);
  m_core        = std::make_unique<Acts::GreedyAmbiguityResolution>(m_acts_cfg, logger().clone());
}

std::tuple<std::vector<Acts::ConstVectorMultiTrajectory*>,
           std::vector<Acts::ConstVectorTrackContainer*>>
AmbiguitySolver::process(std::vector<const Acts::ConstVectorMultiTrajectory*> input_track_states,
                         std::vector<const Acts::ConstVectorTrackContainer*> input_tracks) {

  // Create output vectors
  std::vector<Acts::ConstVectorMultiTrajectory*> output_track_states;
  std::vector<Acts::ConstVectorTrackContainer*> output_tracks;

  if (input_track_states.empty() || input_tracks.empty()) {
    return {output_track_states, output_tracks};
  }

  // Construct ConstTrackContainer from underlying containers
  auto trackStateContainer =
      std::make_shared<Acts::ConstVectorMultiTrajectory>(*input_track_states.front());
  auto trackContainer = std::make_shared<Acts::ConstVectorTrackContainer>(*input_tracks.front());
  ActsExamples::ConstTrackContainer input_trks(trackContainer, trackStateContainer);

  Acts::GreedyAmbiguityResolution::State state;
  m_core->computeInitialState(input_trks, state, &sourceLinkHash, &sourceLinkEquality);
  m_core->resolve(state);

  ActsExamples::TrackContainer solvedTracks{std::make_shared<Acts::VectorTrackContainer>(),
                                            std::make_shared<Acts::VectorMultiTrajectory>()};
  solvedTracks.ensureDynamicColumns(input_trks);

  for (auto iTrack : state.selectedTracks) {

    auto destProxy = solvedTracks.getTrack(solvedTracks.addTrack());
    auto srcProxy  = input_trks.getTrack(state.trackTips.at(iTrack));
#if Acts_VERSION_MAJOR >= 44
    destProxy.copyFromWithoutStates(srcProxy);
#else
    destProxy.copyFrom(srcProxy, false);
#endif
    destProxy.tipIndex() = srcProxy.tipIndex();
  }

  // Move track states and track container to const containers and return as separate vectors
  output_track_states.push_back(new Acts::ConstVectorMultiTrajectory(*input_track_states.front()));
  output_tracks.push_back(new Acts::ConstVectorTrackContainer(std::move(solvedTracks.container())));

  return {output_track_states, output_tracks};
}

} // namespace eicrecon
