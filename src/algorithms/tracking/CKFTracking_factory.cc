// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//
#include "CKFTracking_factory.h"
#include "services/geometry/acts/ACTSGeo_service.h"
#include <services/geometry/dd4hep/JDD4hep_service.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <JANA/JEvent.h>

void eicrecon::CKFTracking_factory::Init() {
    // This prefix will be used for parameters
    std::string param_prefix = "CKFTracking_" + GetTag();   // Will be something like SiTrkDigi_BarrelTrackerRawHit

    // Create plugin level sub-log
    m_log = spdlog::stdout_color_mt("CKFTracking_factory");

    // Ask service locator for parameter manager. We want to get this plugin parameters.
    auto pm = this->GetApplication()->GetJParameterManager();

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

    // Get ACTS context from ACTSGeo service
    auto acts_service = GetApplication()->GetService<ACTSGeo_service>();


    auto dd4hp_service = GetApplication()->GetService<JDD4hep_service>();

    // Initialize algorithm
    auto cellid_converter = std::make_shared<const dd4hep::rec::CellIDPositionConverter>(*dd4hp_service->detector());

    m_tracking_algo.initialize(acts_service->acts_context());
}

void eicrecon::CKFTracking_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {

}

void eicrecon::CKFTracking_factory::Process(const std::shared_ptr<const JEvent> &event) {
    // Now we check that user provided an input names
    std::string input_tag = m_input_tag;
    if(input_tag.empty()) {
        input_tag = GetDefaultInputTags()[0];
    }

    // Collect all hits
    auto source_linker_result = event->GetSingle<eicrecon::TrackerSourceLinkerResult>(input_tag);
    auto track_parameters = event->Get<Jug::TrackParameters>();
    Jug::TrackParametersContainer acts_track_params;
    for(auto track_params_item: track_parameters) {
        acts_track_params.push_back(*track_params_item);
    }


    auto trajectories = m_tracking_algo.execute(source_linker_result->sourceLinks, source_linker_result->measurements, acts_track_params);

    Set(trajectories);

}
