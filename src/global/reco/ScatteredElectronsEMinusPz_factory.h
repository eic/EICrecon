// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Wouter Deconinck

#pragma once

#include <JANA/JEvent.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "algorithms/reco/ScatteredElectronsEMinusPz.h"
#include "algorithms/reco/ScatteredElectronsEMinusPzConfig.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class ScatteredElectronsEMinusPz_factory :
        public JOmniFactory<ScatteredElectronsEMinusPz_factory, ScatteredElectronsEMinusPzConfig> {

public:
    using AlgoT = eicrecon::ScatteredElectronsEMinusPz;
private:
    std::unique_ptr<AlgoT> m_algo;

    PodioInput<edm4eic::ReconstructedParticle> m_rc_particles_input {this};
    PodioInput<edm4eic::ReconstructedParticle> m_rc_electrons_input {this};
    
    // Declare outputs
    PodioOutput<edm4eic::ReconstructedParticle> m_out_reco_particles {this};

public:
    void Configure() {
        m_algo = std::make_unique<AlgoT>();
        m_algo->init(logger());
        m_algo->applyConfig(config());
    }

    void ChangeRun(int64_t run_number) {
    }

    void Process(int64_t run_number, uint64_t event_number) {
        auto output = m_algo->execute(m_rc_particles_input(), m_rc_electrons_input());
        m_out_reco_particles() = std::move(output);
    }
};

} // eicrecon
