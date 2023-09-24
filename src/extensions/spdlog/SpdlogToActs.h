// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <boost/assign.hpp>
#include <boost/bimap.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

#include <Acts/Utilities/Logger.hpp>
#include <JANA/JException.h>

namespace eicrecon {

using namespace Acts::Logging;

using SpdlogToActsLevel_t = boost::bimap<spdlog::level::level_enum, Acts::Logging::Level>;
static SpdlogToActsLevel_t kSpdlogToActsLevel = boost::assign::list_of<SpdlogToActsLevel_t::relation>
  (spdlog::level::trace, Acts::Logging::VERBOSE)
  (spdlog::level::debug, Acts::Logging::DEBUG)
  (spdlog::level::info, Acts::Logging::INFO)
  (spdlog::level::warn, Acts::Logging::WARNING)
  (spdlog::level::err, Acts::Logging::ERROR)
  (spdlog::level::critical, Acts::Logging::FATAL)
  (spdlog::level::off, Acts::Logging::FATAL);

inline Acts::Logging::Level SpdlogToActsLevel(spdlog::level::level_enum input) {
  try {
    return kSpdlogToActsLevel.right.at(input);
  } catch (...) {
    auto err_msg = fmt::format("SpdlogToActsLevel don't know this log level: '{}'", input);
    throw JException(err_msg);
  }
}

inline spdlog::level::level_enum ActsToSpdlogLevel(Acts::Logging::Level input) {
  try {
    return kSpdlogToActsLevel.left.at(input);
  } catch (...) {
    auto err_msg = fmt::format("ActsToSpdlogLevel don't know this log level: '{}'", input);
    throw JException(err_msg);
  }
}

/// @brief default print policy for debug messages
///
/// This class allows to print debug messages without further modifications to
/// a specified output stream.
class SpdlogPrintPolicy final : public Acts::Logging::OutputPrintPolicy {
  public:
    /// @brief constructor
    ///
    /// @param [in] out pointer to output stream object
    ///
    /// @pre @p out is non-zero
    explicit SpdlogPrintPolicy(std::shared_ptr<spdlog::logger> out) : m_out(out) {}

    /// @brief flush the debug message to the destination stream
    ///
    /// @param [in] lvl   debug level of debug message
    /// @param [in] input text of debug message
    void flush(const Level& lvl, const std::string& input) final {
      m_out->log(ActsToSpdlogLevel(lvl), input);
      if (lvl >= getFailureThreshold()) {
        throw ThresholdFailure(
            "Previous debug message exceeds the "
            "ACTS_LOG_FAILURE_THRESHOLD=" +
            std::string{levelName(getFailureThreshold())} +
            " configuration, bailing out. See "
            "https://acts.readthedocs.io/en/latest/core/"
            "logging.html#logging-thresholds");
      }
    }

  #if 0 // name() and clone() require Acts 22.0.0, https://github.com/acts-project/acts/commit/85b4b292c980f358ed6ba3ce19cdcee361c8ea5b
    /// Fulfill @c OutputPrintPolicy interface. This policy doesn't actually have a
    /// name, so the assumption is that somewhere in the decorator hierarchy,
    /// there is something that returns a name without delegating to a wrappee,
    /// before reaching this overload.
    /// @note This method will throw an exception
    /// @return the name, but it never returns
    const std::string& name() const override {
      throw std::runtime_error{
          "Default print policy doesn't have a name. Is there no named output in "
          "the decorator chain?"};
    };

    /// Make a copy of this print policy with a new name
    /// @param name the new name
    /// @return the copy
    std::unique_ptr<OutputPrintPolicy> clone(
        const std::string& name) const override {
      (void)name;
      return std::make_unique<SpdlogPrintPolicy>(m_out);
    };
  #endif

  private:
    /// pointer to destination output stream
    std::shared_ptr<spdlog::logger> m_out;
};

inline std::unique_ptr<const Acts::Logger> getSpdlogLogger(
    const std::string& name,
    const Acts::Logging::Level& lvl,
    std::shared_ptr<spdlog::logger> log) {

  auto output = std::make_unique<LevelOutputDecorator>(
      std::make_unique<NamedOutputDecorator>(
          std::make_unique<TimedOutputDecorator>(
              std::make_unique<SpdlogPrintPolicy>(log)),
          name));
  auto print = std::make_unique<DefaultFilterPolicy>(lvl);
  return std::make_unique<const Acts::Logger>(std::move(output), std::move(print));
}


} // eicrecon
