// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <JANA/JEvent.h>

#include <edm4eic/TrackParametersCollection.h>

#include "TrackSeeding_factory.h"
#include "extensions/spdlog/SpdlogExtensions.h"
#include "services/geometry/acts/ACTSGeo_service.h"

void eicrecon::TrackSeeding_factory::Init() {
    auto app = GetApplication();

    // This prefix will be used for parameters
    std::string plugin_name = GetPluginName();
    std::string param_prefix = plugin_name+ ":" + GetTag();

    // Initialize input tags
    InitDataTags(param_prefix);

    // Initialize logger
    InitLogger(app, param_prefix, "info");

    // Get ACTS context from ACTSGeo service
    auto acts_service = app->GetService<ACTSGeo_service>();

    // Algorithm configuration
    auto cfg = GetDefaultConfig();

    app->SetDefaultParameter(param_prefix + ":rMax", cfg.m_rMax, "max measurement radius for Acts::OrthogonalSeedFinder");
    app->SetDefaultParameter(param_prefix + ":rMin", cfg.m_rMin, "min measurement radius for Acts::OrthogonalSeedFinder");
    app->SetDefaultParameter(param_prefix + ":deltaRMinTopSP", cfg.m_deltaRMinTopSP, "min distance in r between middle and top space point in one seed for Acts::OrthogonalSeedFinder");
    app->SetDefaultParameter(param_prefix + ":deltaRMaxTopSP", cfg.m_deltaRMaxTopSP, "max distance in r between middle and top space point in one seed for Acts::OrthogonalSeedFinder");
    app->SetDefaultParameter(param_prefix + ":deltaRMinBottomSP", cfg.m_deltaRMinBottomSP, "min distance in r between bottom and middle space point in one seed for Acts::OrthogonalSeedFinder");
    app->SetDefaultParameter(param_prefix + ":deltaRMaxBottomSP", cfg.m_deltaRMaxBottomSP, "max distance in r between bottom and middle space point in one seed for Acts::OrthogonalSeedFinder");
    app->SetDefaultParameter(param_prefix + ":collisionRegionMin", cfg.m_collisionRegionMin, "min location in z for collision region for Acts::OrthogonalSeedFinder");
    app->SetDefaultParameter(param_prefix + ":collisionRegionMax", cfg.m_collisionRegionMax, "max location in z for collision region for Acts::OrthogonalSeedFinder");
    app->SetDefaultParameter(param_prefix + ":zMax", cfg.m_zMax, "Max z location for measurements for Acts::OrthogonalSeedFinder");
    app->SetDefaultParameter(param_prefix + ":zMin", cfg.m_zMin, "Min z location for measurements for Acts::OrthogonalSeedFinder");
    app->SetDefaultParameter(param_prefix + ":maxSeedsPerSpM", cfg.m_maxSeedsPerSpM, "Maximum number of seeds one space point can be the middle of for Acts::OrthogonalSeedFinder");
    app->SetDefaultParameter(param_prefix + ":cotThetaMax", cfg.m_cotThetaMax, "cot of maximum theta angle for Acts::OrthogonalSeedFinder");
    app->SetDefaultParameter(param_prefix + ":sigmaScattering", cfg.m_sigmaScattering, "number of sigmas of scattering angle to consider for Acts::OrthogonalSeedFinder");
    app->SetDefaultParameter(param_prefix + ":radLengthPerSeed", cfg.m_radLengthPerSeed, "Approximate number of radiation lengths one seed traverses for Acts::OrthogonalSeedFinder");
    app->SetDefaultParameter(param_prefix + ":minPt", cfg.m_minPt, "Minimum pT to search for for Acts::OrthogonalSeedFinder");
    app->SetDefaultParameter(param_prefix + ":bFieldInZ", cfg.m_bFieldInZ, "Value of B Field to use in kiloTesla for Acts::OrthogonalSeedFinder");
    app->SetDefaultParameter(param_prefix + ":beamPosX", cfg.m_beamPosX, "Beam position in x for Acts::OrthogonalSeedFinder");
    app->SetDefaultParameter(param_prefix + ":beamPosY", cfg.m_beamPosY, "Beam position in y for Acts::OrthogonalSeedFinder");
    app->SetDefaultParameter(param_prefix + ":impactMax", cfg.m_impactMax, "maximum impact parameter allowed for seeds for Acts::OrthogonalSeedFinder. rMin should be larger than impactMax.");
    app->SetDefaultParameter(param_prefix + ":rMinMiddle", cfg.m_rMinMiddle, "min radius for middle space point for Acts::OrthogonalSeedFinder");
    app->SetDefaultParameter(param_prefix + ":rMaxMiddle", cfg.m_rMaxMiddle, "max radius for middle space point for Acts::OrthogonalSeedFinder");

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
        throw JException(e.what());
    }
}
