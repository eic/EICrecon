// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Wouter Deconinck

#pragma once

#include <ActsExamples/EventData/Track.hpp>
#include <JANA/JEvent.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class ActsTrackMerger_factory : public JOmniFactory<ActsTrackMerger_factory, NoConfig> {
private:
  Input<ActsExamples::ConstTrackContainer> m_acts_tracks1_input{this};
  Input<ActsExamples::ConstTrackContainer> m_acts_tracks2_input{this};
  Output<ActsExamples::ConstTrackContainer> m_acts_tracks_output{this};

public:
  void Configure() {}

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    // Simply concatenate the two track container vectors
    for (auto tracks : m_acts_tracks1_input()) {
      m_acts_tracks_output().push_back(const_cast<ActsExamples::ConstTrackContainer*>(tracks));
    }
    for (auto tracks : m_acts_tracks2_input()) {
      m_acts_tracks_output().push_back(const_cast<ActsExamples::ConstTrackContainer*>(tracks));
    }
  }
};

} // namespace eicrecon
