// Created by Joe Osborn
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <ActsExamples/EventData/Track.hpp>
#include <JANA/JEvent.h>
#include <cassert>
#include <edm4eic/VertexCollection.h>
#include <edm4eic/ReconstructedParticle.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "algorithms/tracking/IterativeVertexFinderConfig.h"
#include "algorithms/tracking/IterativeVertexFinder.h"
#include "extensions/jana/JOmniFactory.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"

namespace eicrecon {

class IterativeVertexFinder_factory
    : public JOmniFactory<IterativeVertexFinder_factory, IterativeVertexFinderConfig> {

private:
  using AlgoT = eicrecon::IterativeVertexFinder;
  std::unique_ptr<AlgoT> m_algo;

  Input<Acts::ConstVectorMultiTrajectory> m_acts_track_states_input{this};
  Input<Acts::ConstVectorTrackContainer> m_acts_tracks_input{this};
  PodioInput<edm4eic::ReconstructedParticle> m_edm4eic_reconParticles_input{this};
  PodioOutput<edm4eic::Vertex> m_vertices_output{this};

  ParameterRef<int> m_maxVertices{this, "maxVertices", config().maxVertices,
                                  "Maximum num vertices that can be found"};
  ParameterRef<bool> m_reassignTracksAfterFirstFit{
      this, "reassignTracksAfterFirstFit", config().reassignTracksAfterFirstFit,
      "Whether or not to reassign tracks after first fit"};

  Service<AlgorithmsInit_service> m_algorithmsInit{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(this->GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    auto track_states_vec = m_acts_track_states_input();
    auto tracks_vec       = m_acts_tracks_input();
    assert(!track_states_vec.empty() && "ConstVectorMultiTrajectory vector should not be empty");
    assert(track_states_vec.front() != nullptr &&
           "ConstVectorMultiTrajectory pointer should not be null");
    assert(!tracks_vec.empty() && "ConstVectorTrackContainer vector should not be empty");
    assert(tracks_vec.front() != nullptr && "ConstVectorTrackContainer pointer should not be null");

    m_algo->process(AlgoT::Input{track_states_vec.front(), tracks_vec.front(),
                                 m_edm4eic_reconParticles_input()},
                    AlgoT::Output{m_vertices_output().get()});
  }
};

} // namespace eicrecon
