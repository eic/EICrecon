// Copyright 2023, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.
// Created by Nathan Brei

#pragma once

#include <JANA/JFactorySet.h>
#include <JANA/JFactoryGenerator.h>
#include <JANA/Utils/JTypeInfo.h>
#include <vector>

namespace eicrecon {

template <class ComponentT> class JOmniUnfolderGeneratorT {
public:
  using ConfigT = ComponentT::ConfigT;

  struct Wiring {
    std::string tag;
    JEventLevel parent_level = JEventLevel::Timeslice;
    JEventLevel child_level  = JEventLevel::PhysicsEvent;
    std::vector<std::string> input_names;
    std::vector<JEventLevel> input_levels;
    std::vector<std::vector<std::string>> variadic_input_names;
    std::vector<JEventLevel> variadic_input_levels;
    std::vector<std::string> output_names;
    std::vector<std::vector<std::string>> variadic_output_names;
    ConfigT configs = {}; // Must be copyable!
  };

private:
  Wiring m_wiring;

public:
  explicit JOmniUnfolderGeneratorT(Wiring&& wiring) : m_wiring(wiring) {}

  void Generate(JApplication* app) {

    auto component = new ComponentT;

    // EICrecon JOFG does _not_ include plugin name in prefix
    // Unlike with factories, plugin name is set automatically during app->Add()
    component->SetPrefix(m_wiring.tag);
    component->SetTypeName(JTypeInfo::demangle<ComponentT>());
    component->SetParentLevel(m_wiring.parent_level);
    component->SetChildLevel(m_wiring.child_level);
    component->config() = m_wiring.configs;

    // Override the input and output collection names via parameters
    // (This is also EICrecon-specific, the long term plan is to use the wiring file instead)
    app->SetDefaultParameter(m_wiring.tag + ":InputTags", m_wiring.input_names,
                             "Input collection names");
    app->SetDefaultParameter(m_wiring.tag + ":OutputTags", m_wiring.output_names,
                             "Output collection names");
    app->SetDefaultParameter(m_wiring.tag + ":VariadicInputTags", m_wiring.variadic_input_names,
                             "Input collection names");
    app->SetDefaultParameter(m_wiring.tag + ":VariadicOutputTags", m_wiring.variadic_output_names,
                             "Output collection names");

    component->WireInputs(m_wiring.parent_level, m_wiring.input_levels, m_wiring.input_names,
                          m_wiring.variadic_input_levels, m_wiring.variadic_input_names);
    component->WireOutputs(m_wiring.child_level, m_wiring.output_names,
                           m_wiring.variadic_output_names,
                           false // use_short_names
    );

    app->Add(component);
  }
};

} // namespace eicrecon
