// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include "TrackerHitCollector_factory.h"
#include "services/log/Log_service.h"
#include "extensions/spdlog/SpdlogExtensions.h"
#include <JANA/JEvent.h>

namespace eicrecon {
    void TrackerHitCollector_factory::Init() {
        // Ask JApplication and parameter managers
        auto app =  this->GetApplication();

        // This prefix will be used for parameters
        std::string param_prefix = "Tracking:" + GetTag();   // Will be something like SiTrkDigi_BarrelTrackerRawHit

        // Now we check that user provided an input names
        app->SetDefaultParameter(param_prefix + ":input_tags", m_input_tags, "Input data tag name");
        if(m_input_tags.empty()) {
            m_input_tags = GetDefaultInputTags();
        }

        // Logger and log level from user parameter or default
        m_log = app->GetService<Log_service>()->logger(param_prefix);
        std::string log_level_str = "info";
        app->SetDefaultParameter(param_prefix + ":LogLevel", log_level_str, "verbosity: trace, debug, info, warn, err, critical, off");
        m_log->set_level(eicrecon::ParseLogLevel(log_level_str));

        // jana should not delete edm4eic::TrackerHit from this factory
        // TrackerHits created by other factories, this factory only collect them together
        SetFactoryFlag(JFactory::NOT_OBJECT_OWNER);

    }

    void TrackerHitCollector_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {
    }

    void TrackerHitCollector_factory::Process(const std::shared_ptr<const JEvent> &event) {
        std::vector<edm4eic::TrackerHit*> total_hits;

        // Just collect hits together
        for(auto input_tag: m_input_tags) {
            auto hits = event->Get<edm4eic::TrackerHit>(input_tag);
            for (const auto hit : hits) {
                total_hits.push_back(const_cast<edm4eic::TrackerHit*>(hit));
            }
        }
        Set(total_hits);
    }
} // eicrecon