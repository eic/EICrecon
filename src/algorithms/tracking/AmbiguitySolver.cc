// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Minjung Kim, Barak Schmookler
#include "AmbiguitySolver.h"

#include <Acts/AmbiguityResolution/GreedyAmbiguityResolution.hpp>
#include <Acts/EventData/MeasurementHelpers.hpp>
#include <Acts/EventData/SourceLink.hpp>
#include <Acts/EventData/TrackStatePropMask.hpp>
#include <Acts/EventData/VectorMultiTrajectory.hpp>
#include <Acts/EventData/VectorTrackContainer.hpp>
#if Acts_VERSION_MAJOR < 43
#include <Acts/Utilities/Iterator.hpp>
#endif
#include <Acts/Utilities/Logger.hpp>
#include <ActsExamples/EventData/IndexSourceLink.hpp>
#include <ActsExamples/EventData/Track.hpp>
#include <ActsPlugins/EDM4hep/PodioTrackContainer.hpp>
#include <ActsPlugins/EDM4hep/PodioTrackStateContainer.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/container/vector.hpp>
#include <spdlog/common.h>
#include <Eigen/LU> // IWYU pragma: keep
#include <any>
#include <cstddef>
#include <gsl/pointers>
#include <string>
#include <utility>
#include <vector>

#include "algorithms/tracking/ActsGeometryProvider.h"
#include "algorithms/tracking/AmbiguitySolverConfig.h"
#include "algorithms/tracking/PodioGeometryIdConversionHelper.h"
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
  const auto [input_track_states, input_track_parameters, input_track_jacobians, input_tracks] =
      input;
  auto [output_track_states, output_track_parameters, output_track_jacobians, output_tracks] =
      output;

  // Create conversion helper for Podio backend
  PodioGeometryIdConversionHelper helper(m_geoSvc->getActsGeometryContext(),
                                         m_geoSvc->trackingGeometry());

  // Construct ConstPodioTrackContainer from input Podio collections
  ActsPlugins::ConstPodioTrackStateContainer<> inputTrackStateContainer(
      helper, Acts::ConstRefHolder<const ActsPodioEdm::TrackStateCollection>{*input_track_states},
      Acts::ConstRefHolder<const ActsPodioEdm::BoundParametersCollection>{*input_track_parameters},
      Acts::ConstRefHolder<const ActsPodioEdm::JacobianCollection>{*input_track_jacobians});
  ActsPlugins::ConstPodioTrackContainer<> inputTrackContainer(
      helper, Acts::ConstRefHolder<const ActsPodioEdm::TrackCollection>{*input_tracks});
  Acts::TrackContainer input_trks(inputTrackContainer, inputTrackStateContainer);

  // Run ambiguity resolution
  Acts::GreedyAmbiguityResolution::State state;
  m_core->computeInitialState(input_trks, state, &sourceLinkHash, &sourceLinkEquality);
  m_core->resolve(state);

  // Create mutable Podio containers for output
  ActsPlugins::MutablePodioTrackStateContainer<Acts::RefHolder> outputTrackStateContainer(
      helper, Acts::RefHolder{*output_track_states}, Acts::RefHolder{*output_track_parameters},
      Acts::RefHolder{*output_track_jacobians});
  ActsPlugins::MutablePodioTrackContainer<Acts::RefHolder> outputTrackContainer(
      helper, Acts::RefHolder{*output_tracks});
  Acts::TrackContainer solvedTracks(outputTrackContainer, outputTrackStateContainer);

  // FIXME: Ensure dynamic columns are created in the output track container
  // There is no viable conversion from 'ActsPlugins::ConstPodioTrackContainer const' to 'const MutablePodioTrackContainer'
  //solvedTracks.ensureDynamicColumns(input_trks);

  // Copy selected tracks to output
  for (auto iTrack : state.selectedTracks) {
    auto destProxy = solvedTracks.makeTrack();
    auto srcProxy  = input_trks.getTrack(state.trackTips.at(iTrack));
    destProxy.copyFrom(srcProxy);
  }
}

} // namespace eicrecon
