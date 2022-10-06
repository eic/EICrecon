// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//
#include <spdlog/sinks/stdout_color_sinks.h>
#include "TrackerHitReconstruction_factory.h"
#include "services/geometry/dd4hep/JDD4hep_service.h"
#include "services/log/Log_service.h"
#include "extensions/spdlog/SpdlogExtensions.h"
#include <edm4eic/RawTrackerHit.h>
#include <JANA/JEvent.h>

void TrackerHitReconstruction_factory::Init() {

    auto app = GetApplication();

    // This prefix will be used for parameters
    std::string param_prefix = "Tracking:" + GetTag();   // Will be something like SiTrkDigi_BarrelTrackerRawHit

    // Ask service locator for parameter manager. We want to get this plugin parameters.
    auto pm = app->GetJParameterManager();
    pm->SetDefaultParameter(param_prefix + ":TimeResolution", m_reco_algo.getConfig().time_resolution, "threshold");

    // Get input data tags
    pm->SetDefaultParameter(param_prefix + ":InputTags", m_input_tags, "Input data tag name");
    if(m_input_tags.empty()) {
        m_input_tags = GetDefaultInputTags();
    }

    // Logger. Get plugin level sub-log
    m_log = app->GetService<Log_service>()->logger(param_prefix);

    // Get log level from user parameter or default
    std::string log_level_str = "info";
    pm->SetDefaultParameter(param_prefix + ":LogLevel", log_level_str, "LogLevel: trace, debug, info, warn, err, critical, off");
    m_log->set_level(eicrecon::ParseLogLevel(log_level_str));

    // get geometry service
    auto geo_service = app->GetService<JDD4hep_service>();

    // Initialize reconstruction algorithm
    m_reco_algo.init(geo_service->detector(), m_log);
}

void TrackerHitReconstruction_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {
    // nothing here
}

void TrackerHitReconstruction_factory::Process(const std::shared_ptr<const JEvent> &event) {

    // Get RawTrackerHit-s with the proper tag
    auto raw_hits = event->Get<edm4eic::RawTrackerHit>(m_input_tags[0]);

    // Output array
    std::vector<edm4eic::TrackerHit*> hits;

    // Create output hits using TrackerHitReconstruction algorithm
    for(auto raw_hit: raw_hits){
        hits.push_back(m_reco_algo.produce(raw_hit));
    }

    Set(hits);

    m_log->debug("End of process. Hits count: {}", hits.size());
}
