// Copyright 2022, Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//
#include <spdlog/sinks/stdout_color_sinks.h>
#include "Spdlog_service.h"
#include <JANA/JException.h>

void Spdlog_service::Initialize() {
    /* all global configuration of spdlog should be here */
}



std::shared_ptr<spdlog::logger> Spdlog_service::getLogger(const std::string &name) {
    try {
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


