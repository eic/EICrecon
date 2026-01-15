// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Wouter Deconinck

#pragma once

#include <ActsPodioEdm/BoundParametersCollection.h>
#include <ActsPodioEdm/JacobianCollection.h>
#include <ActsPodioEdm/TrackCollection.h>
#include <ActsPodioEdm/TrackStateCollection.h>
#include <JANA/JEvent.h>
#include <memory>
#include <string>
#include <utility>

#include "algorithms/tracking/ActsTrackMerger.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

/// Factory that merges Acts track containers from multiple sources into a single output.
///
/// Typical use is to combine tracks reconstructed in different subsystems
/// (e.g. central tracker and B0 tracker) into one unified collection for
/// downstream reconstruction or analysis. The current implementation simply
/// concatenates the input Podio collections in the order they are provided.
class ActsTrackMerger_factory : public JOmniFactory<ActsTrackMerger_factory, NoConfig> {
public:
  using AlgoT = eicrecon::ActsTrackMerger;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<ActsPodioEdm::TrackState> m_acts_track_states1_input{this};
  PodioInput<ActsPodioEdm::BoundParameters> m_acts_track_parameters1_input{this};
  PodioInput<ActsPodioEdm::Jacobian> m_acts_track_jacobians1_input{this};
  PodioInput<ActsPodioEdm::Track> m_acts_tracks1_input{this};
  PodioInput<ActsPodioEdm::TrackState> m_acts_track_states2_input{this};
  PodioInput<ActsPodioEdm::BoundParameters> m_acts_track_parameters2_input{this};
  PodioInput<ActsPodioEdm::Jacobian> m_acts_track_jacobians2_input{this};
  PodioInput<ActsPodioEdm::Track> m_acts_tracks2_input{this};
  PodioOutput<ActsPodioEdm::TrackState> m_acts_track_states_output{this};
  PodioOutput<ActsPodioEdm::BoundParameters> m_acts_track_parameters_output{this};
  PodioOutput<ActsPodioEdm::Jacobian> m_acts_track_jacobians_output{this};
  PodioOutput<ActsPodioEdm::Track> m_acts_tracks_output{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(this->GetPrefix());
    m_algo->level((algorithms::LogLevel)logger()->level());
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process(
        AlgoT::Input{m_acts_track_states1_input(), m_acts_track_parameters1_input(),
                     m_acts_track_jacobians1_input(), m_acts_tracks1_input(),
                     m_acts_track_states2_input(), m_acts_track_parameters2_input(),
                     m_acts_track_jacobians2_input(), m_acts_tracks2_input()},
        AlgoT::Output{m_acts_track_states_output().get(), m_acts_track_parameters_output().get(),
                      m_acts_track_jacobians_output().get(), m_acts_tracks_output().get()});
  }
};

} // namespace eicrecon
