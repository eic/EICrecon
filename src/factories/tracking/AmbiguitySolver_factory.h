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

  PodioInput<ActsPodioEdm::TrackState> m_acts_track_states_input{this};
  PodioInput<ActsPodioEdm::BoundParameters> m_acts_track_parameters_input{this};
  PodioInput<ActsPodioEdm::Jacobian> m_acts_track_jacobians_input{this};
  PodioInput<ActsPodioEdm::Track> m_acts_tracks_input{this};
  PodioOutput<ActsPodioEdm::TrackState> m_acts_track_states_output{this};
  PodioOutput<ActsPodioEdm::BoundParameters> m_acts_track_parameters_output{this};
  PodioOutput<ActsPodioEdm::Jacobian> m_acts_track_jacobians_output{this};
  PodioOutput<ActsPodioEdm::Track> m_acts_tracks_output{this};

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
    m_algo = std::make_unique<AlgoT>(this->GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process(
        AlgoT::Input{m_acts_track_states_input(), m_acts_track_parameters_input(),
                     m_acts_track_jacobians_input(), m_acts_tracks_input()},
        AlgoT::Output{m_acts_track_states_output().get(), m_acts_track_parameters_output().get(),
                      m_acts_track_jacobians_output().get(), m_acts_tracks_output().get()});
  }
};

} // namespace eicrecon
