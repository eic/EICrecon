// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <JANA/JEvent.h>
#include <edm4eic/TrackParameters.h>
#include "extensions/string/StringHelpers.h"
#include "ParticlesWithTruthPID_factory.h"


void eicrecon::ParticlesWithTruthPID_factory::Init() {
    auto app = GetApplication();

    // This prefix will be used for parameters
    std::string plugin_name  = eicrecon::str::ReplaceAll(GetPluginName(), ".so", "");
    std::string param_prefix = plugin_name + ":" + GetTag();

    // SpdlogMixin logger initialization, sets m_log
    InitLogger(GetPrefix(), "info");
    m_matching_algo.init(m_log);
}

void eicrecon::ParticlesWithTruthPID_factory::Process(const std::shared_ptr<const JEvent> &event) {
    // TODO: NWB: We are using GetCollectionBase because GetCollection is temporarily out of commission due to JFactoryPodioTFixed
    // auto mc_particles = event->GetCollection<edm4hep::MCParticle>(GetInputTags()[0]);
    // auto track_params = event->GetCollection<edm4eic::TrackParameters>(GetInputTags()[1]);
    auto mc_particles = static_cast<const edm4hep::MCParticleCollection*>(event->GetCollectionBase(GetInputTags()[0]));
    auto track_params = static_cast<const edm4eic::TrackParametersCollection*>(event->GetCollectionBase(GetInputTags()[1]));

    try {
        auto prt_with_assoc = m_matching_algo.process(mc_particles, track_params);
        SetCollection<edm4eic::ReconstructedParticle>(GetOutputTags()[0], std::move(prt_with_assoc.first));
        SetCollection<edm4eic::MCRecoParticleAssociation>(GetOutputTags()[1], std::move(prt_with_assoc.second));
    }
    catch(std::exception &e) {
        m_log->warn("Exception in underlying algorithm: {}. Event data will be skipped", e.what());
    }
}
