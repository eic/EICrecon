// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Minjung Kim, Barak Schmookler
#pragma once

#include "algorithms/tracking/AmbiguitySolver.h"
#include "algorithms/tracking/AmbiguitySolverConfig.h"
#include "extensions/jana/JOmniFactory.h"
#include "extensions/spdlog/SpdlogMixin.h"
#include <ActsExamples/EventData/Track.hpp>
#include <JANA/JEvent.h>
#include <cassert>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace eicrecon {

class AmbiguitySolver_factory
    : public JOmniFactory<AmbiguitySolver_factory, AmbiguitySolverConfig> {

private:
  using AlgoT = eicrecon::AmbiguitySolver;
  std::unique_ptr<AlgoT> m_algo;

  Input<Acts::ConstVectorMultiTrajectory> m_acts_track_states_input{this};
  Input<Acts::ConstVectorTrackContainer> m_acts_tracks_input{this};
  PodioInput<edm4eic::Measurement2D> m_measurements_input{this};
  Output<Acts::ConstVectorMultiTrajectory> m_acts_track_states_output{this};
  Output<Acts::ConstVectorTrackContainer> m_acts_tracks_output{this};

  ParameterRef<std::uint32_t> m_maximumSharedHits{this, "maximumSharedHits",
                                                  config().maximum_shared_hits,
                                                  "Maximum number of shared hits allowed"};
  ParameterRef<std::uint32_t> m_maximumIterations{
      this, "maximumIterations", config().maximum_iterations, "Maximum number of iterations"};
  ParameterRef<std::size_t> m_nMeasurementsMin{
      this, "nMeasurementsMin", config().n_measurements_min,
      "Number of measurements required for further reconstruction"};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>();
    // TODO: convert AmbiguitySolver to inherit from algorithm::Algorithm
    // m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init(logger());
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    // FIXME clear output since it may not have been initialized or reset
    // See https://github.com/eic/EICrecon/issues/1961
    m_acts_track_states_output().clear();
    m_acts_tracks_output().clear();

    auto track_states_vec = m_acts_track_states_input();
    auto tracks_vec       = m_acts_tracks_input();
    assert(!track_states_vec.empty() && "ConstVectorMultiTrajectory vector should not be empty");
    assert(track_states_vec.front() != nullptr &&
           "ConstVectorMultiTrajectory pointer should not be null");
    assert(!tracks_vec.empty() && "ConstVectorTrackContainer vector should not be empty");
    assert(tracks_vec.front() != nullptr && "ConstVectorTrackContainer pointer should not be null");

    auto [output_track_states, output_tracks] =
        m_algo->process(track_states_vec, tracks_vec, *m_measurements_input());

    // Transfer ownership to output collections in a single, exception-safe operation
    m_acts_track_states_output() = std::move(output_track_states);
    m_acts_tracks_output()       = std::move(output_tracks);
  }
};

} // namespace eicrecon
