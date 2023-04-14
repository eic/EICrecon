
// Copyright 2023, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

/**
 * JChainFactories aim to easier wrap framework independent algorithms
 * It is assumed multiple input data (controlled by input tags)
 * which might be changed by user parameters.
 */

#pragma once

#include <string>
#include <vector>

#include <services/io/podio/datamodel_glue.h>
#include <JANA/JMultifactory.h>
#include <extensions/jana/JChainFactoryT.h>  // Just to pull in struct NoConfig
#include "extensions/string/StringHelpers.h"


template <typename ConfigT = NoConfig>
class JChainMultifactoryT : public JMultifactory {
public:

    using ConfigType = ConfigT;

    /// Constructor with NoConfig
    JChainMultifactoryT( std::string tag,
                    std::vector<std::string> input_tags,
                    std::vector<std::string> output_tags,
                    ConfigT cfg):
            m_tag(std::move(tag)),
            m_input_tags( std::move(input_tags) ),
            m_output_tags(std::move(output_tags)),
            m_default_cfg(cfg)
    {
    }

    /// Constructor for NoConfig case
    explicit JChainMultifactoryT( std::string tag, std::vector<std::string> input_tags, std::vector<std::string> output_tags):
            m_tag(std::move(tag)),
            m_input_tags( std::move(input_tags) ),
            m_output_tags(std::move(output_tags))
    {
    }

    /// The unique tag for the multifactory instance. Used for the param_prefix
    std::string& GetTag() { return m_tag; }

    /// Gets final input tags (with adjustment by user parameters)
    std::vector<std::string>& GetInputTags() { return m_input_tags; }

    /// Gets final output tags (with adjustment by user parameters)
    std::vector<std::string>& GetOutputTags() { return m_output_tags; }

    /// Gets config
    ConfigT& GetDefaultConfig() { return m_default_cfg; }


    // Setters and getters for things that really belong on JMultifactory
    // TODO: NWB: Remove
    void SetPluginName(std::string plugin_name) { m_plugin_name = std::move(plugin_name);}

    std::string GetPluginName() { return m_plugin_name; }

    void SetApplication(JApplication* app) { m_app = app; }

    JApplication* GetApplication() { return m_app; }

    void SetPrefix(std::string prefix) { m_prefix = std::move(prefix); }

    std::string& GetPrefix() { return m_prefix; }


protected:

    /// Underlying algorithm config
    ConfigT m_default_cfg;

private:
    // Used to uniquely identify an _instance_ of this particular JChainMultifactory (since there is no longer a unique
    // output tag we can use)
    std::string m_tag;

    // Prefix for parameters and loggers, derived from plugin name and tag in Init().
    std::string m_prefix;

    /// Working input tags (with adjustment by user parameters)
    std::vector<std::string> m_input_tags;

    /// Output tags
    std::vector<std::string> m_output_tags;

    /// TODO: NWB: Pull these directly into JMultifactory
    JApplication* m_app;
    std::string m_plugin_name;

};
