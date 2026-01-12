// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Wouter Deconinck

#pragma once

#include <ActsExamples/EventData/Track.hpp>
#include <JANA/JEvent.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "algorithms/tracking/ActsTrackMerger.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

/// Factory that merges Acts track containers from multiple sources into a single output.
///
/// Typical use is to combine tracks reconstructed in different subsystems
/// (e.g. central tracker and B0 tracker) into one unified collection for
/// downstream reconstruction or analysis. The current implementation simply
/// concatenates the input ConstTrackContainer collections in the order they
/// are provided.
class ActsTrackMerger_factory : public JOmniFactory<ActsTrackMerger_factory, NoConfig> {
public:
  using AlgoT = eicrecon::ActsTrackMerger;

private:
  std::unique_ptr<AlgoT> m_algo;

  Input<Acts::ConstVectorMultiTrajectory> m_acts_track_states1_input{this};
  Input<Acts::ConstVectorTrackContainer> m_acts_tracks1_input{this};
  Input<Acts::ConstVectorMultiTrajectory> m_acts_track_states2_input{this};
  Input<Acts::ConstVectorTrackContainer> m_acts_tracks2_input{this};
  Output<Acts::ConstVectorMultiTrajectory> m_acts_track_states_output{this};
  Output<Acts::ConstVectorTrackContainer> m_acts_tracks_output{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(this->GetPrefix());
    m_algo->level((algorithms::LogLevel)logger()->level());
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    assert(m_acts_track_states_output().size() == 0 &&
           "ActsTrackMerger_factory: m_acts_track_states_output not cleared from previous event");
    assert(m_acts_tracks_output().size() == 0 &&
           "ActsTrackMerger_factory: m_acts_tracks_output not cleared from previous event");

    // Use helper merge method
    auto [track_states, tracks] =
        m_algo->merge(m_acts_track_states1_input(), m_acts_tracks1_input(),
                      m_acts_track_states2_input(), m_acts_tracks2_input());
    m_acts_track_states_output() = track_states;
    m_acts_tracks_output()       = tracks;
  }
};

} // namespace eicrecon
