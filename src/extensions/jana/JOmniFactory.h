// Copyright 2023, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.
// Created by Nathan Brei

#pragma once

/**
 * Omnifactories are a lightweight layer connecting JANA to generic algorithms
 * It is assumed multiple input data (controlled by input tags)
 * which might be changed by user parameters.
 */

#include <JANA/JEvent.h>
#include <JANA/JMultifactory.h>
#include <spdlog/spdlog.h>
#include <spdlog/version.h>
#if SPDLOG_VERSION >= 11400 && (!defined(SPDLOG_NO_TLS) || !SPDLOG_NO_TLS)
#include <spdlog/mdc.h>
#endif

#include "services/io/podio/datamodel_glue_compat.h"
#include "services/log/Log_service.h"

#include <string>
#include <vector>

struct EmptyConfig {};

template <typename AlgoT, typename ConfigT = EmptyConfig>
class JOmniFactory : public JMultifactory {
public:
  /// ========================
  /// Handle input collections
  /// ========================

  struct InputBase {
    std::string type_name;
    std::vector<std::string> collection_names;
    bool is_variadic = false;

    virtual void GetCollection(const JEvent& event) = 0;
  };

  template <typename T, bool IsOptional = false> class Input : public InputBase {

    std::vector<const T*> m_data;

  public:
    Input(JOmniFactory* owner, std::string default_tag = "") {
      owner->RegisterInput(this);
      this->collection_names.push_back(default_tag);
      this->type_name = JTypeInfo::demangle<T>();
    }

    const std::vector<const T*>& operator()() { return m_data; }

  private:
    friend class JOmniFactory;

    void GetCollection(const JEvent& event) {
      try {
        m_data = event.Get<T>(this->collection_names[0], !IsOptional);
      } catch (const JException& e) {
        if constexpr (!IsOptional) {
          throw JException("JOmniFactory: Failed to get collection %s: %s",
                           this->collection_names[0].c_str(), e.what());
        }
      }
    }
  };

  template <typename PodioT, bool IsOptional = false> class PodioInput : public InputBase {

    const typename PodioTypeMap<PodioT>::collection_t* m_data;

  public:
    PodioInput(JOmniFactory* owner, std::string default_collection_name = "") {
      owner->RegisterInput(this);
      this->collection_names.push_back(default_collection_name);
      this->type_name = JTypeInfo::demangle<PodioT>();
    }

    const typename PodioTypeMap<PodioT>::collection_t* operator()() { return m_data; }

  private:
    friend class JOmniFactory;

    void GetCollection(const JEvent& event) {
      try {
        m_data = event.GetCollection<PodioT>(this->collection_names[0], !IsOptional);
      } catch (const JException& e) {
        if constexpr (!IsOptional) {
          throw JException("JOmniFactory: Failed to get collection %s: %s",
                           this->collection_names[0].c_str(), e.what());
        }
      }
    }
  };

  template <typename PodioT, bool IsOptional = false> class VariadicPodioInput : public InputBase {

    std::vector<const typename PodioTypeMap<PodioT>::collection_t*> m_data;

  public:
    VariadicPodioInput(JOmniFactory* owner, std::vector<std::string> default_names = {}) {
      owner->RegisterInput(this);
      this->collection_names = default_names;
      this->type_name        = JTypeInfo::demangle<PodioT>();
      this->is_variadic      = true;
    }

    const std::vector<const typename PodioTypeMap<PodioT>::collection_t*> operator()() {
      return m_data;
    }

  private:
    friend class JOmniFactory;

    void GetCollection(const JEvent& event) {
      m_data.clear();
      for (auto& coll_name : this->collection_names) {
        try {
          m_data.push_back(event.GetCollection<PodioT>(coll_name, !IsOptional));
        } catch (const JException& e) {
          if constexpr (!IsOptional) {
            throw JException("JOmniFactory: Failed to get collection %s: %s", coll_name.c_str(),
                             e.what());
          }
        }
      }
    }
  };

  void RegisterInput(InputBase* input) { m_inputs.push_back(input); }

  /// =========================
  /// Handle output collections
  /// =========================

  struct OutputBase {
    std::string type_name;
    std::vector<std::string> collection_names;
    bool is_variadic = false;

    virtual void CreateHelperFactory(JOmniFactory& fac) = 0;
    virtual void SetCollection(JOmniFactory& fac)       = 0;
    virtual void Reset()                                = 0;
  };

