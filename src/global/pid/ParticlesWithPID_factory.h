// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <JANA/JEvent.h>
#include <JANA/JException.h>
#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/ParticleIDCollection.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "algorithms/pid/ParticlesWithPID.h"
#include "algorithms/pid/ParticlesWithPIDConfig.h"
#include "extensions/jana/JOmniFactory.h"


namespace eicrecon {

class ParticlesWithPID_factory : public JOmniFactory<ParticlesWithPID_factory, ParticlesWithPIDConfig> {

private:
    ParticlesWithPID m_algo;

    PodioInput<edm4hep::MCParticle> m_mc_particles_input {this};
    PodioInput<edm4eic::Trajectory> m_trajectories_input {this};
    PodioInput<edm4eic::CherenkovParticleID> m_drich_particle_id_input {this};

    PodioOutput<edm4eic::Track> m_tracks_output {this};
    PodioOutput<edm4eic::ReconstructedParticle> m_particles_output {this};
    PodioOutput<edm4eic::MCRecoParticleAssociation> m_particles_assoc_output {this};
    PodioOutput<edm4hep::ParticleID> m_particle_id_output {this};

    ParameterRef<double> m_momentumRelativeTolerance {this, "momentumRelativeTolerance", config().momentumRelativeTolerance};
    ParameterRef<double> m_phiTolerance {this, "phiTolerance", config().phiTolerance};
    ParameterRef<double> m_etaTolerance {this, "etaTolerance", config().etaTolerance};

public:
    void Configure() {
        m_algo.applyConfig(config());
        m_algo.init(logger());
    }

    void ChangeRun(int64_t run_number) {
    }

    void Process(int64_t run_number, uint64_t event_number) {
        std::tie(m_tracks_output(), m_particles_output(), m_particles_assoc_output(), m_particle_id_output()) = m_algo.process(m_mc_particles_input(), m_trajectories_input(), m_drich_particle_id_input());
    }
};

} // eicrecon
