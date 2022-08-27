// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifndef EICRECON_CHAIN_FACTORY_GENERATOR_T_H
#define EICRECON_CHAIN_FACTORY_GENERATOR_T_H


#include <vector>

#include <JANA/JFactorySet.h>
#include <JANA/JFactoryGenerator.h>

#include "JChainFactoryT.h"

template<class T>
class JChainFactoryGeneratorT : public JFactoryGenerator {

public:

    explicit JChainFactoryGeneratorT(std::vector<std::string> default_input_tags, std::string tag):
        m_default_input_tags(std::move(default_input_tags)),
        m_tag(std::move(tag))
        {};

    void GenerateFactories(JFactorySet *factory_set) override {
        T *factory = new T(m_default_input_tags);

        factory->SetTag(m_tag);
        factory->SetFactoryName(JTypeInfo::demangle<T>());
        factory->SetPluginName(this->GetPluginName());
        factory_set->Add(factory);
    }

    std::vector<std::string>& GetDefaultInputTags() { return m_default_input_tags; }

private:
    std::string m_tag;
    std::vector<std::string> m_default_input_tags;
};


#endif //EICRECON_CHAIN_FACTORY_GENERATOR_T_H