  template <typename T> class Output : public OutputBase {
    std::vector<T*> m_data;

  public:
    Output(JOmniFactory* owner, std::string default_tag_name = "") {
      owner->RegisterOutput(this);
      this->collection_names.push_back(default_tag_name);
      this->type_name = JTypeInfo::demangle<T>();
    }

    std::vector<T*>& operator()() { return m_data; }

  private:
    friend class JOmniFactory;

    void CreateHelperFactory(JOmniFactory& fac) override {
      fac.DeclareOutput<T>(this->collection_names[0]);
    }

    void SetCollection(JOmniFactory& fac) override {
      fac.SetData<T>(this->collection_names[0], this->m_data);
    }

    void Reset() override { m_data.clear(); }
  };

  template <typename PodioT> class PodioOutput : public OutputBase {

    std::unique_ptr<typename PodioTypeMap<PodioT>::collection_t> m_data;

  public:
    PodioOutput(JOmniFactory* owner, std::string default_collection_name = "") {
      owner->RegisterOutput(this);
      this->collection_names.push_back(default_collection_name);
      this->type_name = JTypeInfo::demangle<PodioT>();
    }

    std::unique_ptr<typename PodioTypeMap<PodioT>::collection_t>& operator()() { return m_data; }

  private:
    friend class JOmniFactory;

    void CreateHelperFactory(JOmniFactory& fac) override {
      fac.DeclarePodioOutput<PodioT>(this->collection_names[0]);
    }

    void SetCollection(JOmniFactory& fac) override {
      if (m_data == nullptr) {
        throw JException("JOmniFactory: SetCollection failed due to missing output collection '%s'",
                         this->collection_names[0].c_str());
        // Otherwise this leads to a PODIO segfault
      }
      fac.SetCollection<PodioT>(this->collection_names[0], std::move(this->m_data));
    }

    void Reset() override {
      m_data = std::move(std::make_unique<typename PodioTypeMap<PodioT>::collection_t>());
    }
  };

  template <typename PodioT> class VariadicPodioOutput : public OutputBase {

    std::vector<std::unique_ptr<typename PodioTypeMap<PodioT>::collection_t>> m_data;

  public:
    VariadicPodioOutput(JOmniFactory* owner,
                        std::vector<std::string> default_collection_names = {}) {
      owner->RegisterOutput(this);
      this->collection_names = default_collection_names;
      this->type_name        = JTypeInfo::demangle<PodioT>();
      this->is_variadic      = true;
    }

    std::vector<std::unique_ptr<typename PodioTypeMap<PodioT>::collection_t>>& operator()() {
      return m_data;
    }

  private:
    friend class JOmniFactory;

    void CreateHelperFactory(JOmniFactory& fac) override {
      for (auto& coll_name : this->collection_names) {
        fac.DeclarePodioOutput<PodioT>(coll_name);
      }
    }

    void SetCollection(JOmniFactory& fac) override {
      if (m_data.size() != this->collection_names.size()) {
        throw JException("JOmniFactory: VariadicPodioOutput SetCollection failed: Declared %d "
                         "collections, but provided %d.",
                         this->collection_names.size(), m_data.size());
        // Otherwise this leads to a PODIO segfault
      }
      std::size_t i = 0;
      for (auto& coll_name : this->collection_names) {
        fac.SetCollection<PodioT>(coll_name, std::move(this->m_data[i++]));
      }
    }

    void Reset() override {
      m_data.clear();
      for (auto& coll_name [[maybe_unused]] : this->collection_names) {
        m_data.push_back(std::make_unique<typename PodioTypeMap<PodioT>::collection_t>());
      }
    }
  };

  void RegisterOutput(OutputBase* output) { m_outputs.push_back(output); }

  // =================
  // Handle parameters
  // =================

  struct ParameterBase {
    std::string m_name;
    std::string m_description;
    virtual void Configure(JParameterManager& parman, const std::string& prefix) = 0;
    virtual void Configure(std::map<std::string, std::string> fields)            = 0;
  };

  template <typename T> class ParameterRef : public ParameterBase {

    T* m_data;

  public:
    ParameterRef(JOmniFactory* owner, std::string name, T& slot, std::string description = "") {
      owner->RegisterParameter(this);
      this->m_name        = name;
      this->m_description = description;
      m_data              = &slot;
    }

    const T& operator()() { return *m_data; }

  private:
    friend class JOmniFactory;

    void Configure(JParameterManager& parman, const std::string& prefix) override {
      parman.SetDefaultParameter(prefix + ":" + this->m_name, *m_data, this->m_description);
    }
    void Configure(std::map<std::string, std::string> fields) override {
      auto it = fields.find(this->m_name);
      if (it != fields.end()) {
        const auto& value_str = it->second;
        JParameterManager::Parse(value_str, *m_data);
      }
    }
  };

