// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Wouter Deconinck

#pragma once

#ifndef EICRECON_FACTORY_PRECOMPILE
// Preprocessor-based precompilation pattern:
// When EICRECON_FACTORY_PRECOMPILE is not defined, plugin code sees only
// forward declarations and extern templates for fast compilation.
// The full definition is compiled once into a precompile library.

namespace eicrecon {
class ActsTrackMerger_factory;
}

extern template class JOmniFactory<eicrecon::ActsTrackMerger_factory, NoConfig>;

#else
// Full factory definition: compiled into precompile library

#include <ActsExamples/EventData/Track.hpp>
#include <JANA/JEvent.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "algorithms/tracking/ActsTrackMerger.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

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
    auto track_states1_vec = m_acts_track_states1_input();
    auto tracks1_vec       = m_acts_tracks1_input();
    auto track_states2_vec = m_acts_track_states2_input();
    auto tracks2_vec       = m_acts_tracks2_input();

    assert(!track_states1_vec.empty() && "ConstVectorMultiTrajectory vector 1 should not be empty");
    assert(track_states1_vec.front() != nullptr &&
           "ConstVectorMultiTrajectory pointer 1 should not be null");
    assert(!tracks1_vec.empty() && "ConstVectorTrackContainer vector 1 should not be empty");
    assert(tracks1_vec.front() != nullptr &&
           "ConstVectorTrackContainer pointer 1 should not be null");
    assert(!track_states2_vec.empty() && "ConstVectorMultiTrajectory vector 2 should not be empty");
    assert(track_states2_vec.front() != nullptr &&
           "ConstVectorMultiTrajectory pointer 2 should not be null");
    assert(!tracks2_vec.empty() && "ConstVectorTrackContainer vector 2 should not be empty");
    assert(tracks2_vec.front() != nullptr &&
           "ConstVectorTrackContainer pointer 2 should not be null");

    m_algo->process(AlgoT::Input{track_states1_vec.front(), tracks1_vec.front(),
                                 track_states2_vec.front(), tracks2_vec.front()},
                    AlgoT::Output{&m_acts_track_states_output().emplace_back(),
                                  &m_acts_tracks_output().emplace_back()});
  }
};

} // namespace eicrecon

#endif // EICRECON_FACTORY_PRECOMPILE
