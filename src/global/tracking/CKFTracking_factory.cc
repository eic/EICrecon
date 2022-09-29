// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//


#include <Acts/Propagator/Navigator.hpp>
#include "CKFTracking_factory.h"
#include "extensions/spdlog/SpdlogExtensions.h"
#include <JANA/JEvent.h>
#include "services/geometry/acts/ACTSGeo_service.h"
#include "services/log/Log_service.h"
#include <services/geometry/dd4hep/JDD4hep_service.h>

void eicrecon::CKFTracking_factory::Init() {
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
    if(m_input_tags.empty()) {
        m_input_tags = GetDefaultInputTags();
    }

    // Get ACTS context from ACTSGeo service
    auto acts_service = GetApplication()->GetService<ACTSGeo_service>();
    auto dd4hp_service = GetApplication()->GetService<JDD4hep_service>();

    // Initialize algorithm
    auto cellid_converter = std::make_shared<const dd4hep::rec::CellIDPositionConverter>(*dd4hp_service->detector());

    m_tracking_algo.init(acts_service->actsGeoProvider(), m_log);
}

void eicrecon::CKFTracking_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {

}

void eicrecon::CKFTracking_factory::Process(const std::shared_ptr<const JEvent> &event) {
    // Now we check that user provided an input names
    std::string input_tag = m_input_tags[0];

    // Collect all hits
    auto source_linker_result = event->GetSingle<eicrecon::TrackerSourceLinkerResult>(input_tag);
    auto track_parameters = event->Get<Jug::TrackParameters>("InitTrackParams");
    Jug::TrackParametersContainer acts_track_params;
    for(auto track_params_item: track_parameters) {
        acts_track_params.push_back(*track_params_item);
    }

    // Reading the geometry may take a long time and if the JANA ticker is enabled, it will keep printing
    // while no other output is coming which makes it look like something is wrong. Disable the ticker
    // while parsing and loading the geometry
    auto tickerEnabled = GetApplication()->IsTickerEnabled();
    GetApplication()->SetTicker( false );


    // Convert vector of source links to a sorted in geometry order container used in tracking
    Jug::IndexSourceLinkContainer source_links;
    auto measurements_ptr = source_linker_result->measurements;
    for(auto &sourceLink: source_linker_result->sourceLinks){
        // add to output containers. since the input is already geometry-order,
        // new elements in geometry containers can just be appended at the end.
        source_links.emplace_hint(source_links.end(), *sourceLink);
    }

    // >oO Debug output for SourceLinks
    if(m_log->level() <= spdlog::level::trace) {
        m_log->trace("Checking Source links: ");
        for(auto sourceLink: source_links) {
            m_log->trace("   index: {:<5} geometryId: {}", sourceLink.get().index(), sourceLink.get().geometryId().value());
        }
    }
    m_log->debug("Source links count: {}", source_links.size());
    m_log->debug("Measurements count: {}", source_linker_result->measurements->size());
    m_log->debug("Diving into tracking...");

    // RUN TRACKING ALGORITHM
    auto trajectories = m_tracking_algo.process(
            source_links,
            *source_linker_result->measurements,
            acts_track_params);

    // Save the result
    Set(trajectories);

    // Enable ticker back
    GetApplication()->SetTicker(tickerEnabled);
}
