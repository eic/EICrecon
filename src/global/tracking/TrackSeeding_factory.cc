// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//


#include <Acts/Propagator/Navigator.hpp>
#include "TrackSeeding_factory.h"
#include "extensions/spdlog/SpdlogExtensions.h"
#include <JANA/JEvent.h>
#include "services/geometry/acts/ACTSGeo_service.h"
#include "services/log/Log_service.h"
#include "extensions/string/StringHelpers.h"
#include <services/geometry/dd4hep/JDD4hep_service.h>

void eicrecon::TrackSeeding_factory::Init() {
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
    m_seeding_algo.applyConfig(cfg);
    m_seeding_algo.init(acts_service->actsGeoProvider(), m_log);
}

void eicrecon::TrackSeeding_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {

}

void eicrecon::TrackSeeding_factory::Process(const std::shared_ptr<const JEvent> &event) {
    // Collect all hits
    std::vector<const edm4eic::TrackerHit*> total_hits;

    for(auto input_tag: GetInputTags()) {
        auto hits = event->Get<edm4eic::TrackerHit>(input_tag);
        for (const auto hit : hits) {
            total_hits.push_back(hit);
        }
    }

    m_log->debug("Process method");

    try {
        auto result = m_seeding_algo.produce(total_hits);
        Set(result);    // Set() - is what factory produced
    }
    catch(std::exception &e) {
        m_log->warn("Exception in underlying algorithm: {}. Event data will be skipped", e.what());
    }
}
