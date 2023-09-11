// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include "CKFTracking_factory.h"

#include <JANA/JEvent.h>

#include <edm4eic/TrackParametersCollection.h>

#include "algorithms/tracking/TrackerSourceLinkerResult.h"
#include "extensions/spdlog/SpdlogExtensions.h"
#include "services/geometry/acts/ACTSGeo_service.h"
#include "services/geometry/dd4hep/JDD4hep_service.h"

void eicrecon::CKFTracking_factory::Init() {
    auto app = GetApplication();

    // This prefix will be used for parameters
    std::string plugin_name = GetPluginName();
    std::string param_prefix = plugin_name+ ":" + GetTag();

    // Initialize logger
    InitLogger(app, param_prefix, "info");

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

void eicrecon::CKFTracking_factory::Process(const std::shared_ptr<const JEvent> &event) {
    // Collect all inputs
    auto seed_track_parameters = static_cast<const edm4eic::TrackParametersCollection*>(event->GetCollectionBase(GetInputTags()[0]));
    auto source_linker_result = event->GetSingle<eicrecon::TrackerSourceLinkerResult>(GetInputTags()[1]);

    if(!source_linker_result) {
        m_log->warn("TrackerSourceLinkerResult is null (hasn't been produced?). Skipping tracking for the whole event!");
        return;
    }

    // Convert vector of source links to a sorted in geometry order container used in tracking
    ActsExamples::IndexSourceLinkContainer source_links;
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
        auto [trajectories, track_parameters, acts_trajectories] = m_tracking_algo.process(
                source_links,
                *source_linker_result->measurements,
                *seed_track_parameters);

        // Save the result
        SetCollection<edm4eic::Trajectory>(GetOutputTags()[0], std::move(trajectories));
        SetCollection<edm4eic::TrackParameters>(GetOutputTags()[1], std::move(track_parameters));
        SetData<ActsExamples::Trajectories>(GetOutputTags()[2], std::move(acts_trajectories));
    }
    catch(std::exception &e) {
        throw JException(e.what());
    }
}
