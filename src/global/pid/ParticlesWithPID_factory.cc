// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <JANA/JEvent.h>
#include <edm4eic/TrackParameters.h>
#include "extensions/string/StringHelpers.h"
#include "ParticlesWithPID_factory.h"


void eicrecon::ParticlesWithPID_factory::Init() {
    auto app = GetApplication();

    // This prefix will be used for parameters
    std::string plugin_name  = eicrecon::str::ReplaceAll(GetPluginName(), ".so", "");
    std::string param_prefix = plugin_name + GetPrefix();

    // SpdlogMixin logger initialization, sets m_log
    InitLogger(param_prefix, "info");
    m_matching_algo.init(m_log);
}

void eicrecon::ParticlesWithPID_factory::Process(const std::shared_ptr<const JEvent> &event) {
    // TODO: NWB: We are using GetCollectionBase because GetCollection is temporarily out of commission due to JFactoryPodioTFixed
    // auto mc_particles = event->GetCollection<edm4hep::MCParticle>(GetInputTags()[0]);
    // auto track_params = event->GetCollection<edm4eic::TrackParameters>(GetInputTags()[1]);
    auto mc_particles = static_cast<const edm4hep::MCParticleCollection*>(event->GetCollectionBase(GetInputTags().at(0)));
    auto track_params = static_cast<const edm4eic::TrackParametersCollection*>(event->GetCollectionBase(GetInputTags().at(1)));
    std::vector<const edm4eic::CherenkovParticleIDCollection*> cherenkov_pids;
    cherenkov_pids.push_back(static_cast<const edm4eic::CherenkovParticleIDCollection*>(event->GetCollectionBase(GetInputTags().at(2)))); // DRICH

    try {
        auto result = m_matching_algo.process(mc_particles, track_params, cherenkov_pids);
        SetCollection<edm4eic::ReconstructedParticle>(GetOutputTags().at(0),     std::move(result.parts));
        SetCollection<edm4eic::MCRecoParticleAssociation>(GetOutputTags().at(1), std::move(result.assocs));
        SetCollection<edm4hep::ParticleID>(GetOutputTags().at(2),                std::move(result.pids));
    }
    catch(std::exception &e) {
        m_log->warn("Exception in underlying algorithm: {}. Event data will be skipped", e.what());
    }
}
