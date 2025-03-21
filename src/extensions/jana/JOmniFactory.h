// Copyright 2023, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.
// Created by Nathan Brei

#pragma once

/**
 * Omnifactories are a lightweight layer connecting JANA to generic algorithms
 * It is assumed multiple input data (controlled by input tags)
 * which might be changed by user parameters.
 */

#include <JANA/JVersion.h>
#include <JANA/JMultifactory.h>
#include <JANA/JEvent.h>
#include <JANA/Components/JHasInputs.h>
#include <spdlog/spdlog.h>

#include "services/log/Log_service.h"

#include <string>
#include <vector>

struct EmptyConfig {};

template <typename AlgoT, typename ConfigT=EmptyConfig>
class JOmniFactory : public JMultifactory, public jana::components::JHasInputs {
public:

    /// =========================
    /// Handle output collections
    /// =========================

    struct OutputBase {
        std::string type_name;
        std::vector<std::string> collection_names;
        bool is_variadic = false;

        virtual void CreateHelperFactory(JOmniFactory& fac) = 0;
        virtual void SetCollection(JOmniFactory& fac) = 0;
        virtual void Reset() = 0;
    };

    template <typename T>
    class Output : public OutputBase {
        std::vector<T*> m_data;

    public:
        Output(JOmniFactory* owner, std::string default_tag_name="") {
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

        void Reset() override { }
    };


    template <typename PodioT>
    class PodioOutput : public OutputBase {

        std::unique_ptr<typename PodioT::collection_type> m_data;

    public:

        PodioOutput(JOmniFactory* owner, std::string default_collection_name="") {
            owner->RegisterOutput(this);
            this->collection_names.push_back(default_collection_name);
            this->type_name = JTypeInfo::demangle<PodioT>();
        }

        std::unique_ptr<typename PodioT::collection_type>& operator()() { return m_data; }

    private:
        friend class JOmniFactory;

        void CreateHelperFactory(JOmniFactory& fac) override {
            fac.DeclarePodioOutput<PodioT>(this->collection_names[0]);
        }

        void SetCollection(JOmniFactory& fac) override {
            if (m_data == nullptr) {
                throw JException("JOmniFactory: SetCollection failed due to missing output collection '%s'", this->collection_names[0].c_str());
                // Otherwise this leads to a PODIO segfault
            }
            fac.SetCollection<PodioT>(this->collection_names[0], std::move(this->m_data));
        }

        void Reset() override {
            m_data = std::move(std::make_unique<typename PodioT::collection_type>());
        }
    };


    template <typename PodioT>
    class VariadicPodioOutput : public OutputBase {

        std::vector<std::unique_ptr<typename PodioT::collection_type>> m_data;

    public:

        VariadicPodioOutput(JOmniFactory* owner, std::vector<std::string> default_collection_names={}) {
            owner->RegisterOutput(this);
            this->collection_names = default_collection_names;
            this->type_name = JTypeInfo::demangle<PodioT>();
            this->is_variadic = true;
        }

        std::vector<std::unique_ptr<typename PodioT::collection_type>>& operator()() { return m_data; }

    private:
        friend class JOmniFactory;

        void CreateHelperFactory(JOmniFactory& fac) override {
            for (auto& coll_name : this->collection_names) {
                fac.DeclarePodioOutput<PodioT>(coll_name);
            }
        }

        void SetCollection(JOmniFactory& fac) override {
            if (m_data.size() != this->collection_names.size()) {
                throw JException("JOmniFactory: VariadicPodioOutput SetCollection failed: Declared %d collections, but provided %d.", this->collection_names.size(), m_data.size());
                // Otherwise this leads to a PODIO segfault
            }
            size_t i = 0;
            for (auto& coll_name : this->collection_names) {
                fac.SetCollection<PodioT>(coll_name, std::move(this->m_data[i++]));
            }
        }

        void Reset() override {
            m_data.clear();
            for (auto& coll_name : this->collection_names) {
                m_data.push_back(std::make_unique<typename PodioT::collection_type>());
            }
        }
    };

    void RegisterOutput(OutputBase* output) {
        m_outputs.push_back(output);
    }


public:
    std::vector<OutputBase*> m_outputs;

private:
    /// Current logger
    std::shared_ptr<spdlog::logger> m_spd_logger;

    /// Configuration
    ConfigT m_config;

public:

    size_t FindVariadicCollectionCount(size_t total_input_count, size_t variadic_input_count, size_t total_collection_count, bool is_input) {

        size_t variadic_collection_count = total_collection_count - (total_input_count - variadic_input_count);

        if (variadic_input_count == 0) {
            // No variadic inputs: check that collection_name count matches input count exactly
            if (total_input_count != total_collection_count) {
                throw JException("JOmniFactory '%s': Wrong number of %s collection names: %d expected, %d found.",
                                m_prefix.c_str(), (is_input ? "input" : "output"), total_input_count, total_collection_count);
            }
        }
        else {
            // Variadic inputs: check that we have enough collection names for the non-variadic inputs
            if (total_input_count-variadic_input_count > total_collection_count) {
                throw JException("JOmniFactory '%s': Not enough %s collection names: %d needed, %d found.",
                                m_prefix.c_str(), (is_input ? "input" : "output"), total_input_count-variadic_input_count, total_collection_count);
            }

            // Variadic inputs: check that the variadic collection names is evenly divided by the variadic input count
            if (variadic_collection_count % variadic_input_count != 0) {
                throw JException("JOmniFactory '%s': Wrong number of %s collection names: %d found total, but %d can't be distributed among %d variadic inputs evenly.",
                                m_prefix.c_str(), (is_input ? "input" : "output"), total_collection_count, variadic_collection_count, variadic_input_count);
            }
        }
        return variadic_collection_count;
    }

