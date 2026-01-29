// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright 2024, Dmitry Kalinkin

#pragma once

#include <edm4eic/EDM4eicVersion.h>
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
  PodioInput<edm4eic::TrackSeed> m_seeds_input{this};
  Input<ActsExamples::ConstTrackContainer> m_acts_tracks_input{this};
  PodioInput<edm4eic::MCRecoTrackerHitAssociation> m_raw_hit_assocs_input{this};
  PodioOutput<edm4eic::Trajectory> m_trajectories_output{this};
  PodioOutput<edm4eic::TrackParameters> m_parameters_output{this};
  PodioOutput<edm4eic::Track> m_tracks_output{this};
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
  PodioOutput<edm4eic::MCRecoTrackParticleLink> m_track_links_output{this};
#endif
  PodioOutput<edm4eic::MCRecoTrackParticleAssociation> m_track_assocs_output{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(this->GetPrefix());
    m_algo->level((algorithms::LogLevel)logger()->level());
    m_algo->applyConfig(config());
    m_algo->init();
  };

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    auto tracks_vec = m_acts_tracks_input();
    assert(!tracks_vec.empty() && "ConstTrackContainer vector should not be empty");
    assert(tracks_vec.front() != nullptr && "ConstTrackContainer pointer should not be null");
    m_algo->process(
        {
            m_measurements_input(),
            m_seeds_input(),
            tracks_vec.front(),
            m_raw_hit_assocs_input(),
        },
        {
            m_trajectories_output().get(),
            m_parameters_output().get(),
            m_tracks_output().get(),
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
            m_track_links_output().get(),
#endif
            m_track_assocs_output().get(),
        });
  }
};

} // namespace eicrecon
