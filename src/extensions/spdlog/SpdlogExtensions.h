// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

namespace eicrecon {
inline spdlog::level::level_enum ParseLogLevel(const std::string& input) {

  // Convert the source string to lower case
  std::string lc_input; // Lower case input
  lc_input.resize(input.size());
  std::transform(input.begin(), input.end(), lc_input.begin(), ::tolower);

  if (lc_input == "trace" || lc_input == std::to_string(SPDLOG_LEVEL_TRACE))
    return spdlog::level::trace;
  if (lc_input == "debug" || lc_input == std::to_string(SPDLOG_LEVEL_DEBUG))
    return spdlog::level::debug;
  if (lc_input == "info" || lc_input == std::to_string(SPDLOG_LEVEL_INFO))
    return spdlog::level::info;
  if (lc_input == "warn" || lc_input == "warning" || lc_input == std::to_string(SPDLOG_LEVEL_WARN))
    return spdlog::level::warn;
  if (lc_input == "err" || lc_input == "error" || lc_input == std::to_string(SPDLOG_LEVEL_ERROR))
    return spdlog::level::err;
  if (lc_input == "critical" || lc_input == std::to_string(SPDLOG_LEVEL_CRITICAL))
    return spdlog::level::critical;
  if (lc_input == "off" || lc_input == std::to_string(SPDLOG_LEVEL_OFF))
    return spdlog::level::off;

  auto err_msg = fmt::format("ParseLogLevel can't parse input string: '{}'", input);
  throw std::runtime_error(err_msg);
}

inline std::string LogLevelToString(spdlog::level::level_enum input) {

  // Convert the source string to lower case
  switch (input) {
  case spdlog::level::trace:
    return "trace";
  case spdlog::level::debug:
    return "debug";
  case spdlog::level::info:
    return "info";
  case spdlog::level::warn:
    return "warn";
  case spdlog::level::err:
    return "error";
  case spdlog::level::critical:
    return "critical";
  case spdlog::level::off:
    return "off";
  case spdlog::level::n_levels:
    [[fallthrough]];
  default:
    break;
  }

  auto err_msg =
      fmt::format("LogLevelToString doesn't know this log level: '{}'", fmt::underlying(input));
  throw std::runtime_error(err_msg);
}
} // namespace eicrecon
