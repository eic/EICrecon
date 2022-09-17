// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include "TrackingParticles_factory.h"
#include "services/log/Log_service.h"
#include "extensions/spdlog/SpdlogExtensions.h"
#include <JANA/JEvent.h>

void TrackingParticles_factory::Init() {
    // This prefix will be used for parameters
    std::string param_prefix = "CKFTracking:" + GetTag();   // Will be something like SiTrkDigi_BarrelTrackerRawHit

    auto app = GetApplication();

    // Logger and log level from user parameter or default
    m_log = app->GetService<Log_service>()->logger(param_prefix);

    // Ask service locator for parameter manager. We want to get this plugin parameters.
    auto pm = this->GetApplication()->GetJParameterManager();
    std::string log_level_str = "info";
    pm->SetDefaultParameter(param_prefix + ":LogLevel", log_level_str, "verbosity: trace, debug, info, warn, err, critical, off");
    m_log->set_level(eicrecon::ParseLogLevel(log_level_str));

    // Now we check that user provided an input names
    pm->SetDefaultParameter(param_prefix + ":input_tags", m_input_tags, "Input data tag name");
    if(m_input_tags.size() == 0) {
        m_input_tags = GetDefaultInputTags();
    }

    m_particle_maker_algo.init(m_log);
}

void TrackingParticles_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {

}

void TrackingParticles_factory::Process(const std::shared_ptr<const JEvent> &event) {
    // Now we check that user provided an input names
    std::string input_tag = m_input_tags[0];

    // Collect all hits
    auto trajectories = event->Get<Jug::Trajectories>(input_tag);
    auto result = m_particle_maker_algo.execute(trajectories);
    Insert(result);
}
