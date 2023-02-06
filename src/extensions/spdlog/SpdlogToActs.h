// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

#include <Acts/Utilities/Logger.hpp>
#include <JANA/JException.h>

namespace eicrecon {
    inline Acts::Logging::Level SpdlogToActsLevel(spdlog::level::level_enum input) {

        // Convert the source string to lower case
        switch (input) {
            case spdlog::level::trace:
                return Acts::Logging::VERBOSE;
            case spdlog::level::debug:
                return Acts::Logging::DEBUG;
            case spdlog::level::info:
                return Acts::Logging::INFO;
            case spdlog::level::warn:
                return Acts::Logging::WARNING;
            case spdlog::level::err:
                return Acts::Logging::ERROR;
            case spdlog::level::critical:
                return Acts::Logging::FATAL;
            case spdlog::level::off:
                return Acts::Logging::FATAL;
            case spdlog::level::n_levels:
                [[fallthrough]];
            default:
                break;
        }

        auto err_msg = fmt::format("SpdlogToActsLevel don't know this log level: '{}'", input);
        throw JException(err_msg);
    }
}
