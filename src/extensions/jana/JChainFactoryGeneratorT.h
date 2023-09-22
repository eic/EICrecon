// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once


#include <vector>

#include <JANA/JFactorySet.h>
#include <JANA/JFactoryGenerator.h>

#include "JChainFactoryT.h"

template<class FactoryT>
class JChainFactoryGeneratorT : public JFactoryGenerator {

public:

    using FactoryOutputType = typename FactoryT::OutputType;
    using FactoryConfigType = typename FactoryT::ConfigType;

    /// Constructor with config
    explicit JChainFactoryGeneratorT(std::vector<std::string> default_input_tags, std::string tag, FactoryConfigType cfg):
            m_default_input_tags(std::move(default_input_tags)),
            m_output_tag(std::move(tag)),
            m_default_cfg(cfg)
        {};

    /// Constructor for NoConfig configuration
    explicit JChainFactoryGeneratorT(std::vector<std::string> default_input_tags, std::string tag):
            m_default_input_tags(std::move(default_input_tags)),
            m_output_tag(std::move(tag))
    {};

    void GenerateFactories(JFactorySet *factory_set) override {

        FactoryT *factory;
        if constexpr(std::is_base_of<eicrecon::NoConfig,FactoryConfigType>()) {
            factory = new FactoryT(m_default_input_tags);
        } else {
            factory = new FactoryT(m_default_input_tags, m_default_cfg);
        }



        factory->SetTag(m_output_tag);
        factory->SetFactoryName(JTypeInfo::demangle<FactoryT>());
        factory->SetPluginName(this->GetPluginName());
        factory_set->Add(factory);
    }

    std::vector<std::string>& GetDefaultInputTags() { return m_default_input_tags; }

private:
    std::string m_output_tag;
    std::vector<std::string> m_default_input_tags;
    FactoryConfigType m_default_cfg;                   /// Default config for a factories. (!) Must be properly copyable
};
