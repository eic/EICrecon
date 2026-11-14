// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Tyler Kutz, Wouter Deconinck

#pragma once

#include <Acts/Surfaces/Surface.hpp>
#include <JANA/JEvent.h>
#include <cassert>
#include <edm4eic/TrackSegmentCollection.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/tracking/TrackPropagation.h"
#include "algorithms/tracking/TrackPropagationConfig.h"
#include "extensions/jana/JOmniFactory.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"

namespace eicrecon {

class TrackPropagation_factory
    : public JOmniFactory<TrackPropagation_factory, TrackPropagationConfig> {

private:
  using AlgoT = eicrecon::TrackPropagation;
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4eic::Track> m_tracks_input{this};
  Input<Acts::ConstVectorMultiTrajectory> m_acts_track_states_input{this};
  Input<Acts::ConstVectorTrackContainer> m_acts_tracks_input{this};
  PodioOutput<edm4eic::TrackSegment> m_track_segments_output{this};

  Service<AlgorithmsInit_service> m_algorithmsInit{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(this->GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    auto track_states_vec = m_acts_track_states_input();
    auto tracks_vec       = m_acts_tracks_input();
    assert(!track_states_vec.empty() && "ConstVectorMultiTrajectory vector should not be empty");
    assert(track_states_vec.front() != nullptr &&
           "ConstVectorMultiTrajectory pointer should not be null");
    assert(!tracks_vec.empty() && "ConstVectorTrackContainer vector should not be empty");
    assert(tracks_vec.front() != nullptr && "ConstVectorTrackContainer pointer should not be null");

    m_algo->process(AlgoT::Input{m_tracks_input(), track_states_vec.front(), tracks_vec.front()},
                    AlgoT::Output{m_track_segments_output().get()});
  }
};

} // namespace eicrecon
