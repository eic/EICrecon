// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include "CKFTracking_factory.h"

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>
#include <JANA/JException.h>
#include <edm4eic/Measurement2DCollection.h>
#include <edm4eic/TrackParametersCollection.h>
#include <spdlog/logger.h>
#include <exception>

#include "CKFTracking.h"
#include "CKFTrackingConfig.h"
#include "datamodel_glue.h"
#include "services/geometry/acts/ACTSGeo_service.h"
#include "services/geometry/dd4hep/DD4hep_service.h"

void eicrecon::CKFTracking_factory::Init() {
    auto *app = GetApplication();

    // This prefix will be used for parameters
    std::string plugin_name = GetPluginName();
    std::string param_prefix = plugin_name+ ":" + GetTag();

    // Initialize logger
    InitLogger(app, param_prefix, "info");

    // Get ACTS context from ACTSGeo service
    auto acts_service   = app->GetService<ACTSGeo_service>();
    auto dd4hep_service = app->GetService<DD4hep_service>();


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
    auto seed_track_parameters = static_cast<const edm4eic::TrackParametersCollection*>(event->GetCollectionBase(GetInputTags()[0])); // 0 and 1 as ordered in tracking.cc
    auto meas2Ds = static_cast<const edm4eic::Measurement2DCollection*>(event->GetCollectionBase(GetInputTags()[1]));

    if(!meas2Ds) {
        m_log->warn("TrackerMeasurementFromHits is null (hasn't been produced?). Skipping tracking for the whole event!");
        return;
    }

    try {
        // RUN TRACKING ALGORITHM
        auto [trajectories, track_parameters, acts_trajectories] = m_tracking_algo.process(
                *meas2Ds,
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
