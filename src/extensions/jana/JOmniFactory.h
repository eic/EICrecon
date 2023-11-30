// Copyright 2023, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.
// Created by Nathan Brei

#pragma once

/**
 * Omnifactories are a lightweight layer connecting JANA to generic algorithms
 * It is assumed multiple input data (controlled by input tags)
 * which might be changed by user parameters.
 */

#include <datamodel_glue.h>
#include <JANA/JMultifactory.h>
#include <JANA/JEvent.h>
#include <spdlog/spdlog.h>
#include "extensions/spdlog/SpdlogExtensions.h"
#include <services/log/Log_service.h>

#include <string>
#include <vector>

using namespace eicrecon;
struct EmptyConfig {};

template <typename AlgoT, typename ConfigT=EmptyConfig>
class JOmniFactory : public JMultifactory {
public:

    /// ========================
    /// Handle input collections
    /// ========================

    struct InputBase {
        std::string type_name;
        std::string collection_name;
        virtual void GetCollection(const JEvent& event) = 0;
    };

    template <typename T>
    class Input : public InputBase {

        std::vector<const T*> m_data;

    public:
        Input(JOmniFactory* owner, std::string default_tag="") {
            owner->RegisterInput(this);
            this->collection_name = default_tag;
            this->type_name = JTypeInfo::demangle<T>();
        }

        const std::vector<const T*>& operator()() { return m_data; }

    private:
        friend class JOmniFactory;

        void GetData(const JEvent& event) {
            m_data = event.Get<T>(this->collection_name);
        }
    };


    template <typename PodioT>
    class PodioInput : public InputBase {

        const typename PodioTypeMap<PodioT>::collection_t* m_data;

    public:

        PodioInput(JOmniFactory* owner, std::string default_collection_name="") {
            owner->RegisterInput(this);
            this->collection_name = default_collection_name;
            this->type_name = JTypeInfo::demangle<PodioT>();
        }

        const typename PodioTypeMap<PodioT>::collection_t* operator()() {
            return m_data;
        }

    private:
        friend class JOmniFactory;

        void GetCollection(const JEvent& event) {
            m_data = event.GetCollection<PodioT>(this->collection_name);
        }
    };

    void RegisterInput(InputBase* input) {
        m_inputs.push_back(input);
    }



    /// =========================
    /// Handle output collections
    /// =========================

    struct OutputBase {
        std::string type_name;
        std::string collection_name;
        virtual void CreateHelperFactory(JOmniFactory& fac) = 0;
        virtual void SetCollection(JOmniFactory& fac) = 0;
    };

    template <typename T>
    class Output : public OutputBase {
        std::vector<T*> m_data;

    public:
        Output(JOmniFactory* owner, std::string default_tag_name="") {
            owner->RegisterOutput(this);
            this->collection_name = default_tag_name;
            this->type_name = JTypeInfo::demangle<T>();
        }

        std::vector<T*>& operator()() { return m_data; }

    private:
        friend class JOmniFactory;

        void CreateHelperFactory(JOmniFactory& fac) override {
            fac.DeclareOutput<T>(this->tag_name);
        }

        void SetCollection(JOmniFactory& fac) override {
            fac.SetData<T>(this->tag_name, this->data);
        }
    };


    template <typename PodioT>
    class PodioOutput : public OutputBase {

        std::unique_ptr<typename PodioTypeMap<PodioT>::collection_t> m_data;

    public:

        PodioOutput(JOmniFactory* owner, std::string default_collection_name="") {
            owner->RegisterOutput(this);
            this->collection_name = default_collection_name;
            this->type_name = JTypeInfo::demangle<PodioT>();
        }

        std::unique_ptr<typename PodioTypeMap<PodioT>::collection_t>& operator()() { return m_data; }

    private:
        friend class JOmniFactory;

        void CreateHelperFactory(JOmniFactory& fac) override {
            fac.DeclarePodioOutput<PodioT>(this->collection_name);
        }

        void SetCollection(JOmniFactory& fac) {
            if (m_data == nullptr) {
                throw JException("JOmniFactory: SetCollection failed due to missing output collection '%s'", this->collection_name.c_str());
                // Otherwise this leads to a PODIO segfault
            }
            fac.SetCollection<PodioT>(this->collection_name, std::move(this->m_data));
        }
    };

    void RegisterOutput(OutputBase* output) {
        m_outputs.push_back(output);
    }


    // =================
    // Handle parameters
    // =================

    struct ParameterBase {
        std::string m_name;
        std::string m_description;
        virtual void Configure(JParameterManager& parman, const std::string& prefix) = 0;
        virtual void Configure(std::map<std::string, std::string> fields) = 0;
    };

    template <typename T>
    class ParameterRef : public ParameterBase {

        T* m_data;

