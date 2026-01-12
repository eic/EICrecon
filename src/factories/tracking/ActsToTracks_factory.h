// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright 2024, Dmitry Kalinkin

#pragma once

#include <cassert>
#include <memory>

#include "algorithms/tracking/ActsToTracks.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class ActsToTracks_factory : public JOmniFactory<ActsToTracks_factory, NoConfig> {
public:
  using AlgoT = eicrecon::ActsToTracks;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4eic::Measurement2D> m_measurements_input{this};
  Input<Acts::ConstVectorMultiTrajectory> m_acts_track_states_input{this};
  Input<Acts::ConstVectorTrackContainer> m_acts_tracks_input{this};
  PodioInput<edm4eic::MCRecoTrackerHitAssociation> m_raw_hit_assocs_input{this};
  PodioOutput<edm4eic::Trajectory> m_trajectories_output{this};
  PodioOutput<edm4eic::TrackParameters> m_parameters_output{this};
  PodioOutput<edm4eic::Track> m_tracks_output{this};
  PodioOutput<edm4eic::MCRecoTrackParticleAssociation> m_track_assocs_output{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(this->GetPrefix());
    m_algo->level((algorithms::LogLevel)logger()->level());
    m_algo->applyConfig(config());
    m_algo->init();
  };

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    auto track_states_vec = m_acts_track_states_input();
    auto tracks_vec       = m_acts_tracks_input();
    assert(!track_states_vec.empty() && "ConstVectorMultiTrajectory vector should not be empty");
    assert(track_states_vec.front() != nullptr &&
           "ConstVectorMultiTrajectory pointer should not be null");
    assert(!tracks_vec.empty() && "ConstVectorTrackContainer vector should not be empty");
    assert(tracks_vec.front() != nullptr && "ConstVectorTrackContainer pointer should not be null");
    m_algo->process(
        {
            m_measurements_input(),
            track_states_vec.front(),
            tracks_vec.front(),
            m_raw_hit_assocs_input(),
        },
        {
            m_trajectories_output().get(),
            m_parameters_output().get(),
            m_tracks_output().get(),
            m_track_assocs_output().get(),
        });
  }
};

} // namespace eicrecon
