// Created by Joe Osborn
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <ActsExamples/EventData/Track.hpp>
#include <JANA/JEvent.h>
#include <edm4eic/VertexCollection.h>
#include <edm4eic/ReconstructedParticle.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "algorithms/tracking/IterativeVertexFinderConfig.h"
#include "algorithms/tracking/IterativeVertexFinder.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class IterativeVertexFinder_factory
    : public JOmniFactory<IterativeVertexFinder_factory, IterativeVertexFinderConfig> {

private:
  using AlgoT = eicrecon::IterativeVertexFinder;
  std::unique_ptr<AlgoT> m_algo;

  Input<ActsExamples::ConstTrackContainer> m_acts_tracks_input{this};
  PodioInput<edm4eic::ReconstructedParticle> m_edm4eic_reconParticles_input{this};
  PodioOutput<edm4eic::Vertex> m_vertices_output{this};

  ParameterRef<int> m_maxVertices{this, "maxVertices", config().maxVertices,
                                  "Maximum num vertices that can be found"};
  ParameterRef<bool> m_reassignTracksAfterFirstFit{
      this, "reassignTracksAfterFirstFit", config().reassignTracksAfterFirstFit,
      "Whether or not to reassign tracks after first fit"};

  Service<ACTSGeo_service> m_ACTSGeoSvc{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>();
    // TODO: convert IterativeVertexFinder to inherit from algorithm::Algorithm
    // m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init(m_ACTSGeoSvc().actsGeoProvider(), logger());
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    auto tracks_vec = m_acts_tracks_input();
    if (!tracks_vec.empty()) {
      m_vertices_output() = m_algo->produce(tracks_vec.front(), m_edm4eic_reconParticles_input());
    }
  }
};

} // namespace eicrecon
