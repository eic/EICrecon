// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <JANA/JEvent.h>
#include "ReconstructedParticle_factory.h"
#include "services/log/Log_service.h"
#include "extensions/spdlog/SpdlogExtensions.h"
#include "ParticlesFromTrackFitResult.h"

namespace eicrecon {
    void ReconstructedParticle_factory::Init() {
        auto app = GetApplication();

        // This prefix will be used for parameters
        std::string param_prefix = "GlbReco:" + GetTag();

        // Get input data tags
        app->SetDefaultParameter(param_prefix + ":InputTags", m_input_tags, "Input data tag name");
        if(m_input_tags.empty()) {
            m_input_tags = GetDefaultInputTags();
        }

        // Logger. Get plugin level sub-log
        m_log = app->GetService<Log_service>()->logger(param_prefix);

        // Get log level from user parameter or default
        std::string log_level_str = "info";
        app->SetDefaultParameter(param_prefix + ":LogLevel", log_level_str, "LogLevel: trace, debug, info, warn, err, critical, off");
        m_log->set_level(eicrecon::ParseLogLevel(log_level_str));
    }

    void ReconstructedParticle_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {
        // Nothing to do here
    }

    void ReconstructedParticle_factory::Process(const std::shared_ptr<const JEvent> &event) {
        auto tracking_data = event->GetSingle<ParticlesFromTrackFitResult>("CentralTrackingParticles");
        std::vector<edm4eic::ReconstructedParticle*> result;
        for(size_t i=0; i < tracking_data->particles()->size(); i++) {
            auto particle = (*tracking_data->particles())[i];
            result.push_back(new edm4eic::ReconstructedParticle(particle));
        }
        Set(result);
    }
} // eicrecon