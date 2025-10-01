// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Xin Dong

#pragma once

#include <JANA/JEvent.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "algorithms/reco/SecondaryVertices.h"
#include "algorithms/reco/SecondaryVerticesConfig.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class SecondaryVertices_factory
    : public JOmniFactory<SecondaryVertices_factory, SecondaryVerticesConfig> {

public:
  using AlgoT = eicrecon::SecondaryVertices;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4eic::Vertex> m_rc_vertices_input{this};
  PodioInput<edm4eic::ReconstructedParticle> m_rc_parts_input{this};

  // Declare outputs
  PodioOutput<edm4eic::Vertex> m_secondary_vertices_output{this};

  Service<AlgorithmsInit_service> m_algorithmsInit{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level((algorithms::LogLevel)logger()->level());

    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_rc_vertices_input(), m_rc_parts_input()}, {m_secondary_vertices_output().get()});
  }
};

} // namespace eicrecon