  template <typename T> class Parameter : public ParameterBase {

    T m_data;

  public:
    Parameter(JOmniFactory* owner, std::string name, T default_value, std::string description) {
      owner->RegisterParameter(this);
      this->m_name        = name;
      this->m_description = description;
      m_data              = default_value;
    }

    const T& operator()() { return m_data; }

  private:
    friend class JOmniFactory;

    void Configure(JParameterManager& parman, const std::string& /* prefix */) override {
      parman.SetDefaultParameter(m_prefix + ":" + this->m_name, m_data, this->m_description);
    }
    void Configure(std::map<std::string, std::string> fields) override {
      auto it = fields.find(this->m_name);
      if (it != fields.end()) {
        const auto& value_str = it->second;
        if constexpr (10000 * JVersion::major + 100 * JVersion::minor + 1 * JVersion::patch <
                      20102) {
          m_data = JParameterManager::Parse<T>(value_str);
        } else {
          JParameterManager::Parse(value_str, m_data);
        }
      }
    }
  };

  void RegisterParameter(ParameterBase* parameter) { m_parameters.push_back(parameter); }

  void ConfigureAllParameters(std::map<std::string, std::string> fields) {
    for (auto* parameter : this->m_parameters) {
      parameter->Configure(fields);
    }
  }

  // ===============
  // Handle services
  // ===============

  struct ServiceBase {
    virtual void Init(JApplication* app) = 0;
  };

  template <typename ServiceT> class Service : public ServiceBase {

    std::shared_ptr<ServiceT> m_data;

  public:
    Service(JOmniFactory* owner) { owner->RegisterService(this); }

    ServiceT& operator()() { return *m_data; }

  private:
    friend class JOmniFactory;

    void Init(JApplication* app) { m_data = app->GetService<ServiceT>(); }
  };

  void RegisterService(ServiceBase* service) { m_services.push_back(service); }

  // ================
  // Handle resources
  // ================

  struct ResourceBase {
    virtual void ChangeRun(const JEvent& event) = 0;
  };

  template <typename ServiceT, typename ResourceT, typename LambdaT>
  class Resource : public ResourceBase {
    ResourceT m_data;
    LambdaT m_lambda;

  public:
    Resource(JOmniFactory* owner, LambdaT lambda) : m_lambda(lambda) {
      owner->RegisterResource(this);
    };

    const ResourceT& operator()() { return m_data; }

  private:
    friend class JOmniFactory;

    void ChangeRun(const JEvent& event) {
      auto run_nr                       = event.GetRunNumber();
      std::shared_ptr<ServiceT> service = event.GetJApplication()->template GetService<ServiceT>();
      m_data                            = m_lambda(service, run_nr);
    }
  };

  void RegisterResource(ResourceBase* resource) { m_resources.push_back(resource); }

public:
  std::vector<InputBase*> m_inputs;
  std::vector<OutputBase*> m_outputs;
  std::vector<ParameterBase*> m_parameters;
  std::vector<ServiceBase*> m_services;
  std::vector<ResourceBase*> m_resources;

private:
  // App belongs on JMultifactory, it is just missing temporarily
  JApplication* m_app;

  // Plugin name belongs on JMultifactory, it is just missing temporarily
  std::string m_plugin_name;

  // Prefix for parameters and loggers, derived from plugin name and tag in PreInit().
  std::string m_prefix;

  /// Current logger
  std::shared_ptr<spdlog::logger> m_logger;

  /// Configuration
  ConfigT m_config;

public:
  std::size_t FindVariadicCollectionCount(std::size_t total_input_count,
                                          std::size_t variadic_input_count,
                                          std::size_t total_collection_count, bool is_input) {

    std::size_t variadic_collection_count =
        total_collection_count - (total_input_count - variadic_input_count);

    if (variadic_input_count == 0) {
      // No variadic inputs: check that collection_name count matches input count exactly
      if (total_input_count != total_collection_count) {
        throw JException(
            "JOmniFactory '%s': Wrong number of %s collection names: %d expected, %d found.",
            m_prefix.c_str(), (is_input ? "input" : "output"), total_input_count,
            total_collection_count);
      }
    } else {
      // Variadic inputs: check that we have enough collection names for the non-variadic inputs
      if (total_input_count - variadic_input_count > total_collection_count) {
        throw JException("JOmniFactory '%s': Not enough %s collection names: %d needed, %d found.",
                         m_prefix.c_str(), (is_input ? "input" : "output"),
                         total_input_count - variadic_input_count, total_collection_count);
      }

      // Variadic inputs: check that the variadic collection names is evenly divided by the variadic input count
      if (variadic_collection_count % variadic_input_count != 0) {
        throw JException("JOmniFactory '%s': Wrong number of %s collection names: %d found total, "
                         "but %d can't be distributed among %d variadic inputs evenly.",
                         m_prefix.c_str(), (is_input ? "input" : "output"), total_collection_count,
                         variadic_collection_count, variadic_input_count);
      }
    }
    return variadic_collection_count;
  }

