// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifndef EICRECON_SPDLOGEXTENSIONS_H
#define EICRECON_SPDLOGEXTENSIONS_H

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>
#include <JANA/JException.h>

namespace eicrecon {
    inline spdlog::level::level_enum ParseLogLevel(const std::string &input) {

        // Convert the source string to lower case
        std::string lc_input;              // Lower case input
        lc_input.resize(input.size());
        std::transform(input.begin(), input.end(), lc_input.begin(), ::tolower);

        if(lc_input == "trace" || lc_input == std::to_string(SPDLOG_LEVEL_TRACE)) return spdlog::level::trace;
        if(lc_input == "debug" || lc_input == std::to_string(SPDLOG_LEVEL_DEBUG)) return spdlog::level::debug;
        if(lc_input == "info" || lc_input == std::to_string(SPDLOG_LEVEL_INFO)) return spdlog::level::info;
        if(lc_input == "warn" || lc_input == "warning" || lc_input == std::to_string(SPDLOG_LEVEL_WARN)) return spdlog::level::warn;
        if(lc_input == "err" || lc_input == "error" || lc_input == std::to_string(SPDLOG_LEVEL_ERROR)) return spdlog::level::err;
        if(lc_input == "critical" || lc_input == std::to_string(SPDLOG_LEVEL_CRITICAL)) return spdlog::level::critical;
        if(lc_input == "off" || lc_input == std::to_string(SPDLOG_LEVEL_OFF)) return spdlog::level::off;

        auto err_msg = fmt::format("ParseLogLevel can't parse input string: '{}'", input);
        throw JException(err_msg);
    }
}
#endif //EICRECON_SPDLOGEXTENSIONS_H
