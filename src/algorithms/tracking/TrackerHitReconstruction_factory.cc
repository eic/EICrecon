// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//
#include <spdlog/sinks/stdout_color_sinks.h>
#include "TrackerHitReconstruction_factory.h"
#include "services/geometry/dd4hep/JDD4hep_service.h"
#include <eicd/RawTrackerHit.h>
#include <JANA/JEvent.h>

void TrackerHitReconstruction_factory::Init() {

    // This prefix will be used for parameters
    std::string param_prefix = "TrkHitReco_" + GetTag();   // Will be something like SiTrkDigi_BarrelTrackerRawHit

    // Create plugin level sub-log
    m_log = spdlog::stdout_color_mt("TrackerHitReconstruction_factory");

    // get geometry service
    auto geo_service = GetApplication()->GetService<JDD4hep_service>();

    // Ask service locator for parameter manager. We want to get this plugin parameters.
    auto pm = this->GetApplication()->GetJParameterManager();
    pm->SetDefaultParameter(param_prefix + ":time_resolution", m_reco_algo.getConfig().time_resolution, "threshold");
    pm->SetDefaultParameter(param_prefix + ":verbose", m_verbose, "verbosity: 0 - none, 1 - default, 2 - debug, 3 - trace");
    pm->SetDefaultParameter(param_prefix + ":input_tags", m_input_tag, "Input data tag name");

    // This level will work for this plugin only
    switch (m_verbose) {
        case 0:
            m_log->set_level(spdlog::level::warn); break;
        case 2:
            m_log->set_level(spdlog::level::debug); break;
        case 3:
            m_log->set_level(spdlog::level::trace); break;
        default:
            m_log->set_level(spdlog::level::info); break;
    }

    // Initialize reconstruction algorithm
    m_reco_algo.init(geo_service->detector(), m_log);
}

void TrackerHitReconstruction_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {
    // nothing here
}

void TrackerHitReconstruction_factory::Process(const std::shared_ptr<const JEvent> &event) {
    // Now we check that user provided an input names
    std::string input_tag = m_input_tag;
    if(input_tag.empty()) {
        input_tag = GetDefaultInputTags()[0];
    }

    // Get RawTrackerHit-s with the proper tag
    auto raw_hits = event->Get<eicd::RawTrackerHit>(input_tag);

    // Output array
    std::vector<eicd::TrackerHit*> hits;

    // Create output hits using TrackerHitReconstruction algorithm
    for(auto raw_hit: raw_hits){
        hits.push_back(m_reco_algo.produce(raw_hit));
    }
}
