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

#include "algorithms/tracking/TrackProjector.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class TrackProjector_factory : public JOmniFactory<TrackProjector_factory> {

private:
  using AlgoT = eicrecon::TrackProjector;

  std::unique_ptr<AlgoT> m_algo;

  Input<ActsExamples::Trajectories> m_acts_trajectories_input{this};
  PodioInput<edm4eic::Track> m_tracks_input{this};
  PodioOutput<edm4eic::TrackSegment> m_segments_output{this};

  Service<AlgorithmsInit_service> m_algorithmsInit{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(this->GetPrefix());
    m_algo->level((algorithms::LogLevel)logger()->level());
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    std::vector<gsl::not_null<const ActsExamples::Trajectories*>> acts_trajectories_input;
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
