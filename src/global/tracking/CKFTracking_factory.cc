// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include "CKFTracking_factory.h"

#include <Acts/Propagator/Navigator.hpp>
#include <Acts/Surfaces/PerigeeSurface.hpp>
#include <edm4eic/TrackParametersCollection.h>
#include <JANA/JEvent.h>

#include "extensions/spdlog/SpdlogExtensions.h"
#include "extensions/string/StringHelpers.h"
#include "services/geometry/acts/ACTSGeo_service.h"
#include "services/geometry/dd4hep/JDD4hep_service.h"
#include "services/log/Log_service.h"

void eicrecon::CKFTracking_factory::Init() {
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

void eicrecon::CKFTracking_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {

}

void eicrecon::CKFTracking_factory::Process(const std::shared_ptr<const JEvent> &event) {
    // Collect all inputs
    auto track_parameters = static_cast<const edm4eic::TrackParametersCollection*>(event->GetCollectionBase(GetInputTags()[0]));
    auto source_linker_result = event->GetSingle<eicrecon::TrackerSourceLinkerResult>(GetInputTags()[1]);

    if(!source_linker_result) {
        m_log->warn("TrackerSourceLinkerResult is null (hasn't been produced?). Skipping tracking for the whole event!");
        return;
    }

    eicrecon::TrackParametersContainer acts_track_params;
    for (const auto& track_parameter: *track_parameters) {

        Acts::BoundVector params;
        params(Acts::eBoundLoc0)   = track_parameter.getLoc().a * Acts::UnitConstants::mm;  // cylinder radius
        params(Acts::eBoundLoc1)   = track_parameter.getLoc().b * Acts::UnitConstants::mm;  // cylinder length
        params(Acts::eBoundTheta)  = track_parameter.getTheta();
        params(Acts::eBoundPhi)    = track_parameter.getPhi();
        params(Acts::eBoundQOverP) = track_parameter.getQOverP() / Acts::UnitConstants::GeV;
        params(Acts::eBoundTime)   = track_parameter.getTime() * Acts::UnitConstants::ns;

        double charge = track_parameter.getCharge();

        Acts::BoundSymMatrix cov                    = Acts::BoundSymMatrix::Zero();
        cov(Acts::eBoundLoc0, Acts::eBoundLoc0)     = std::pow( track_parameter.getLocError().xx ,2)*Acts::UnitConstants::mm*Acts::UnitConstants::mm;
        cov(Acts::eBoundLoc1, Acts::eBoundLoc1)     = std::pow( track_parameter.getLocError().yy,2)*Acts::UnitConstants::mm*Acts::UnitConstants::mm;
        cov(Acts::eBoundTheta, Acts::eBoundTheta)   = std::pow( track_parameter.getMomentumError().xx,2);
        cov(Acts::eBoundPhi, Acts::eBoundPhi)       = std::pow( track_parameter.getMomentumError().yy,2);
        cov(Acts::eBoundQOverP, Acts::eBoundQOverP) = std::pow( track_parameter.getMomentumError().zz,2) / (Acts::UnitConstants::GeV*Acts::UnitConstants::GeV);
        cov(Acts::eBoundTime, Acts::eBoundTime)     = std::pow( track_parameter.getTimeError(),2)*Acts::UnitConstants::ns*Acts::UnitConstants::ns;

        // Construct a perigee surface as the target surface
        auto pSurface = Acts::Surface::makeShared<const Acts::PerigeeSurface>(Acts::Vector3(0,0,0));

        // Create parameters
        acts_track_params.emplace_back(pSurface, params, charge, cov);
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
        throw JException(e.what());
    }

    // Enable ticker back
    GetApplication()->SetTicker(tickerEnabled);
}
