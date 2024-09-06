// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Daniel Brandenburg

#pragma once

#include <JANA/JEvent.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "algorithms/reco/PrimaryVertices.h"
#include "algorithms/reco/PrimaryVerticesConfig.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class PrimaryVertices_factory :
        public JOmniFactory<PrimaryVertices_factory, PrimaryVerticesConfig> {

public:
    using AlgoT = eicrecon::PrimaryVertices;
private:
    std::unique_ptr<AlgoT> m_algo;

    PodioInput<edm4eic::Vertex> m_rc_vertices_input {this};

    // Declare outputs
    PodioOutput<edm4eic::Vertex> m_out_primary_vertices {this};

public:
    void Configure() {
        m_algo = std::make_unique<AlgoT>();
        m_algo->init(logger());
        m_algo->applyConfig(config());
    }

    void ChangeRun(int64_t run_number) {
    }

    void Process(int64_t run_number, uint64_t event_number) {
        auto output = m_algo->execute(m_rc_vertices_input());
        m_out_primary_vertices() = std::move(output);
    }
};

} // eicrecon
