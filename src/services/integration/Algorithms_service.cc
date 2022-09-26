// Created by Dmitry Romanov, Sylvester Joosten
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include "Algorithms_service.h"
#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <services/log/Log_service.h>

#include <spdlog/spdlog.h>

#include <algorithms/geo.h>
#include <algorithms/logger.h>
#include <algorithms/service.h>


algorithms::LogLevel toAlgoLogLevel(spdlog::level::level_enum spdlog_level) {
    if(spdlog_level >= spdlog::level::off) return algorithms::LogLevel::kCritical;
    return static_cast<algorithms::LogLevel>(spdlog_level);
}

void eicrecon::Algorithms_service::acquire_services(JServiceLocator *srv_locator) {
    auto eicrecon_dd4hep_svc = srv_locator->get<JDD4hep_service>();
    auto log_service = srv_locator->get<Log_service>();
    auto eicrecon_log = log_service->logger("AlgoServiceSvc");


    auto& serviceSvc = algorithms::ServiceSvc::instance();
    eicrecon_log->info("ServiceSvc declared {} services", serviceSvc.services().size());
    // Always initialize the LogSvc first to ensure proper logging for the others
    {
        auto& algo_log = algorithms::LogSvc::instance();
        auto level = toAlgoLogLevel(eicrecon_log->level());
        eicrecon_log->info("Setting up algorithms::LogSvc with default level {}", algorithms::logLevelName(level));

        algo_log.defaultLevel(level);
        algo_log.action(
                [&eicrecon_log](const algorithms::LogLevel l, std::string_view caller, std::string_view msg) {
                    const std::string text = fmt::format("[{}] {}", caller, msg);
                    if (l == algorithms::LogLevel::kCritical) {
                        eicrecon_log->critical(text);
                    } else if (l == algorithms::LogLevel::kError) {
                        eicrecon_log->error(text);
                    } else if (l == algorithms::LogLevel::kWarning) {
                        eicrecon_log->warn(text);
                    } else if (l == algorithms::LogLevel::kInfo) {
                        eicrecon_log->info(text);
                    } else if (l == algorithms::LogLevel::kDebug) {
                        eicrecon_log->debug(text);
                    } else if (l == algorithms::LogLevel::kTrace) {
                        eicrecon_log->trace(text);
                    }
                });

    }
    // loop over all remaining services and handle each properly
    // Note: this code is kind of dangerous, as getting the types wrong will lead to
    // undefined runtime behavior.
    for (auto [name, svc] : serviceSvc.services()) {
        if (name == algorithms::LogSvc::kName) {
            ; // Logsvc already initialized, do nothing
        } else if (name == algorithms::GeoSvc::kName) {

            eicrecon_log->info("Setting up algorithms::GeoSvc");
            auto* geo = static_cast<algorithms::GeoSvc*>(svc);
            geo->init(eicrecon_dd4hep_svc->detector());
        } else {
            auto message = fmt::format("Unknown service with name='{}' encountered, please implement the necessary framework hooks", name);
            eicrecon_log->critical(message);
            throw JException(message);
        }
    }

    eicrecon_log->info("AlgoServiceSvc initialized successfully");
}