    inline void PreInit(std::string tag,
                        JEventLevel level,
                        std::vector<std::string> input_collection_names,
                        std::vector<JEventLevel> input_collection_levels,
                        std::vector<std::string> output_collection_names ) {

        m_prefix = (this->GetPluginName().empty()) ? tag : this->GetPluginName() + ":" + tag;
        m_level = level;

        // Obtain collection name overrides if provided.
        // Priority = [JParameterManager, JOmniFactoryGenerator]
        m_app->SetDefaultParameter(m_prefix + ":InputTags", input_collection_names, "Input collection names");
        m_app->SetDefaultParameter(m_prefix + ":OutputTags", output_collection_names, "Output collection names");

        // Figure out variadic inputs
        size_t variadic_input_count = 0;
        for (auto* input : m_inputs) {
            if (input->is_variadic) {
               variadic_input_count += 1;
            }
        }
        size_t variadic_input_collection_count = FindVariadicCollectionCount(m_inputs.size(), variadic_input_count, input_collection_names.size(), true);

        // Set input collection names
        size_t i = 0;
        for (auto* input : m_inputs) {
            input->names.clear();
            if (input->is_variadic) {
                for (size_t j = 0; j<(variadic_input_collection_count/variadic_input_count); ++j) {
                    input->names.push_back(input_collection_names[i++]);
                    if (!input_collection_levels.empty()) {
                        input->levels.push_back(input_collection_levels[i++]);
                    }
                    else {
                        input->levels.push_back(level);
                    }
                }
            }
            else {
                input->names.push_back(input_collection_names[i++]);
                if (!input_collection_levels.empty()) {
                    input->levels.push_back(input_collection_levels[i++]);
                }
                else {
                    input->levels.push_back(level);
                }
            }
        }

        // Figure out variadic outputs
        size_t variadic_output_count = 0;
        for (auto* output : m_outputs) {
            if (output->is_variadic) {
               variadic_output_count += 1;
            }
        }
        size_t variadic_output_collection_count = FindVariadicCollectionCount(m_outputs.size(), variadic_output_count, output_collection_names.size(), false);

        // Set output collection names and create corresponding helper factories
        i = 0;
        for (auto* output : m_outputs) {
            output->collection_names.clear();
            if (output->is_variadic) {
                for (size_t j = 0; j<(variadic_output_collection_count/variadic_output_count); ++j) {
                    output->collection_names.push_back(output_collection_names[i++]);
                }
            }
            else {
                output->collection_names.push_back(output_collection_names[i++]);
            }
            output->CreateHelperFactory(*this);
        }

        // Obtain loggers
        m_logger = m_app->GetService<JParameterManager>()->GetLogger(m_prefix);
        m_spd_logger = m_app->GetService<Log_service>()->logger(m_prefix);
    }

    void Init() override {
        for (auto* parameter : m_parameters) {
            parameter->Init(*(m_app->GetJParameterManager()), m_prefix);
        }
        for (auto* service : m_services) {
            service->Fetch(m_app);
        }
        static_cast<AlgoT*>(this)->Configure();
    }

    void BeginRun(const std::shared_ptr<const JEvent>& event) override {
        for (auto* resource : m_resources) {
            resource->ChangeRun(event->GetRunNumber(), m_app);
        }
        static_cast<AlgoT*>(this)->ChangeRun(event->GetRunNumber());
    }

    void Process(const std::shared_ptr<const JEvent> &event) override {
        try {
            for (auto* input : m_inputs) {
                input->GetCollection(*event);
            }
            for (auto* output : m_outputs) {
                output->Reset();
            }
            static_cast<AlgoT*>(this)->Process(event->GetRunNumber(), event->GetEventNumber());
            for (auto* output : m_outputs) {
                output->SetCollection(*this);
            }
        }
        catch(std::exception &e) {
            throw JException(e.what());
        }
    }

    using ConfigType = ConfigT;

    /// Retrieve reference to already-configured logger
    std::shared_ptr<spdlog::logger> &logger() { return m_spd_logger; }

    /// Retrieve reference to embedded config object
    ConfigT& config() { return m_config; }


    /// Generate summary for UI, inspector
    void Summarize(JComponentSummary& summary) const override {

        auto* mfs = new JComponentSummary::Component(
            "OmniFactory", GetPrefix(), GetTypeName(), GetLevel(), GetPluginName());

        for (const auto* input : m_inputs) {
            size_t subinput_count = input->names.size();
            for (size_t i=0; i<subinput_count; ++i) {
                mfs->AddInput(new JComponentSummary::Collection("", input->names[i], input->type_name, input->levels[i]));
            }
        }
        for (const auto* output : m_outputs) {
            size_t suboutput_count = output->collection_names.size();
            for (size_t i=0; i<suboutput_count; ++i) {
                mfs->AddOutput(new JComponentSummary::Collection("", output->collection_names[i], output->type_name, GetLevel()));
            }
        }
        summary.Add(mfs);
    }

};



