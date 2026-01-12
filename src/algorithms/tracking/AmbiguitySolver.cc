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
#include <gsl/pointers>
#include <new>
#include <spdlog/common.h>
#include <string>
#include <utility>
#include <vector>

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

void AmbiguitySolver::init() {
  // Convert algorithm log level to Acts log level
  const auto spdlog_level = static_cast<spdlog::level::level_enum>(this->level());
  const auto acts_level   = eicrecon::SpdlogToActsLevel(spdlog_level);

  // Create Acts logger with appropriate level
  m_acts_logger = Acts::getDefaultLogger("AmbiguitySolver", acts_level);
  m_acts_cfg    = transformConfig(m_cfg);
  m_core = std::make_unique<Acts::GreedyAmbiguityResolution>(m_acts_cfg, acts_logger().clone());
}

void AmbiguitySolver::process(const Input& input, const Output& output) const {
  const auto [input_track_states, input_tracks] = input;
  auto [output_track_states, output_tracks]     = output;

  // Construct ConstTrackContainer from underlying containers
  auto trackStateContainer =
      std::make_shared<Acts::ConstVectorMultiTrajectory>(*input_track_states);
  auto trackContainer = std::make_shared<Acts::ConstVectorTrackContainer>(*input_tracks);
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

  // Output pointers are double-pointers (pointer to element in vector)
  // Dereference once to get the location where we construct the object
  // Use placement new since Acts const containers can't be assigned
  new (*output_track_states) Acts::ConstVectorMultiTrajectory(*input_track_states);
  new (*output_tracks) Acts::ConstVectorTrackContainer(std::move(solvedTracks.container()));
}

} // namespace eicrecon
