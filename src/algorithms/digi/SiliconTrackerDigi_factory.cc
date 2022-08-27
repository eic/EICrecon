// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <spdlog/sinks/stdout_color_sinks.h>
#include "SiliconTrackerDigi_factory.h"

void eicrecon::SiliconTrackerDigi_factory::Init() {
    // We will use plugin name to get parameters for correct factory
    // So if we use <plugin name>:parameter whichever plugin uses this template. eg:
    //    "BTRK:parameter" or "FarForward:parameter"
    // That has limitations but the convenient in the most of the cases
    std::string param_prefix = "SiTrkDigi_" + GetTag();   // Will be something like SiTrkDigi_BarrelTrackerRawHit

    // Create plugin level sub-log
    m_log = spdlog::stdout_color_mt("SiliconTrackerDigi_factory");

    // This level will work for this plugin only
    m_log->set_level(spdlog::level::debug);

    // Setup digitisation algorithm
    m_digi_algo.setLogger(m_log);
    auto &m_cfg = m_digi_algo.getConfig();
    m_digi_algo.init();

    // Ask service locator for parameter manager. We want to get this plugin parameters.
    auto pm = this->GetApplication()->GetJParameterManager();
    pm->SetDefaultParameter(param_prefix + ":threshold", m_cfg.threshold, "threshold");
    pm->SetDefaultParameter(param_prefix + ":time_res", m_cfg.timeResolution, "Time resolution. Probably ns. Fix my units!!!!");

    pm->SetDefaultParameter(param_prefix + ":verbose", m_verbose, "verbosity: 0 - none, 1 - default, 2 - debug, 3 - trace");
    pm->SetDefaultParameter(param_prefix + ":input_tags", m_input_tags, "Input data tag names");
}

void eicrecon::SiliconTrackerDigi_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {

}

void eicrecon::SiliconTrackerDigi_factory::Process(const std::shared_ptr<const JEvent> &event) {

    // Now we check that user provided an input names
    std::vector<std::string> &input_tags = m_input_tags;
    if(input_tags.size() == 0) {
        input_tags = GetDefaultInputTags();
    }

    // Collect all hits
    std::vector<const edm4hep::SimTrackerHit*> total_sim_hits;
    for(auto input_tag: input_tags) {
        auto simHits = event->Get<edm4hep::SimTrackerHit>(input_tag);
        for (const auto ahit : simHits) {
            total_sim_hits.push_back(ahit);                     /// TODO a better way to concatenate arrays
        }
    }

    // Digitize hits
    auto digitised_hits = m_digi_algo.produce(total_sim_hits);

    // Add data as a factory output
    this->Set(digitised_hits);

    // >oO debug
    m_log->trace("SiliconTrackerDigi_factoryT<>::Process(...) end\n");
}

eicrecon::SiliconTrackerDigi_factory::SiliconTrackerDigi_factory(const std::vector<std::string> &default_input_tags)
        : JChainFactoryT(default_input_tags) {}
