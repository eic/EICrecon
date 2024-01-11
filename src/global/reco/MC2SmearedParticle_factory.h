// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <JANA/JEvent.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <cstddef>
#include <memory>
#include <string>
#include <typeindex>
#include <utility>
#include <vector>

#include "algorithms/reco/MC2SmearedParticle.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class MC2SmearedParticle_factory :
        public JOmniFactory<MC2SmearedParticle_factory> {

private:
    using AlgoT = eicrecon::MC2SmearedParticle;
    std::unique_ptr<AlgoT> m_algo;

    PodioInput<edm4hep::MCParticle> m_mc_particles_input {this};
    PodioOutput<edm4eic::ReconstructedParticle> m_rc_particles_output {this};

public:
    void Configure() {
        m_algo = std::make_unique<AlgoT>();
        m_algo->init(logger());
    }

    void ChangeRun(int64_t run_number) {
    }

    void Process(int64_t run_number, uint64_t event_number) {
        m_rc_particles_output() = m_algo->produce(m_mc_particles_input());
    }
};

} // eicrecon
