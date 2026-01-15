// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Tyler Kutz, Wouter Deconinck

#pragma once

#include <Acts/Surfaces/Surface.hpp>
#include <JANA/JEvent.h>
#include <cassert>
#include <edm4eic/TrackSegmentCollection.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/tracking/TrackPropagation.h"
#include "algorithms/tracking/TrackPropagationConfig.h"
#include "extensions/jana/JOmniFactory.h"
#include "services/geometry/dd4hep/DD4hep_service.h"

namespace eicrecon {

class TrackPropagation_factory
    : public JOmniFactory<TrackPropagation_factory, TrackPropagationConfig> {

private:
  using AlgoT = eicrecon::TrackPropagation;
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4eic::Track> m_tracks_input{this};
  PodioInput<ActsPodioEdm::TrackState> m_acts_track_states_input{this};
  PodioInput<ActsPodioEdm::BoundParameters> m_acts_track_parameters_input{this};
  PodioInput<ActsPodioEdm::Jacobian> m_acts_track_jacobians_input{this};
  PodioInput<ActsPodioEdm::Track> m_acts_tracks_input{this};
  PodioOutput<edm4eic::TrackSegment> m_track_segments_output{this};

  Service<DD4hep_service> m_GeoSvc{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(this->GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process(AlgoT::Input{m_tracks_input(), m_acts_track_states_input(),
                                 m_acts_track_parameters_input(), m_acts_track_jacobians_input(),
                                 m_acts_tracks_input()},
                    AlgoT::Output{m_track_segments_output().get()});
  }
};

} // namespace eicrecon
