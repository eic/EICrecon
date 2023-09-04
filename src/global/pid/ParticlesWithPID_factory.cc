// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <JANA/JEvent.h>
#include <edm4eic/TrackParameters.h>
#include "ParticlesWithPID_factory.h"


void eicrecon::ParticlesWithPID_factory::Init() {
    auto app = GetApplication();

    // SpdlogMixin logger initialization, sets m_log
    InitLogger(app, GetPrefix(), "info");
    m_matching_algo.init(m_log);
}

void eicrecon::ParticlesWithPID_factory::Process(const std::shared_ptr<const JEvent> &event) {
    auto mc_particles = static_cast<const edm4hep::MCParticleCollection*>(event->GetCollectionBase(GetInputTags().at(0)));
    auto trajectories = static_cast<const edm4eic::TrajectoryCollection*>(event->GetCollectionBase(GetInputTags().at(1)));
    std::vector<const edm4eic::CherenkovParticleIDCollection*> cherenkov_pids;
    cherenkov_pids.push_back(static_cast<const edm4eic::CherenkovParticleIDCollection*>(event->GetCollectionBase(GetInputTags().at(2)))); // DRICH

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
