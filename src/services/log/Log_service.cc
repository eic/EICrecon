// Copyright 2022, Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//
#include <spdlog/sinks/stdout_color_sinks.h>
#include "Log_service.h"
#include <JANA/JException.h>
#include <extensions/spdlog/SpdlogExtensions.h>


Log_service::Log_service(JApplication *app) {
    // Here one could add centralized documentation for spdlog::default_logger()
    // All subsequent loggers are cloned from the spdlog::default_logger()
    m_application = app;

    m_log_level_str = "info";
    m_application->SetDefaultParameter("eicrecon:LogLevel", m_log_level_str, "log_level: trace, debug, info, warn, error, critical, off");
    spdlog::default_logger()->set_level(eicrecon::ParseLogLevel(m_log_level_str));

//    // Get default loggers
//    static std::once_flag on_first_execution;
//    std::call_once(on_first_execution, [this](){
//        m_log_level_str = "info";
//        m_application->SetDefaultParameter("eicrecon:LogLevel", m_log_level_str, "log_level: trace, debug, info, warn, error, critical, off");
//        spdlog::default_logger()->set_level(eicrecon::ParseLogLevel(m_log_level_str));
//    });

}


std::shared_ptr<spdlog::logger> Log_service::logger(const std::string &name) {

    try {
        std::lock_guard<std::recursive_mutex> locker(m_lock);

        // Try to get existing logger
        auto logger = spdlog::get(name);
        if(!logger) {
            // or create a new one with current configuration
            logger = spdlog::default_logger()->clone(name);

            // Set log level for this named logger allowing user to specify as config. parameter
            // e.g. EcalEndcapPRecHits:LogLevel
            std::string log_level_str = m_log_level_str;
            m_application->SetDefaultParameter(name+":LogLevel", log_level_str, "log_level for "+name+": trace, debug, info, warn, error, critical, off");
            logger->set_level(eicrecon::ParseLogLevel(log_level_str));
        }
        return logger;
    }
    catch(const std::exception & exception) {
        throw JException(exception.what());
    }
}



