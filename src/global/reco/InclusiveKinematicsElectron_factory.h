// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Wouter Deconinck

#pragma once

#include <JANA/JEvent.h>
#include <edm4eic/InclusiveKinematicsCollection.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "algorithms/reco/InclusiveKinematicsElectron.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class InclusiveKinematicsElectron_factory :
        public JOmniFactory<InclusiveKinematicsElectron_factory> {

public:
    using AlgoT = eicrecon::InclusiveKinematicsElectron;
private:
    std::unique_ptr<AlgoT> m_algo;

    PodioInput<edm4hep::MCParticle> m_mc_particles_input {this};
    PodioInput<edm4eic::ReconstructedParticle> m_rc_particles_input {this};
    PodioInput<edm4eic::MCRecoParticleAssociation> m_rc_particles_assoc_input {this};
    PodioOutput<edm4eic::InclusiveKinematics> m_inclusive_kinematics_output {this};

public:
    void Configure() {
        m_algo = std::make_unique<AlgoT>(GetPrefix());
        m_algo->init(logger());
    }

    void ChangeRun(int64_t run_number) {
    }

    void Process(int64_t run_number, uint64_t event_number) {
        m_algo->process({m_mc_particles_input(), m_rc_particles_input(), m_rc_particles_assoc_input()},
                        {m_inclusive_kinematics_output().get()});
    }
};

} // eicrecon
