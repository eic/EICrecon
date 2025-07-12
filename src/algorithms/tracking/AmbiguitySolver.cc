// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Minjung Kim, Barak Schmookler
#include "AmbiguitySolver.h"

#include <Acts/AmbiguityResolution/GreedyAmbiguityResolution.hpp>
#include <Acts/EventData/GenericBoundTrackParameters.hpp>
#include <Acts/EventData/MeasurementHelpers.hpp>
#include <Acts/EventData/MultiTrajectory.hpp>
#include <Acts/EventData/SourceLink.hpp>
#include <Acts/EventData/TrackContainer.hpp>
#include <Acts/EventData/TrackProxy.hpp>
#include <Acts/EventData/TrackStatePropMask.hpp>
#include <Acts/EventData/VectorMultiTrajectory.hpp>
#include <Acts/EventData/VectorTrackContainer.hpp>
#include <Acts/Surfaces/Surface.hpp>
#if (Acts_VERSION_MAJOR >= 37) && (Acts_VERSION_MAJOR < 43)
#include <Acts/Utilities/Iterator.hpp>
#endif
#include <ActsExamples/EventData/IndexSourceLink.hpp>
#include <ActsExamples/EventData/Track.hpp>
#include <ActsExamples/EventData/Trajectories.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/container/vector.hpp>
#include <edm4eic/Measurement2DCollection.h>
#include <Eigen/LU>
#include <any>
#include <cstddef>
#include <optional>
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

template <typename edm_t>
std::size_t AmbiguitySolver<edm_t>::sourceLinkHash(const Acts::SourceLink& a) {
  return static_cast<std::size_t>(a.get<ActsExamples::IndexSourceLink>().index());
}

template <typename edm_t>
bool AmbiguitySolver<edm_t>::sourceLinkEquality(const Acts::SourceLink& a,
                                                const Acts::SourceLink& b) {
  return a.get<ActsExamples::IndexSourceLink>().index() ==
         b.get<ActsExamples::IndexSourceLink>().index();
}

template <typename edm_t> void AmbiguitySolver<edm_t>::init(std::shared_ptr<spdlog::logger> log) {

  m_log         = log;
  m_acts_logger = eicrecon::getSpdlogLogger("AmbiguitySolver", m_log);
  m_acts_cfg    = transformConfig(m_cfg);
  m_core        = std::make_unique<Acts::GreedyAmbiguityResolution>(m_acts_cfg, logger().clone());
}

template <typename edm_t>
std::tuple<std::vector<typename edm_t::ConstTrackContainer*>,
           std::vector<typename edm_t::Trajectories*>>
AmbiguitySolver<edm_t>::process(
    std::vector<const typename edm_t::ConstTrackContainer*> input_container,
    const edm4eic::Measurement2DCollection& /* meas2Ds */) {

  // Assuming ActsExamples::ConstTrackContainer is compatible with Acts::ConstVectorTrackContainer
  // Create track container
  std::vector<typename edm_t::Trajectories*> output_trajectories;
  std::vector<typename edm_t::ConstTrackContainer*> output_tracks;

  if (input_container.empty()) {
    return std::make_tuple(std::move(output_tracks), std::move(output_trajectories));
  }

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
#if Acts_VERSION_MAJOR >= 44
    destProxy.copyFromWithoutStates(srcProxy);
#else
    destProxy.copyFrom(srcProxy, false);
#endif
    destProxy.tipIndex() = srcProxy.tipIndex();
  }

  output_tracks.push_back(new typename edm_t::ConstTrackContainer(
      std::make_shared<Acts::ConstVectorTrackContainer>(std::move(solvedTracks.container())),
      input_trks->trackStateContainerHolder()));

  //Make output trajectories
  typename edm_t::Trajectories::IndexedParameters parameters;
  std::vector<Acts::MultiTrajectoryTraits::IndexType> tips;

  for (const auto& track : *(output_tracks.front())) {

    tips.clear();
    parameters.clear();

    tips.push_back(track.tipIndex());
    parameters.emplace(std::pair{
        track.tipIndex(),
        ActsExamples::TrackParameters{track.referenceSurface().getSharedPtr(), track.parameters(),
                                      track.covariance(), track.particleHypothesis()}});

    output_trajectories.push_back(new typename edm_t::Trajectories(
        ((*output_tracks.front())).trackStateContainer(), tips, parameters));
  }

  return std::make_tuple(std::move(output_tracks), std::move(output_trajectories));
}

} // namespace eicrecon
