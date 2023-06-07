// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//


#include <Acts/Propagator/Navigator.hpp>
#include "CKFSeededTracking_factory.h"
#include "extensions/spdlog/SpdlogExtensions.h"
#include <JANA/JEvent.h>
#include "services/geometry/acts/ACTSGeo_service.h"
#include "services/log/Log_service.h"
#include "extensions/string/StringHelpers.h"
#include <services/geometry/dd4hep/JDD4hep_service.h>

void eicrecon::CKFSeededTracking_factory::Init() {
    auto app = GetApplication();

    // This prefix will be used for parameters
    std::string plugin_name = eicrecon::str::ReplaceAll(GetPluginName(), ".so", "");
    std::string param_prefix = plugin_name+ ":" + GetTag();

    // Initialize input tags
    InitDataTags(param_prefix);

    // Initialize logger
    InitLogger(param_prefix, "info");

    // Get ACTS context from ACTSGeo service
    auto acts_service = app->GetService<ACTSGeo_service>();
    auto dd4hp_service = app->GetService<JDD4hep_service>();


    // Algorithm configuration
    auto cfg = GetDefaultConfig();
    app->SetDefaultParameter(param_prefix + ":EtaBins", cfg.m_etaBins, "Eta Bins for ACTS CKF tracking reco");
    app->SetDefaultParameter(param_prefix + ":Chi2CutOff", cfg.m_chi2CutOff, "Chi2 Cut Off for ACTS CKF tracking");
    app->SetDefaultParameter(param_prefix + ":NumMeasurementsCutOff", cfg.m_numMeasurementsCutOff, "Number of measurements Cut Off for ACTS CKF tracking");

    // Initialize algorithm
    m_tracking_algo.applyConfig(cfg);
    m_tracking_algo.init(acts_service->actsGeoProvider(), m_log);
}

void eicrecon::CKFSeededTracking_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {

}

void eicrecon::CKFSeededTracking_factory::Process(const std::shared_ptr<const JEvent> &event) {
    // Now we check that user provided an input names
    std::string input_tag = GetInputTags()[0];

    // Collect all hits
    auto source_linker_result = event->GetSingle<eicrecon::TrackerSourceLinkerResult>(input_tag);

    if(!source_linker_result) {
        m_log->warn("TrackerSourceLinkerResult is null (hasn't been produced?). Skipping tracking for the whole event!");
        return;
    }

    auto track_parameters = event->Get<eicrecon::TrackParameters>("SeededTrackParams");
    eicrecon::TrackParametersContainer acts_track_params;
    for(auto track_params_item: track_parameters) {
        acts_track_params.push_back(*track_params_item);
    }

    // Reading the geometry may take a long time and if the JANA ticker is enabled, it will keep printing
    // while no other output is coming which makes it look like something is wrong. Disable the ticker
    // while parsing and loading the geometry
    auto tickerEnabled = GetApplication()->IsTickerEnabled();
    GetApplication()->SetTicker( false );


    // Convert vector of source links to a sorted in geometry order container used in tracking
    eicrecon::IndexSourceLinkContainer source_links;
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

    try {
        // RUN TRACKING ALGORITHM
        auto trajectories = m_tracking_algo.process(
                source_links,
                *source_linker_result->measurements,
                acts_track_params);

        // Save the result
        Set(trajectories);
    }
    catch(std::exception &e) {
        m_log->warn("Exception in underlying algorithm: {}. Event data will be skipped", e.what());
    }

    // Enable ticker back
    GetApplication()->SetTicker(tickerEnabled);
}
