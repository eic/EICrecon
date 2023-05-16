// Created by Simon Gardner
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <JANA/JEvent.h>
#include <edm4eic/TrackerHitCollection.h>

#include "TrackerParticleCollector_factory.h"
#include "services/log/Log_service.h"
#include "extensions/spdlog/SpdlogExtensions.h"
#include "extensions/string/StringHelpers.h"

namespace eicrecon {
    void TrackerParticleCollector_factory::Init() {
        // Ask JApplication and parameter managers
        auto app =  this->GetApplication();

        // This prefix will be used for parameters
        std::string plugin_name = eicrecon::str::ReplaceAll(GetPluginName(), ".so", "");
        std::string param_prefix = plugin_name+ ":" + GetTag();

        // Now we check that user provided an input names
        app->SetDefaultParameter(param_prefix + ":InputTags", m_input_tags, "Input data tag name");
        if(m_input_tags.empty()) {
            m_input_tags = GetDefaultInputTags();
        }

        // Logger and log level from user parameter or default
        m_log = app->GetService<Log_service>()->logger(param_prefix);
        std::string log_level_str = "info";
        app->SetDefaultParameter(param_prefix + ":LogLevel", log_level_str, "Log level: trace, debug, info, warn, err, critical, off");
        m_log->set_level(eicrecon::ParseLogLevel(log_level_str));

        // jana should not delete edm4eic::TrackerHit from this factory
        // TrackerHits created by other factories, this factory only collect them together
        SetFactoryFlag(JFactory::NOT_OBJECT_OWNER);

    }

    void TrackerParticleCollector_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {
    }

    void TrackerParticleCollector_factory::Process(const std::shared_ptr<const JEvent> &event) {
        std::vector<edm4eic::TrackParameters*> total_hits;

        // Just collect hits together
        for(auto input_tag: m_input_tags) {
            auto hits = event->Get<edm4eic::TrackParameters>(input_tag);
            for (const auto hit : hits) {	      
                total_hits.push_back(const_cast<edm4eic::TrackParameters*>(hit));
            }
        }	
        Set(total_hits);
    }
} // eicrecon
