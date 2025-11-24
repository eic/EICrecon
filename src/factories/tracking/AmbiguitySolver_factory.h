// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Minjung Kim, Barak Schmookler
#pragma once

#include "algorithms/tracking/AmbiguitySolver.h"
#include "algorithms/tracking/AmbiguitySolverConfig.h"
#include "extensions/jana/JOmniFactory.h"
#include "extensions/spdlog/SpdlogMixin.h"
#include <ActsExamples/EventData/Track.hpp>
#include <JANA/JEvent.h>
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

  Input<ActsExamples::ConstTrackContainer> m_acts_tracks_input{this};
  PodioInput<edm4eic::Measurement2D> m_measurements_input{this};
  Output<ActsExamples::ConstTrackContainer> m_acts_tracks_output{this};
  Output<ActsExamples::Trajectories> m_acts_trajectories_output{this};

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
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init(logger());
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    // FIXME clear output since it may not have been initialized or reset
    // See https://github.com/eic/EICrecon/issues/1961
    m_acts_tracks_output().clear();
    m_acts_trajectories_output().clear();

    std::tie(m_acts_tracks_output(), m_acts_trajectories_output()) =
        m_algo->process(m_acts_tracks_input(), *m_measurements_input());
  }
};

} // namespace eicrecon
