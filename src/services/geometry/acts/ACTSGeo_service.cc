// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include "ACTSGeo_service.h"

#include "services/geometry/dd4hep/DD4hep_service.h"
#include "services/log/Log_service.h"

#include <JANA/JException.h>
#include "extensions/spdlog/SpdlogExtensions.h"

// Virtual destructor implementation to pin vtable and typeinfo to this
// translation unit
ACTSGeo_service::~ACTSGeo_service() {};

//----------------------------------------------------------------
// detector
//
/// Return pointer to the dd4hep::Detector object.
/// Call Initialize if needed.
//----------------------------------------------------------------
std::shared_ptr<const ActsGeometryProvider> ACTSGeo_service::actsGeoProvider() {

    try{
        std::call_once(m_init_flag, [this](){
            // Assemble everything on the first call

            if(!m_dd4hepGeo) {
                throw JException("ACTSGeo_service m_dd4hepGeo==null which should never be!");
            }

            // Get material map from user parameter
            std::string material_map_file;
            try {
              material_map_file = m_dd4hepGeo->constant<std::string>("material-map");
            } catch (const std::runtime_error& e) {
              material_map_file = "calibrations/materials-map.cbor";
            }
            m_app->SetDefaultParameter("acts:MaterialMap", material_map_file, "JSON/CBOR material map file path");

            // Reading the geometry may take a long time and if the JANA ticker is enabled, it will keep printing
            // while no other output is coming which makes it look like something is wrong. Disable the ticker
            // while parsing and loading the geometry
            auto tickerEnabled = m_app->IsTickerEnabled();
            m_app->SetTicker(false);

            // Initialize m_acts_provider
            m_acts_provider = std::make_shared<ActsGeometryProvider>();
            m_acts_provider->initialize(m_dd4hepGeo, material_map_file, m_log, m_init_log);

            // Enable ticker back
            m_app->SetTicker(tickerEnabled);
        });
    }
    catch (std::exception &ex) {
        throw JException(ex.what());
    }

    return m_acts_provider;
}



void ACTSGeo_service::acquire_services(JServiceLocator * srv_locator) {

    auto log_service = srv_locator->get<Log_service>();

    // ACTS general log level:
    m_log = log_service->logger("acts");
    std::string log_level_str = log_service->getDefaultLevelStr();
    m_app->SetDefaultParameter("acts:LogLevel", log_level_str, "log_level: trace, debug, info, warn, error, critical, off");
    m_log->set_level(eicrecon::ParseLogLevel(log_level_str));
    m_log->info("Acts GENERAL log level is set to {} ({})", log_level_str, fmt::underlying(m_log->level()));

    // ACTS init log level (geometry conversion):
    m_init_log = log_service->logger("acts_init");
    std::string init_log_level_str = eicrecon::LogLevelToString(m_log->level());  // set general acts log level, if not given by user
    m_app->SetDefaultParameter("acts:InitLogLevel", init_log_level_str, "log_level: trace, debug, info, warn, error, critical, off");
    m_init_log->set_level(eicrecon::ParseLogLevel(init_log_level_str));
    m_init_log->info("Acts INIT log level is set to {} ({})", log_level_str, fmt::underlying(m_init_log->level()));

    // DD4Hep geometry
    auto dd4hep_service = srv_locator->get<DD4hep_service>();
    m_dd4hepGeo = dd4hep_service->detector();
}
