// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Minjung Kim
#pragma once

#include "algorithms/tracking/AmbiguitySolver.h"
#include "algorithms/tracking/AmbiguitySolverConfig.h"
#include "extensions/jana/JOmniFactory.h"
#include "extensions/spdlog/SpdlogMixin.h"
#include <ActsExamples/EventData/Track.hpp>
#include <JANA/JEvent.h>
#include <edm4eic/TrackParametersCollection.h>
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

  Input<ActsExamples::ConstTrackContainer> m_acts_tracks_input {this};
  Input<ActsExamples::Trajectories> m_acts_trajectories_input {this};
  PodioInput<edm4eic::Measurement2D> m_measurements_input {this};
  PodioOutput<edm4eic::Trajectory> m_trajectories_filtered_output {this};
  PodioOutput<edm4eic::TrackParameters> m_parameters_filtered_output {this};
  PodioOutput<edm4eic::Track> m_tracks_filtered_output {this};
  Output<ActsExamples::ConstTrackContainer> m_acts_tracks_filtered_output {this};
  Output<ActsExamples::Trajectories> m_acts_trajectories_filtered_output {this};

  ParameterRef<std::uint32_t> m_maximumSharedHits{this, "m_maximumSharedHits", config().m_maximumSharedHits,
                                                "Maximum number of shared hits allowed"};
  ParameterRef<std::uint32_t> m_maximumIterations{this, "m_maximumIterations", config().m_maximumIterations,
                                                "Maximum number of iterations"};
  ParameterRef<std::size_t> m_nMeasurementsMin{
      this, "m_nMeasurementsMin", config().m_nMeasurementsMin,
      "Number of measurements required for further reconstruction"};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>();
    m_algo->applyConfig(config());
    m_algo->init(logger());
  }

  void ChangeRun(int64_t run_number) {}

  void Process(int64_t run_number, uint64_t event_number) {
   std::tie(m_trajectories_filtered_output(), m_parameters_filtered_output(), m_tracks_filtered_output(),m_acts_tracks_filtered_output(),m_acts_trajectories_filtered_output()) = m_algo->process(m_acts_tracks_input(),m_acts_trajectories_input(),*m_measurements_input());
  }
} ;

} // namespace eicrecon
