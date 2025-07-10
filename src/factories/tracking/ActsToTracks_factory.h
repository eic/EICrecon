// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright 2024, Dmitry Kalinkin

#pragma once

#include <memory>

#include "algorithms/tracking/ActsExamplesEdm.h"
#include "algorithms/tracking/ActsToTracks.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

template <typename edm_t = eicrecon::ActsExamplesEdm>
class ActsToTracks_factory : public JOmniFactory<ActsToTracks_factory<edm_t>> {
public:
  using AlgoT    = eicrecon::ActsToTracks<edm_t>;
  using FactoryT = JOmniFactory<ActsToTracks_factory<edm_t>>;

private:
  std::unique_ptr<AlgoT> m_algo;

  template <typename T> using PodioInput   = typename FactoryT::template PodioInput<T>;
  template <typename T> using PodioOutput  = typename FactoryT::template PodioOutput<T>;
  template <typename T> using Input        = typename FactoryT::template Input<T>;
  template <typename T> using Output       = typename FactoryT::template Output<T>;
  template <typename T> using ParameterRef = typename FactoryT::template ParameterRef<T>;
  template <typename T> using Service      = typename FactoryT::template Service<T>;

  PodioInput<edm4eic::Measurement2D> m_measurements_input{this};
  Input<typename edm_t::Trajectories> m_acts_trajectories_input{this};
  PodioInput<edm4eic::MCRecoTrackerHitAssociation> m_raw_hit_assocs_input{this};
  PodioOutput<edm4eic::Trajectory> m_trajectories_output{this};
  PodioOutput<edm4eic::TrackParameters> m_parameters_output{this};
  PodioOutput<edm4eic::Track> m_tracks_output{this};
  PodioOutput<edm4eic::MCRecoTrackParticleAssociation> m_track_assocs_output{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(this->GetPrefix());
    m_algo->level((algorithms::LogLevel)this->logger()->level());
    m_algo->init();
  };

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    std::vector<gsl::not_null<const typename edm_t::Trajectories*>> acts_trajectories_input;
    for (auto acts_traj : m_acts_trajectories_input()) {
      acts_trajectories_input.push_back(acts_traj);
    }
    m_algo->process(
        {
            m_measurements_input(),
            acts_trajectories_input,
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
