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

#include <services/io/podio/JFactoryPodioT.h>
#include "extensions/string/StringHelpers.h"


/// This struct might be used for factories that has no underlying config class
/// @example:
///    JChainFactoryT<OutputType, NoConfig>
struct NoConfig {
};

template <typename OutT, typename ConfigT = NoConfig, template <typename> typename BaseT = eicrecon::JFactoryPodioT>
class JChainFactoryT : public BaseT<OutT> {
public:

    using OutputType = OutT;
    using ConfigType = ConfigT;

    /// Constructor with NoConfig
    JChainFactoryT( std::vector<std::string> default_input_tags, ConfigT cfg):
            m_default_input_tags( std::move(default_input_tags) ),
            m_default_cfg(cfg)
    {
    }

    /// Constructor for NoConfig case
    explicit JChainFactoryT( std::vector<std::string> default_input_tags):
            m_default_input_tags( std::move(default_input_tags) )
    {
    }

    /// Initializes input tags from user parameters
    /// \param param_prefix
    void InitDataTags(const std::string &param_prefix) {
        auto app = this->GetApplication();

        // Get input data tags
        app->SetDefaultParameter(param_prefix + ":InputTags", m_input_tags, "Input data tag name");
        if(m_input_tags.empty()) {
            m_input_tags = GetDefaultInputTags();
        }
    }

    /// Gets input tags that are set by default (if user parameters are not provided)
    std::vector<std::string>& GetDefaultInputTags() { return m_default_input_tags; }

    /// Gets final input tags (with adjustment by user parameters)
    std::vector<std::string>& GetInputTags() { return m_input_tags; }

    /// Gets config
    ConfigT& GetDefaultConfig() { return m_default_cfg; }

    /// Get default prefix name
    std::string GetDefaultParameterPrefix() {
        std::string plugin_name = eicrecon::str::ReplaceAll(this->GetPluginName(), ".so", "");
        std::string param_prefix = plugin_name+ ":" + this->GetTag();
        return std::move(param_prefix);
    }


protected:

    /// Underlying algorithm config
    ConfigT m_default_cfg;

private:
    /// Factory input tags that are set by default (if user parameters are not provided)
    std::vector<std::string> m_default_input_tags;

    /// Working input tags (with adjustment by user parameters)
    std::vector<std::string> m_input_tags;              /// Tag for the input data
};
