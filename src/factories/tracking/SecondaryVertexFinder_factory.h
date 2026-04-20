// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Dongwi H. Dongwi (Bishoy)

#pragma once

#include <ActsExamples/EventData/Track.hpp>
#include <JANA/JEvent.h>
#include <cassert>
#include <edm4eic/Vertex.h>
#include <edm4eic/TrackParameters.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "algorithms/tracking/SecondaryVertexFinderConfig.h"
#include "algorithms/tracking/SecondaryVertexFinder.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class SecondaryVertexFinder_factory
    : public JOmniFactory<SecondaryVertexFinder_factory, SecondaryVertexFinderConfig> {

private:
  using AlgoT = eicrecon::SecondaryVertexFinder;
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4eic::ReconstructedParticle> m_reco_input{this};
  Input<ActsExamples::ConstTrackContainer> m_acts_tracks_input{this};
  PodioOutput<edm4eic::Vertex> m_prm_vertices_output{this};
  PodioOutput<edm4eic::Vertex> m_sec_vertices_output{this};

  ParameterRef<unsigned int> m_maxVertices{this, "maxVertices", config().maxVertices,
                                           "Maximum num vertices that can be found"};
  ParameterRef<bool> m_reassignTracksAfterFirstFit{
      this, "reassignTracksAfterFirstFit", config().reassignTracksAfterFirstFit,
      "Whether or not to reassign tracks after first fit"};
  ParameterRef<float> m_tracksMaxZinterval{this, "tracksMaxZinterval", config().tracksMaxZinterval,
                                           "Max z interval for Acts::AdaptiveMultiVertexFinder."};
  ParameterRef<unsigned int> m_maxIterations{this, "maxIterations", config().maxIterations,
                                             "Max iterations for Acts::AdaptiveMultivertexFinder"};
  ParameterRef<float> m_maxDistToLinPoint{
      this, "maxDistToLinPoint", config().maxDistToLinPoint,
      "Max disttance to line point (pca) for Acts::AdaptiveMultivertexFinder"};

  Service<ACTSGeo_service> m_ACTSGeoSvc{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(this->GetPrefix());
    m_algo->applyConfig(config());
    m_algo->applyLogger(logger());
    m_algo->init();
  }

  void Process(int32_t, uint64_t) {
    auto tracks_vec = m_acts_tracks_input();
    assert(!tracks_vec.empty() && "ConstTrackContainer vector should not be empty");
    assert(tracks_vec.front() != nullptr && "ConstTrackContainer pointer should not be null");
    m_algo->process(
        {
            m_reco_input(),
            tracks_vec.front(),
        },
        {
            m_prm_vertices_output().get(),
            m_sec_vertices_output().get(),
        });
  }
};

} // namespace eicrecon
