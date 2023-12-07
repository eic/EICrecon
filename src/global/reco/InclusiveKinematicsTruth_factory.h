// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Wouter Deconinck

#pragma once

#include <JANA/JEvent.h>
#include <edm4eic/InclusiveKinematicsCollection.h>
#include <cstddef>
#include <memory>
#include <string>
#include <typeindex>
#include <utility>
#include <vector>

#include "algorithms/reco/InclusiveKinematicsTruth.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class InclusiveKinematicsTruth_factory :
        public JOmniFactory<InclusiveKinematicsTruth_factory> {

private:
    using AlgoT = eicrecon::InclusiveKinematicsTruth;
    std::unique_ptr<AlgoT> m_algo;

    PodioInput<edm4hep::MCParticle> m_mc_particles_input {this};
    PodioOutput<edm4eic::InclusiveKinematics> m_inclusive_kinematics_output {this};

public:
    void Configure() {
        m_algo = std::make_unique<AlgoT>();
        m_algo->init(logger());
    }

    void ChangeRun(int64_t run_number) {
    }

    void Process(int64_t run_number, uint64_t event_number) {
        m_inclusive_kinematics_output() = m_algo->execute(*m_mc_particles_input());
    }
};

} // eicrecon
