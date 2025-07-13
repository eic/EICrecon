// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright 2022 - 2025 Dmitry Romanov, Dmitry Kalinkin

#pragma once

#include <ActsExamples/EventData/Trajectories.hpp>
#include <JANA/JEvent.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "algorithms/tracking/ActsExamplesEdm.h"
#include "algorithms/tracking/ActsPodioEdm.h"
#include "algorithms/tracking/TrackProjector.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

template <typename edm_t = eicrecon::ActsExamplesEdm>
class TrackProjector_factory : public JOmniFactory<TrackProjector_factory<edm_t>> {

private:
  using AlgoT    = eicrecon::TrackProjector<edm_t>;
  using FactoryT = JOmniFactory<TrackProjector_factory<edm_t>>;

  std::unique_ptr<AlgoT> m_algo;

  template <typename T> using PodioInput   = typename FactoryT::template PodioInput<T>;
  template <typename T> using PodioOutput  = typename FactoryT::template PodioOutput<T>;
  template <typename T> using Input        = typename FactoryT::template Input<T>;
  template <typename T> using Output       = typename FactoryT::template Output<T>;
  template <typename T> using ParameterRef = typename FactoryT::template ParameterRef<T>;
  template <typename T> using Service      = typename FactoryT::template Service<T>;

  Input<typename edm_t::Trajectories> m_acts_trajectories_input{this};
  PodioInput<edm4eic::Track> m_tracks_input{this};
  PodioOutput<edm4eic::TrackSegment> m_segments_output{this};

  Service<AlgorithmsInit_service> m_algorithmsInit{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(this->GetPrefix());
    m_algo->level((algorithms::LogLevel)this->logger()->level());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    std::vector<gsl::not_null<const typename edm_t::Trajectories*>> acts_trajectories_input;
    for (auto acts_traj : m_acts_trajectories_input()) {
      acts_trajectories_input.push_back(acts_traj);
    }
    m_algo->process(
        {
            acts_trajectories_input,
            m_tracks_input(),
        },
        {
            m_segments_output().get(),
        });
  }
};

} // namespace eicrecon
