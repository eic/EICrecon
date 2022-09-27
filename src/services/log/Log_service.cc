// Copyright 2022, Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//
#include <spdlog/sinks/stdout_color_sinks.h>
#include "Log_service.h"
#include <JANA/JException.h>


Log_service::Log_service(JApplication *app) {
    // Here one could add centralized documentation for spdlog::default_logger()
    // All subsequent loggers are cloned from the spdlog::default_logger()

}


std::shared_ptr<spdlog::logger> Log_service::logger(const std::string &name) {
    try {
        std::lock_guard<std::recursive_mutex> locker(m_lock);
        // Try to get existing logger
        auto logger = spdlog::get(name);
        if(!logger) {
            // or create a new one with current configuration
            logger = spdlog::default_logger()->clone(name);
        }
        return logger;
    }
    catch(const std::exception & exception) {
        throw JException(exception.what());
    }
}



