// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>
#include "SiliconTrackerDigi_factory.h"


void eicrecon::SiliconTrackerDigi_factory::Init() {
    auto app = GetApplication();
    auto pm = app->GetJParameterManager();

    // We will use plugin name to get parameters for correct factory
    // So if we use <plugin name>:parameter whichever plugin uses this template. eg:
    //    "BTRK:parameter" or "FarForward:parameter"
    // That has limitations but the convenient in the most of the cases
    std::string param_prefix = "SiliconTrackerDigi:" + GetTag();   // Will be something like SiTrkDigi_BarrelTrackerRawHit

    // Set input tags
    pm->SetDefaultParameter(param_prefix + ":InputTags", m_input_tags, "Input data tag names");
    if(m_input_tags.empty()) {
        m_input_tags = GetDefaultInputTags();
    }

    // Logger. Get plugin level sub-log
    m_log = app->GetService<Log_service>()->logger("SiliconTrackerDigi");

    // Get log level from user parameter or default
    std::string log_level_str = "info";
    pm->SetDefaultParameter(param_prefix + ":LogLevel", log_level_str, "verbosity: trace, debug, info, warn, err, critical, off");
    m_log->set_level(eicrecon::ParseLogLevel(log_level_str));

    // Setup digitization algorithm
    auto &m_cfg = m_digi_algo.getConfig();
    pm->SetDefaultParameter(param_prefix + ":Threshold", m_cfg.threshold, "threshold");
    pm->SetDefaultParameter(param_prefix + ":TimeResolution", m_cfg.timeResolution, "Time resolution. Probably ns. Fix my units!!!!");

    // Initialize digitization algorithm
    m_digi_algo.init(m_log);
}

void eicrecon::SiliconTrackerDigi_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {

}

void eicrecon::SiliconTrackerDigi_factory::Process(const std::shared_ptr<const JEvent> &event) {

    // Collect all hits from different tags
    std::vector<const edm4hep::SimTrackerHit*> total_sim_hits;
    for(const auto &input_tag: m_input_tags) {
        auto sim_hits = event->Get<edm4hep::SimTrackerHit>(input_tag);
        for (const auto hit : sim_hits) {
            total_sim_hits.push_back(hit);                     /// TODO a better way to concatenate arrays
        }
    }

    // RUN algorithm
    auto digitised_hits = m_digi_algo.produce(total_sim_hits);  // Digitize hits
    this->Set(digitised_hits);                                                       // Add data as a factory output

    // >oO debug
    m_log->trace("SiliconTrackerDigi_factoryT<>::Process(...) end\n");
}

eicrecon::SiliconTrackerDigi_factory::SiliconTrackerDigi_factory(const std::vector<std::string> &default_input_tags)
        : JChainFactoryT(default_input_tags) {}
