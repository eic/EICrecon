// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright 2022 - 2025 Dmitry Romanov, Dmitry Kalinkin

#pragma once

#include <ActsExamples/EventData/Track.hpp>
#include <JANA/JEvent.h>
#include <cassert>
#include <edm4eic/TrackSegmentCollection.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "algorithms/tracking/TrackProjector.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class TrackProjector_factory : public JOmniFactory<TrackProjector_factory, NoConfig> {

private:
  using AlgoT = eicrecon::TrackProjector;

  std::unique_ptr<AlgoT> m_algo;

  Input<ActsExamples::ConstTrackContainer> m_acts_tracks_input{this};
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
    auto tracks_vec = m_acts_tracks_input();
    assert(!tracks_vec.empty() && "ConstTrackContainer vector should not be empty");
    assert(tracks_vec.front() != nullptr && "ConstTrackContainer pointer should not be null");
    m_algo->process(
        {
            tracks_vec.front(),
            m_tracks_input(),
        },
        {
            m_segments_output().get(),
        });
  }
};

} // namespace eicrecon
