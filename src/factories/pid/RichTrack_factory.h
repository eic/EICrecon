// Copyright (C) 2022, 2023 Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

#include <Acts/Surfaces/Surface.hpp>
#include <JANA/JEvent.h>
#include <edm4eic/TrackPoint.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

// algorithms
#include "algorithms/tracking/TrackPropagation.h"
#include "algorithms/tracking/TrackPropagationConfig.h"
#include "extensions/jana/JOmniFactory.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"

namespace eicrecon {

class RichTrack_factory : public JOmniFactory<RichTrack_factory, TrackPropagationConfig> {

private:
  using AlgoT = eicrecon::TrackPropagation;
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4eic::Track> m_tracks_input{this};
  Input<ActsExamples::Trajectories> m_acts_trajectories_input{this};
  Input<ActsExamples::ConstTrackContainer> m_acts_tracks_input{this};
  PodioOutput<edm4eic::TrackSegment> m_track_segments_output{this};

  Service<AlgorithmsInit_service> m_algorithmsInit{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>();
    // TODO: convert RichTrack to inherit from algorithm::Algorithm
    // m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());

    if (config().filter_surfaces.empty())
      throw JException("cannot find filter surface for RICH track propagation");

    m_algo->init(logger());
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->propagateToSurfaceList(
        {*m_tracks_input(), m_acts_trajectories_input(), m_acts_tracks_input()},
        {m_track_segments_output().get()});
  }
};

} // namespace eicrecon
