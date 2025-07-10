// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Minjung Kim, Barak Schmookler
#pragma once

#include "algorithms/tracking/ActsExamplesEdm.h"
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

template <typename edm_t = eicrecon::ActsExamplesEdm>
class AmbiguitySolver_factory
    : public JOmniFactory<AmbiguitySolver_factory<edm_t>, AmbiguitySolverConfig> {

private:
  using AlgoT    = eicrecon::AmbiguitySolver<edm_t>;
  using FactoryT = JOmniFactory<AmbiguitySolver_factory<edm_t>, AmbiguitySolverConfig>;

  std::unique_ptr<AlgoT> m_algo;

  template <typename T> using PodioInput   = typename FactoryT::template PodioInput<T>;
  template <typename T> using PodioOutput  = typename FactoryT::template PodioOutput<T>;
  template <typename T> using Input        = typename FactoryT::template Input<T>;
  template <typename T> using Output       = typename FactoryT::template Output<T>;
  template <typename T> using ParameterRef = typename FactoryT::template ParameterRef<T>;
  template <typename T> using Service      = typename FactoryT::template Service<T>;

  Input<typename edm_t::ConstTrackContainer> m_acts_tracks_input{this};
  PodioInput<edm4eic::Measurement2D> m_measurements_input{this};
  Output<typename edm_t::ConstTrackContainer> m_acts_tracks_output{this};
  Output<typename edm_t::Trajectories> m_acts_trajectories_output{this};

  ParameterRef<std::uint32_t> m_maximumSharedHits{this, "maximumSharedHits",
                                                  this->config().maximum_shared_hits,
                                                  "Maximum number of shared hits allowed"};
  ParameterRef<std::uint32_t> m_maximumIterations{
      this, "maximumIterations", this->config().maximum_iterations, "Maximum number of iterations"};
  ParameterRef<std::size_t> m_nMeasurementsMin{
      this, "nMeasurementsMin", this->config().n_measurements_min,
      "Number of measurements required for further reconstruction"};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>();
    m_algo->applyConfig(this->config());
    m_algo->init(this->logger());
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
