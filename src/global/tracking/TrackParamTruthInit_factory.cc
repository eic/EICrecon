// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include "TrackParamTruthInit_factory.h"

void eicrecon::TrackParamTruthInit_factory::Init() {
    auto app = GetApplication();

    // This prefix will be used for parameters
    std::string plugin_name = GetPluginName();
    std::string param_prefix = plugin_name+ ":" + GetTag();

    // Initialize input tags
    InitDataTags(param_prefix);

    // Initialize logger
    InitLogger(app, param_prefix, "info");

    // Algorithm configuration
    auto cfg = GetDefaultConfig();
    app->SetDefaultParameter(param_prefix + ":MaxVertexX", cfg.m_maxVertexX , "Maximum abs(vertex x) for truth tracks turned into seed");
    app->SetDefaultParameter(param_prefix + ":MaxVertexY", cfg.m_maxVertexY , "Maximum abs(vertex y) for truth tracks turned into seed");
    app->SetDefaultParameter(param_prefix + ":MaxVertexZ", cfg.m_maxVertexZ , "Maximum abs(vertex z) for truth tracks turned into seed");
    app->SetDefaultParameter(param_prefix + ":MinMomentum", cfg.m_minMomentum , "Minimum momentum for truth tracks turned into seed");
    app->SetDefaultParameter(param_prefix + ":MaxEtaForward", cfg.m_maxEtaForward , "Maximum forward abs(eta) for truth tracks turned into seed");
    app->SetDefaultParameter(param_prefix + ":MaxEtaBackward", cfg.m_maxEtaBackward , "Maximum backward abs(eta) for truth tracks turned into seed");
    app->SetDefaultParameter(param_prefix + ":MomentumSmear", cfg.m_momentumSmear, "Momentum magnitude fraction to use as width of gaussian smearing");

    // Initialize algorithm
    m_seeding_algo.applyConfig(cfg);
    m_seeding_algo.init(m_log);
}

void eicrecon::TrackParamTruthInit_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {
}

void eicrecon::TrackParamTruthInit_factory::Process(const std::shared_ptr<const JEvent> &event) {
    auto mc_particles = static_cast<const edm4hep::MCParticleCollection*>(event->GetCollectionBase(GetInputTags()[0]));

    try {
        auto output = m_seeding_algo.produce(mc_particles);
        SetCollection(std::move(output));
    }
    catch(std::exception &e) {
        throw JException(e.what());
    }
}
