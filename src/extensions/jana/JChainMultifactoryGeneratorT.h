
// Copyright 2023, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

#include <vector>

#include <JANA/JFactorySet.h>
#include <JANA/JFactoryGenerator.h>

#include "JChainMultifactoryT.h"

template<class FactoryT>
class JChainMultifactoryGeneratorT : public JFactoryGenerator {

public:

    // using FactoryOuptutType = typename FactoryT::OutputType;
    using FactoryConfigType = typename FactoryT::ConfigType;

    /// Constructor with config
    explicit JChainMultifactoryGeneratorT(std::string tag, std::vector<std::string> default_input_tags, std::vector<std::string> output_tags, FactoryConfigType cfg, JApplication* app):
            m_tag(tag),
            m_default_input_tags(std::move(default_input_tags)),
            m_default_output_tags(std::move(output_tags)),
            m_default_cfg(cfg),
            m_app(app)
    {
        Init();
    };

    /// Constructor for NoConfig configuration
    explicit JChainMultifactoryGeneratorT(std::string tag, std::vector<std::string> default_input_tags, std::vector<std::string> output_tags, JApplication* app):
            m_tag(tag),
            m_default_input_tags(std::move(default_input_tags)),
            m_default_output_tags(std::move(output_tags)),
            m_app(app)
    {
        Init();
    };

    void GenerateFactories(JFactorySet *factory_set) override {

        FactoryT *factory;
        if constexpr(std:: is_base_of<NoConfig,FactoryConfigType>()) {
            factory = new FactoryT(m_tag, m_input_tags, m_output_tags);
        } else {
            factory = new FactoryT(m_tag, m_input_tags, m_output_tags, m_default_cfg);
        }
        factory->SetPrefix(m_prefix);
        factory->SetFactoryName(JTypeInfo::demangle<FactoryT>());
        factory->SetPluginName(this->GetPluginName());
        factory->SetApplication(m_app);
        factory_set->Add(factory);
    }


    void Init() {

        std::string plugin_name = eicrecon::str::ReplaceAll(this->GetPluginName(), ".so", "");
        m_prefix = plugin_name+ ":" + m_tag;

        // Get input data tags
        m_app->SetDefaultParameter(m_prefix + ":InputTags", m_input_tags, "Input data tag names");
        if(m_input_tags.empty()) {
            m_input_tags = m_default_input_tags;
        }

        m_app->SetDefaultParameter(m_prefix + ":OutputTags", m_output_tags, "Output data tag names");
        if(m_output_tags.empty()) {
            m_output_tags = m_default_output_tags;
        }
    }


private:
    std::string m_tag;
    std::string m_prefix;
    std::vector<std::string> m_default_input_tags;
    std::vector<std::string> m_default_output_tags;
    std::vector<std::string> m_input_tags;
    std::vector<std::string> m_output_tags;

    FactoryConfigType m_default_cfg;                   /// Default config for a factories. (!) Must be properly copyable
    JApplication* m_app; // TODO: NWB: Remove me
};