  inline void PreInit(std::string tag, std::vector<std::string> default_input_collection_names,
                      std::vector<std::string> default_output_collection_names) {

    m_prefix = (this->GetPluginName().empty()) ? tag : this->GetPluginName() + ":" + tag;

    // Obtain collection name overrides if provided.
    // Priority = [JParameterManager, JOmniFactoryGenerator]
    m_app->SetDefaultParameter(m_prefix + ":InputTags", default_input_collection_names,
                               "Input collection names");
    m_app->SetDefaultParameter(m_prefix + ":OutputTags", default_output_collection_names,
                               "Output collection names");

    // Figure out variadic inputs
    std::size_t variadic_input_count = 0;
    for (auto* input : m_inputs) {
      if (input->is_variadic) {
        variadic_input_count += 1;
      }
    }
    std::size_t variadic_input_collection_count = FindVariadicCollectionCount(
        m_inputs.size(), variadic_input_count, default_input_collection_names.size(), true);

    // Set input collection names
    for (std::size_t i = 0; auto* input : m_inputs) {
      input->collection_names.clear();
      if (input->is_variadic) {
        for (std::size_t j = 0; j < (variadic_input_collection_count / variadic_input_count); ++j) {
          input->collection_names.push_back(default_input_collection_names[i++]);
        }
      } else {
        input->collection_names.push_back(default_input_collection_names[i++]);
      }
    }

    // Figure out variadic outputs
    std::size_t variadic_output_count = 0;
    for (auto* output : m_outputs) {
      if (output->is_variadic) {
        variadic_output_count += 1;
      }
    }
    std::size_t variadic_output_collection_count = FindVariadicCollectionCount(
        m_outputs.size(), variadic_output_count, default_output_collection_names.size(), true);

    // Set output collection names and create corresponding helper factories
    for (std::size_t i = 0; auto* output : m_outputs) {
      output->collection_names.clear();
      if (output->is_variadic) {
        for (std::size_t j = 0; j < (variadic_output_collection_count / variadic_output_count);
             ++j) {
          output->collection_names.push_back(default_output_collection_names[i++]);
        }
      } else {
        output->collection_names.push_back(default_output_collection_names[i++]);
      }
      output->CreateHelperFactory(*this);
    }

    // Obtain logger (defines the parameter option)
    m_logger = m_app->GetService<Log_service>()->logger(m_prefix);
  }

  void Init() override {
    auto app = GetApplication();
    for (auto* parameter : m_parameters) {
      parameter->Configure(*(app->GetJParameterManager()), m_prefix);
    }
    for (auto* service : m_services) {
      service->Init(app);
    }
    static_cast<AlgoT*>(this)->Configure();
  }

  void BeginRun(const std::shared_ptr<const JEvent>& event) override {
    for (auto* resource : m_resources) {
      resource->ChangeRun(*event);
    }
    static_cast<AlgoT*>(this)->ChangeRun(event->GetRunNumber());
  }

  virtual void ChangeRun(int32_t /* run_number */) override {};

  virtual void Process(int32_t /* run_number */, uint64_t /* event_number */) {};

  void Process(const std::shared_ptr<const JEvent>& event) override {
    try {
      // Reset outputs first to create empty collections
      for (auto* output : m_outputs) {
        output->Reset();
      }
      for (auto* input : m_inputs) {
        input->GetCollection(*event);
      }
#if SPDLOG_VERSION >= 11400 && (!defined(SPDLOG_NO_TLS) || !SPDLOG_NO_TLS)
      spdlog::mdc::put("e", std::to_string(event->GetEventNumber()));
#endif
      static_cast<AlgoT*>(this)->Process(event->GetRunNumber(), event->GetEventNumber());
      for (auto* output : m_outputs) {
        output->SetCollection(*this);
      }
    } catch (std::exception& e) {
      // On exception, still set empty collections for any outputs that were Reset()
      // This ensures podio frames have consistent collections across all events
      for (auto* output : m_outputs) {
        try {
          output->SetCollection(*this);
        } catch (...) {
          // Ignore errors setting empty collections
        }
      }
      throw JException(e.what());
    }
  }

  using ConfigType = ConfigT;

  void SetApplication(JApplication* app) { m_app = app; }

  JApplication* GetApplication() { return m_app; }

  void SetPluginName(std::string plugin_name) { m_plugin_name = plugin_name; }

  std::string GetPluginName() { return m_plugin_name; }

  inline std::string GetPrefix() { return m_prefix; }

  /// Retrieve reference to already-configured logger
  std::shared_ptr<spdlog::logger>& logger() { return m_logger; }

  /// Retrieve reference to embedded config object
  ConfigT& config() { return m_config; }
};
