// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Wouter Deconinck

#pragma once

#include <ActsExamples/EventData/Track.hpp>
#include <JANA/JEvent.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "algorithms/tracking/ActsTracksToTrajectoriesHelper.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class ActsTrackContainerMerger_factory : public JOmniFactory<ActsTrackContainerMerger_factory> {
private:
  Input<ActsExamples::ConstTrackContainer> m_acts_tracks1_input{this};
  Input<ActsExamples::ConstTrackContainer> m_acts_tracks2_input{this};
  Output<ActsExamples::ConstTrackContainer> m_acts_tracks_output{this};

public:
  void Configure() {}

  void ChangeRun(int32_t /* run_number */) {}

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {

    auto& acts_tracks1_input   = *(m_acts_tracks1_input().front());
    auto& trackContainer1      = acts_tracks1_input.container();
    auto& trackStateContainer1 = acts_tracks1_input.trackStateContainer();

    auto& acts_tracks2_input   = *(m_acts_tracks2_input().front());
    auto& trackContainer2      = acts_tracks2_input.container();
    auto& trackStateContainer2 = acts_tracks2_input.trackStateContainer();

    auto trackContainer = std::make_shared<Acts::VectorTrackContainer>();
    trackContainer->reserve(trackContainer1.size_impl() + trackContainer2.size_impl());

    auto trackStateContainer = std::make_shared<Acts::VectorMultiTrajectory>();
    trackStateContainer->reserve(trackStateContainer1.size_impl() +
                                 trackStateContainer2.size_impl());

    ActsExamples::TrackContainer track_container_output(trackContainer, trackStateContainer);
    track_container_output.ensureDynamicColumns(acts_tracks1_input);
    track_container_output.ensureDynamicColumns(acts_tracks2_input);

    for (const auto& track1 : acts_tracks1_input) {
      track_container_output.makeTrack().copyFrom(track1, true);
    }
    for (const auto& track2 : acts_tracks2_input) {
      track_container_output.makeTrack().copyFrom(track2, true);
    }

    m_acts_tracks_output().push_back(new ActsExamples::ConstTrackContainer(
        std::make_shared<Acts::ConstVectorTrackContainer>(std::move(*trackContainer)),
        std::make_shared<Acts::ConstVectorMultiTrajectory>(std::move(*trackStateContainer))));
  }
};

} // namespace eicrecon
