// Copyright 2023, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.
// Created by Nathan Brei

#pragma once

/**
 * This shim class for jana::components::JOmniFactory serves two purposes:
 * - plugging in the spdlog-based logger service we are using in EICrecon
 * - setting the event number in the spdlog Mapped Diagnostic Context (MDC)
 */

#include <JANA/CLI/JVersion.h>
#include <JANA/Components/JOmniFactory.h>
#include <JANA/JMultifactory.h>
#include <JANA/JEvent.h>
#include <spdlog/spdlog.h>
#include <spdlog/version.h>
#if SPDLOG_VERSION >= 11400 && (!defined(SPDLOG_NO_TLS) || !SPDLOG_NO_TLS)
#include <spdlog/mdc.h>
#endif

#include "services/io/podio/datamodel_glue.h"
#include "services/log/Log_service.h"

#include <string>
#include <vector>

namespace eicrecon {

template <typename AlgoT, typename ConfigT = jana::components::EmptyConfig>
class JOmniFactory : public jana::components::JOmniFactory<AlgoT, ConfigT> {
private:
  using JANA_JOmniFactory = jana::components::JOmniFactory<AlgoT, ConfigT>;

  // Hide Process(JEvent) in private to prevent accidental use
  using JANA_JOmniFactory::Process;

  /// Current logger
  std::shared_ptr<spdlog::logger> m_logger;

public:
  inline void PreInit(std::string tag, JEventLevel level,
                      std::vector<std::string> input_collection_names,
                      std::vector<JEventLevel> input_collection_levels,
                      std::vector<std::string> output_collection_names) {

    // PreInit using the underlying JANA JOmniFactory
    JANA_JOmniFactory::PreInit(tag, level, input_collection_names, input_collection_levels,
                               output_collection_names);

    // But obtain our own logger (defines the parameter option)
    m_logger =
        this->GetApplication()->template GetService<Log_service>()->logger(this->GetPrefix());
  }

  virtual void ChangeRun(int32_t /* run_number */) override {};

  virtual void Execute(int32_t run_number, uint64_t event_number) {
#if SPDLOG_VERSION >= 11400 && (!defined(SPDLOG_NO_TLS) || !SPDLOG_NO_TLS)
    spdlog::mdc::put("e", std::to_string(event_number));
#endif
    static_cast<AlgoT*>(this)->Process(run_number, event_number);
  };

  virtual void Process(int32_t /* run_number */, uint64_t /* event_number */) {};

  /// Retrieve reference to already-configured logger
  std::shared_ptr<spdlog::logger>& logger() { return m_logger; }
};

} // namespace eicrecon
