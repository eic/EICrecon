// Copyright 2022, Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//
#include "Log_service.h"

#include <JANA/JApplication.h>
#include <JANA/JException.h>
#include <spdlog/formatter.h>
#include <spdlog/pattern_formatter.h>
#include <spdlog/spdlog.h>
#include <spdlog/version.h>
#if SPDLOG_VERSION >= 11400 && (!defined(SPDLOG_NO_TLS) || !SPDLOG_NO_TLS)
#include <spdlog/mdc.h>
#endif
#include <ctime>
#include <exception>
#include <map>
#include <string_view>
#include <utility>

#include "extensions/spdlog/SpdlogExtensions.h"

#if SPDLOG_VERSION >= 11400 && (!defined(SPDLOG_NO_TLS) || !SPDLOG_NO_TLS)

// Define our own MDC formatter since the one in libspdlog.so does not
// function correctly under some compilers
class mdc_formatter_flag : public spdlog::custom_flag_formatter {
public:
  void format(const spdlog::details::log_msg& /*msg*/, const std::tm& /*tm_time*/,
              spdlog::memory_buf_t& dest) override {
    auto& mdc_map = spdlog::mdc::get_context();
    if (mdc_map.empty()) {
      return;
    }
    format_mdc(mdc_map, dest);
  }

  static void format_mdc(const spdlog::mdc::mdc_map_t& mdc_map, spdlog::memory_buf_t& dest) {
    auto last_element = --mdc_map.end();
    for (auto it = mdc_map.begin(); it != mdc_map.end(); ++it) {
      const auto& pair  = *it;
      const auto& key   = pair.first;
      const auto& value = pair.second;
      dest.append(std::string_view{key});
      dest.append(std::string_view{":"});
      dest.append(std::string_view{value});
      if (it != last_element) {
        dest.append(std::string_view{" "});
      }
    }
  }

  std::unique_ptr<custom_flag_formatter> clone() const override {
    return spdlog::details::make_unique<mdc_formatter_flag>();
  }
};

#endif

Log_service::Log_service(JApplication* app) : m_application(app), m_log_level_str("info") {
  // Here one could add centralized documentation for spdlog::default_logger()
  // All subsequent loggers are cloned from the spdlog::default_logger()

  m_application->SetDefaultParameter("eicrecon:LogLevel", m_log_level_str,
                                     "log_level: trace, debug, info, warn, error, critical, off");
  spdlog::default_logger()->set_level(eicrecon::ParseLogLevel(m_log_level_str));

  auto formatter = std::make_unique<spdlog::pattern_formatter>();
#if SPDLOG_VERSION >= 11400 && (!defined(SPDLOG_NO_TLS) || !SPDLOG_NO_TLS)
  formatter->add_flag<mdc_formatter_flag>('&').set_pattern("[%&] [%n] [%^%l%$] %v");
#else
  formatter->set_pattern("[%n] [%^%l%$] %v");
#endif
  m_application->SetDefaultParameter("eicrecon:LogFormat", m_log_level_str,
                                     "spdlog pattern string");
  spdlog::set_formatter(std::move(formatter));
}

// Virtual destructor implementation to pin vtable and typeinfo to this
// translation unit
Log_service::~Log_service() = default;

std::shared_ptr<spdlog::logger> Log_service::logger(const std::string& name,
                                                    const std::optional<level> default_level) {

  try {
    std::lock_guard<std::recursive_mutex> locker(m_lock);

    // Try to get existing logger
    auto logger = spdlog::get(name);
    if (!logger) {
      // or create a new one with current configuration
      logger = spdlog::default_logger()->clone(name);

      // Set log level for this named logger allowing user to specify as config. parameter
      // e.g. EcalEndcapPRecHits:LogLevel
      std::string log_level_str =
          default_level ? eicrecon::LogLevelToString(default_level.value()) : m_log_level_str;
      m_application->SetDefaultParameter(name + ":LogLevel", log_level_str,
                                         "log_level for " + name +
                                             ": trace, debug, info, warn, error, critical, off");
      logger->set_level(eicrecon::ParseLogLevel(log_level_str));
    }
    return logger;
  } catch (const std::exception& exception) {
    throw JException(exception.what());
  }
}

Log_service::level Log_service::getDefaultLevel() { return spdlog::default_logger()->level(); }

std::string Log_service::getDefaultLevelStr() {
  return eicrecon::LogLevelToString(getDefaultLevel());
}
