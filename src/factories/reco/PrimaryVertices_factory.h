// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Xin Dong

#pragma once

#include <JANA/JEvent.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "algorithms/reco/PrimaryVertices.h"
#include "algorithms/reco/PrimaryVerticesConfig.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class PrimaryVertices_factory
    : public JOmniFactory<PrimaryVertices_factory, PrimaryVerticesConfig> {

public:
  using AlgoT = eicrecon::PrimaryVertices;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4eic::Vertex> m_rc_vertices_input{this};

  // Declare outputs
  PodioOutput<edm4eic::Vertex> m_primary_vertices_output{this};

  Service<AlgorithmsInit_service> m_algorithmsInit{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level((algorithms::LogLevel)logger()->level());

    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_rc_vertices_input()}, {m_primary_vertices_output().get()});
  }
};

} // namespace eicrecon
