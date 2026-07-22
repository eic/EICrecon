// Copyright 2023, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.
// Created by Nathan Brei

#pragma once

#include <JANA/Components/JOmniFactory.h>
#if defined(JANA_VERSION_MAJOR) && defined(JANA_VERSION_MINOR) && defined(JANA_VERSION_PATCH)
#define EICRECON_JANA_IS_243                                                                       \
  (JANA_VERSION_MAJOR == 2 && JANA_VERSION_MINOR == 4 && JANA_VERSION_PATCH == 3)
#else
#define EICRECON_JANA_IS_243 1
#endif
#if !EICRECON_JANA_IS_243
#include <JANA/Components/JPodioOutput.h>
#endif
#include <JANA/Utils/JEventLevel.h>
#include <spdlog/spdlog.h>
#include <spdlog/version.h>
#if SPDLOG_VERSION >= 11400 && (!defined(SPDLOG_NO_TLS) || !SPDLOG_NO_TLS)
#include <spdlog/mdc.h>
#endif

#include "services/log/Log_service.h"

#include <memory>
#include <string>
#include <vector>

namespace eicrecon {

/**
 * This shim class for jana::components::JOmniFactory serves two purposes:
 * - plugging in the spdlog-based logger service we are using in EICrecon
 * - setting the event number in the spdlog Mapped Diagnostic Context (MDC)
 */
template <typename AlgoT, typename ConfigT = jana::components::EmptyConfig>
class JOmniFactory : public jana::components::JOmniFactory<AlgoT, ConfigT> {
private:
  using JANA_JOmniFactory = jana::components::JOmniFactory<AlgoT, ConfigT>;

  // Hide Process(JEvent) in private to prevent accidental use
  using JANA_JOmniFactory::Process;

  /// Current logger
  std::shared_ptr<spdlog::logger> m_logger;

public:
  template <typename T> using ParameterRef = jana::components::JComponent::ParameterRef<T>;

  template <typename T> using Parameter = jana::components::JComponent::Parameter<T>;

  template <typename T> using Service = jana::components::JComponent::Service<T>;

  template <typename T> using Input = jana::components::JHasInputs::Input<T>;

  template <typename T> using VariadicInput = jana::components::JHasInputs::VariadicPodioInput<T>;

  template <typename PodioT, bool IsOptional = false>
  class PodioInput : public jana::components::JHasInputs::PodioInput<PodioT> {
  public:
    explicit PodioInput(JOmniFactory* owner, std::string default_collection_name = "")
        : jana::components::JHasInputs::PodioInput<PodioT>(
              owner, jana::components::JHasInputs::InputOptions{
                         .name        = std::move(default_collection_name),
                         .is_optional = IsOptional,
                     }) {}

    explicit PodioInput(JOmniFactory* owner, jana::components::JHasInputs::InputOptions options)
        : jana::components::JHasInputs::PodioInput<PodioT>(owner, options) {}
  };

  template <typename PodioT, bool IsOptional = false>
  class VariadicPodioInput : public jana::components::JHasInputs::VariadicPodioInput<PodioT> {
  public:
    explicit VariadicPodioInput(JOmniFactory* owner, std::vector<std::string> default_names = {})
        : jana::components::JHasInputs::VariadicPodioInput<PodioT>(
              owner, jana::components::JHasInputs::VariadicInputOptions{
                         .names       = std::move(default_names),
                         .is_optional = IsOptional,
                     }) {}

    explicit VariadicPodioInput(JOmniFactory* owner,
                                jana::components::JHasInputs::VariadicInputOptions options)
        : jana::components::JHasInputs::VariadicPodioInput<PodioT>(owner, options) {}
  };

#if EICRECON_JANA_IS_243
  template <typename PodioT> using PodioOutput = jana::components::JHasOutputs::PodioOutput<PodioT>;
  template <typename PodioT>
  using VariadicPodioOutput = jana::components::JHasOutputs::VariadicPodioOutput<PodioT>;
#else
  template <typename PodioT> using PodioOutput = jana::components::PodioOutput<PodioT>;
  template <typename PodioT>
  using VariadicPodioOutput = jana::components::VariadicPodioOutput<PodioT>;
#endif

  inline void PreInit(std::string tag, JEventLevel level,
                      std::vector<std::string> input_collection_names,
                      std::vector<JEventLevel> input_collection_levels,
                      std::vector<std::vector<std::string>> variadic_input_collection_names,
                      std::vector<JEventLevel> variadic_input_collection_levels,
                      std::vector<std::string> output_collection_names,
                      std::vector<std::vector<std::string>> variadic_output_collection_names) {

    // PreInit using the underlying JANA JOmniFactory
    JANA_JOmniFactory::PreInit(tag, level, input_collection_names, input_collection_levels,
                               variadic_input_collection_names, variadic_input_collection_levels,
                               output_collection_names, variadic_output_collection_names);

    // But obtain our own logger (defines the parameter option)
    m_logger =
        this->GetApplication()->template GetService<Log_service>()->logger(this->GetPrefix());
  }

  virtual void Execute(int32_t run_number, uint64_t event_number) {
#if SPDLOG_VERSION >= 11400 && (!defined(SPDLOG_NO_TLS) || !SPDLOG_NO_TLS)
    spdlog::mdc::put("e", std::to_string(event_number));
#endif
    static_cast<AlgoT*>(this)->Process(run_number, event_number);
  };

  virtual void Process(int32_t /* run_number */, uint64_t /* event_number */) {};

  /// Retrieve reference to already-configured logger
  std::shared_ptr<spdlog::logger>& logger() {
#if EICRECON_JANA_IS_243
    // JANA 2.4.3 does not invoke our PreInit() path, so m_logger may not be initialized.
    if (m_logger == nullptr) {
      m_logger =
          this->GetApplication()->template GetService<Log_service>()->logger(this->GetPrefix());
    }
#endif
    return m_logger;
  }
};

} // namespace eicrecon

#undef EICRECON_JANA_IS_243
