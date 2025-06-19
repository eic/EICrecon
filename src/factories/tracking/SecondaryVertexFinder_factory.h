// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Dongwi H. Dongwi (Bishoy)

#pragma once

#include <ActsExamples/EventData/Trajectories.hpp>
#include <JANA/JEvent.h>
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
  Input<ActsExamples::Trajectories> m_acts_trajectories_input{this};
  PodioOutput<edm4eic::Vertex> prm_vertices_output{this};
  PodioOutput<edm4eic::Vertex> sec_vertices_output{this};

  ParameterRef<int> m_maxVertices{this, "maxVertices", config().maxVertices,
                                  "Maximum num vertices that can be found"};
  ParameterRef<bool> m_reassignTracksAfterFirstFit{
      this, "reassignTracksAfterFirstFit", config().reassignTracksAfterFirstFit,
      "Whether or not to reassign tracks after first fit"};

  Service<ACTSGeo_service> m_ACTSGeoSvc{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>();
    m_algo->level((algorithms::LogLevel)logger()->level());
    m_algo->init(m_ACTSGeoSvc().actsGeoProvider(), logger());
  }

  void ChangeRun(int32_t /*run_number*/) {}

  void Process(int32_t /*run_number*/, uint64_t /*event_number*/) {
    std::tie(prm_vertices_output(), sec_vertices_output()) =
        m_algo->produce(m_reco_input(), m_acts_trajectories_input());
  }
};

} // namespace eicrecon