    public:
        ParameterRef(JOmniFactory* owner, std::string name, T& slot, std::string description="") {
            owner->RegisterParameter(this);
            this->m_name = name;
            this->m_description = description;
            m_data = &slot;
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
                *m_data = JParameterManager::Parse<T>(value_str);
            }
        }
    };

    template <typename T>
    class Parameter : public ParameterBase {

        T m_data;

    public:
        Parameter(JOmniFactory* owner, std::string name, T default_value, std::string description) {
            owner->RegisterParameter(this);
            this->m_name = name;
            this->m_description = description;
            m_data = default_value;
        }

        const T& operator()() { return m_data; }

    private:
        friend class JOmniFactory;

        void Configure(JParameterManager& parman, const std::string& prefix) override {
            parman.SetDefaultParameter(m_prefix + ":" + this->m_name, m_data, this->m_description);
        }
        void Configure(std::map<std::string, std::string> fields) override {
            auto it = fields.find(this->m_name);
            if (it != fields.end()) {
                const auto& value_str = it->second;
                m_data = JParameterManager::Parse<T>(value_str);
            }
        }
    };

    void RegisterParameter(ParameterBase* parameter) {
        m_parameters.push_back(parameter);
    }

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

    template <typename ServiceT>
    class Service : public ServiceBase {

        std::shared_ptr<ServiceT> m_data;

    public:

        Service(JOmniFactory* owner) {
            owner->RegisterService(this);
        }

        ServiceT& operator()() {
            return *m_data;
        }

    private:

        friend class JOmniFactory;

        void Init(JApplication* app) {
            m_data = app->GetService<ServiceT>();
        }

    };

    void RegisterService(ServiceBase* service) {
        m_services.push_back(service);
    }


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
            auto run_nr = event.GetRunNumber();
            std::shared_ptr<ServiceT> service = event.GetJApplication()->template GetService<ServiceT>();
            m_data = m_lambda(service, run_nr);
        }
    };

    void RegisterResource(ResourceBase* resource) {
        m_resources.push_back(resource);
    }


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
    inline void PreInit(std::string tag,
                        std::vector<std::string> default_input_collection_names,
                        std::vector<std::string> default_output_collection_names ) {

        // TODO: NWB: JMultiFactory::GetTag,SetTag are not currently usable
        m_prefix = (this->GetPluginName().empty()) ? tag : this->GetPluginName() + ":" + tag;

        // Obtain collection name overrides if provided.
        // Priority = [JParameterManager, JOmniFactoryGenerator]
        m_app->SetDefaultParameter(m_prefix + ":InputTags", default_input_collection_names, "Input collection names");
        m_app->SetDefaultParameter(m_prefix + ":OutputTags", default_output_collection_names, "Output collection names");

        // Set input collection names
        if (m_inputs.size() != default_input_collection_names.size()) {
            throw JException("JOmniFactory '%s': Wrong number of input collection names: %d expected, %d found.",
                             m_prefix.c_str(), m_inputs.size(), default_input_collection_names.size());
        }
        size_t i = 0;
        for (auto* input : m_inputs) {
           input->collection_name = default_input_collection_names[i++];
        }

        // Set output collection names and create corresponding helper factories
        if (m_outputs.size() != default_output_collection_names.size()) {
            throw JException("JOmniFactory '%s': Wrong number of output collection names: %d expected, %d found.",
                             m_prefix.c_str(), m_outputs.size(), default_output_collection_names.size());
        }
        i = 0;
        for (auto* output : m_outputs) {
            output->collection_name = default_output_collection_names[i++];
            output->CreateHelperFactory(*this);
        }

        // Obtain logger
        m_logger = m_app->GetService<Log_service>()->logger(m_prefix);

        // Configure logger. Priority = [JParameterManager, system log level]
        std::string default_log_level = eicrecon::LogLevelToString(m_logger->level());
        m_app->SetDefaultParameter(m_prefix + ":LogLevel", default_log_level, "LogLevel: trace, debug, info, warn, err, critical, off");
        m_logger->set_level(eicrecon::ParseLogLevel(default_log_level));
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

    void Process(const std::shared_ptr<const JEvent> &event) override {
        try {
            for (auto* input : m_inputs) {
                input->GetCollection(*event);
            }
            static_cast<AlgoT*>(this)->Process(event->GetRunNumber(), event->GetEventNumber());
            for (auto* output : m_outputs) {
                output->SetCollection(*this);
            }
        }
        catch(std::exception &e) {
            throw JException(e.what());
            // TODO: NWB: JMultifactory ought to already do this... test and fix if it doesn't
        }
    }


    using ConfigType = ConfigT;

    void SetApplication(JApplication* app) { m_app = app; }

    JApplication* GetApplication() { return m_app; }

    void SetPluginName(std::string plugin_name) { m_plugin_name = plugin_name; }

    std::string GetPluginName() { return m_plugin_name; }

    inline std::string GetPrefix() { return m_prefix; }

    /// Retrieve reference to already-configured logger
    std::shared_ptr<spdlog::logger> &logger() { return m_logger; }

    /// Retrieve reference to embedded config object
    ConfigT& config() { return m_config; }

};
