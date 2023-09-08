// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <JANA/JEvent.h>
#include <edm4eic/CherenkovParticleIDCollection.h>
#include <edm4eic/TrajectoryCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <exception>

#include "ParticlesWithPID_factory.h"
#include "datamodel_glue.h"


void eicrecon::ParticlesWithPID_factory::Init() {
    auto app = GetApplication();

    // SpdlogMixin logger initialization, sets m_log
    InitLogger(app, GetPrefix(), "info");
    m_matching_algo.init(m_log);
}

void eicrecon::ParticlesWithPID_factory::Process(const std::shared_ptr<const JEvent> &event) {
    auto mc_particles = event->GetCollection<edm4hep::MCParticle>(GetInputTags().at(0));
    auto trajectories = event->GetCollection<edm4eic::Trajectory>(GetInputTags().at(1));
    std::vector<const edm4eic::CherenkovParticleIDCollection*> cherenkov_pids;
    cherenkov_pids.push_back(event->GetCollection<edm4eic::CherenkovParticleID>(GetInputTags().at(2))); // DRICH

    try {
        auto result = m_matching_algo.process(mc_particles, trajectories, cherenkov_pids);
        SetCollection<edm4eic::ReconstructedParticle>(GetOutputTags().at(0),     std::move(result.parts));
        SetCollection<edm4eic::MCRecoParticleAssociation>(GetOutputTags().at(1), std::move(result.assocs));
        SetCollection<edm4hep::ParticleID>(GetOutputTags().at(2),                std::move(result.pids));
    }
    catch(std::exception &e) {
        throw JException(e.what());
    }
}
