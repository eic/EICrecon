// Copyright 2023, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.
// Created by Nathan Brei

#pragma once

#include <JANA/JFactorySet.h>
#include <JANA/JFactoryGenerator.h>
#include <vector>

template <class FactoryT> class JOmniFactoryGeneratorT : public JFactoryGenerator {
public:
  using FactoryConfigType = typename FactoryT::ConfigType;

private:
  struct TypedWiring {
    std::string m_tag;
    std::vector<std::string> m_default_input_tags;
    std::vector<std::string> m_default_output_tags;
    FactoryConfigType m_default_cfg; /// Must be properly copyable!
  };

  struct UntypedWiring {
    std::string m_tag;
    std::vector<std::string> m_default_input_tags;
    std::vector<std::string> m_default_output_tags;
    std::map<std::string, std::string> m_config_params;
  };

public:
  explicit JOmniFactoryGeneratorT(std::string tag, std::vector<std::string> default_input_tags,
                                  std::vector<std::string> default_output_tags,
                                  FactoryConfigType cfg, JApplication* app) {
    m_app = app;
    m_wirings.push_back({.m_tag                 = tag,
                         .m_default_input_tags  = default_input_tags,
                         .m_default_output_tags = default_output_tags,
                         .m_default_cfg         = cfg});
  };

  explicit JOmniFactoryGeneratorT(std::string tag, std::vector<std::string> default_input_tags,
                                  std::vector<std::string> default_output_tags, JApplication* app) {
    m_app = app;
    m_wirings.push_back({.m_tag                 = tag,
                         .m_default_input_tags  = default_input_tags,
                         .m_default_output_tags = default_output_tags,
                         .m_default_cfg         = {}});
  }

  explicit JOmniFactoryGeneratorT(JApplication* app) : m_app(app) {}

  void AddWiring(std::string tag, std::vector<std::string> default_input_tags,
                 std::vector<std::string> default_output_tags, FactoryConfigType cfg) {

    m_wirings.push_back({.m_tag                 = tag,
                         .m_default_input_tags  = default_input_tags,
                         .m_default_output_tags = default_output_tags,
                         .m_default_cfg         = cfg});
  }

  void AddWiring(std::string tag, std::vector<std::string> default_input_tags,
                 std::vector<std::string> default_output_tags,
                 std::map<std::string, std::string> config_params) {

    // Create throwaway factory so we can populate its config using our map<string,string>.
    FactoryT factory;
    factory.ConfigureAllParameters(config_params);
    auto config = factory.config();

    m_wirings.push_back({.m_tag                 = tag,
                         .m_default_input_tags  = default_input_tags,
                         .m_default_output_tags = default_output_tags,
                         .m_default_cfg         = config});
  }

  void GenerateFactories(JFactorySet* factory_set) override {

    for (const auto& wiring : m_wirings) {

      FactoryT* factory = new FactoryT;
      factory->SetApplication(m_app);

      // CRITICAL: SetPluginName MUST be called BEFORE PreInit!
      // PreInit calls CreateHelperFactory which calls DeclareOutput/DeclarePodioOutput
      // Those methods create Helper factories that copy m_plugin_name from parent
      factory->SetPluginName(this->GetPluginName());

      factory->SetFactoryName(JTypeInfo::demangle<FactoryT>());
      factory->config() = wiring.m_default_cfg;

      // Set up all of the wiring prereqs so that Init() can do its thing
      // Specifically, it needs valid input/output tags, a valid logger, and
      // valid default values in its Config object
      factory->PreInit(wiring.m_tag, wiring.m_default_input_tags, wiring.m_default_output_tags);

      // Factory is ready
      factory_set->Add(factory);
    }
  }

private:
  std::vector<TypedWiring> m_wirings;
  JApplication* m_app;
};
