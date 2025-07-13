// Created by Joe Osborn
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <ActsExamples/EventData/Trajectories.hpp>
#include <JANA/JEvent.h>
#include <edm4eic/VertexCollection.h>
#include <edm4eic/ReconstructedParticle.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "algorithms/tracking/ActsExamplesEdm.h"
#include "algorithms/tracking/ActsPodioEdm.h"
#include "algorithms/tracking/IterativeVertexFinderConfig.h"
#include "algorithms/tracking/IterativeVertexFinder.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

template <typename edm_t = eicrecon::ActsExamplesEdm>
class IterativeVertexFinder_factory
    : public JOmniFactory<IterativeVertexFinder_factory<edm_t>, IterativeVertexFinderConfig> {

private:
  using AlgoT    = eicrecon::IterativeVertexFinder<edm_t>;
  using FactoryT = JOmniFactory<IterativeVertexFinder_factory<edm_t>, IterativeVertexFinderConfig>;

  std::unique_ptr<AlgoT> m_algo;

  template <typename T> using PodioInput   = typename FactoryT::template PodioInput<T>;
  template <typename T> using PodioOutput  = typename FactoryT::template PodioOutput<T>;
  template <typename T> using Input        = typename FactoryT::template Input<T>;
  template <typename T> using Output       = typename FactoryT::template Output<T>;
  template <typename T> using ParameterRef = typename FactoryT::template ParameterRef<T>;
  template <typename T> using Service      = typename FactoryT::template Service<T>;

  Input<typename edm_t::Trajectories> m_acts_trajectories_input{this};
  PodioInput<edm4eic::ReconstructedParticle> m_edm4eic_reconParticles_input{this};
  PodioOutput<edm4eic::Vertex> m_vertices_output{this};

  ParameterRef<int> m_maxVertices{this, "maxVertices", this->config().maxVertices,
                                  "Maximum num vertices that can be found"};
  ParameterRef<bool> m_reassignTracksAfterFirstFit{
      this, "reassignTracksAfterFirstFit", this->config().reassignTracksAfterFirstFit,
      "Whether or not to reassign tracks after first fit"};

  Service<ACTSGeo_service> m_ACTSGeoSvc{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>();
    m_algo->applyConfig(this->config());
    m_algo->init(m_ACTSGeoSvc().actsGeoProvider(), this->logger());
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_vertices_output() =
        m_algo->produce(m_acts_trajectories_input(), m_edm4eic_reconParticles_input());
  }
};

} // namespace eicrecon
