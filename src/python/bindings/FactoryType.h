// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck
//
// Generic Python binding for JOmniFactory types (prototype, see docs/design/python_frontend.md).
//
// A FactoryType<T> is a type-erased handle on one JOmniFactory class T. It is created once per
// factory class in a per-domain Python extension module (e.g. eicrecon._calorimetry), and is used
// from Python to (1) introspect the factory signature and config defaults and (2) create a
// JFactoryGenerator for one wiring. The config struct T::ConfigType is translated to and from a
// Python dict by Boost.PFR field-name reflection, so no hand-written binding per config struct is
// needed.

#pragma once

#include <JANA/JApplication.h>
#include <JANA/JFactoryGenerator.h>
#include <JANA/Utils/JTypeInfo.h>
#include <boost/pfr.hpp>
#include <boost/pfr/core_name.hpp>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <memory>
#include <set>
#include <string>
#include <type_traits>
#include <vector>

#include "extensions/jana/JOmniFactoryGeneratorT.h"

namespace eicrecon::python {

namespace py = pybind11;

/// Same as JOmniFactoryGeneratorT, but with an explicit plugin name. JComponentManager::Add()
/// overwrites the plugin name with the plugin currently being loaded, which is empty for
/// generators added from Python before JApplication::Initialize(). The plugin name is part of the
/// parameter prefix ("<plugin>:<tag>:<param>"), so it is restored here to keep -P overrides
/// identical to the C++ plugin wiring.
template <class FactoryT> class PyOmniFactoryGeneratorT : public JOmniFactoryGeneratorT<FactoryT> {
public:
  PyOmniFactoryGeneratorT(std::string plugin, std::string tag, std::vector<std::string> inputs,
                          std::vector<std::string> outputs, typename FactoryT::ConfigType cfg,
                          JApplication* app)
      : JOmniFactoryGeneratorT<FactoryT>(std::move(tag), std::move(inputs), std::move(outputs),
                                         std::move(cfg), app)
      , m_plugin(std::move(plugin)) {}

  void GenerateFactories(JFactorySet* factory_set) override {
    this->SetPluginName(m_plugin);
    JOmniFactoryGeneratorT<FactoryT>::GenerateFactories(factory_set);
  }

private:
  std::string m_plugin;
};

/// Type-erased handle on a JOmniFactory class. Default visibility so pybind11 can share the
/// registered type between eicrecon._core and the per-domain extension modules.
struct __attribute__((visibility("default"))) FactoryTypeBase {
  virtual ~FactoryTypeBase() = default;
  /// Short Python-facing name, e.g. "CalorimeterHitDigi"
  virtual std::string name() const = 0;
  /// Demangled C++ factory class name
  virtual std::string cpp_type() const = 0;
  /// Default-constructed config struct as a dict {field: value}
  virtual py::dict config_defaults() const = 0;
  /// {"inputs": [(type, variadic), ...], "outputs": [...]} from a throwaway factory instance
  virtual py::dict signature() const = 0;
  /// Convert config to the C++ struct and discard it; raises on unknown fields or bad types
  virtual void validate_config(const py::dict& config, const std::string& context) const = 0;
  /// Validate config and create a generator; ownership passes to the caller (JApplication)
  virtual JFactoryGenerator* make_generator(JApplication* app, const std::string& plugin,
                                            const std::string& tag,
                                            const std::vector<std::string>& inputs,
                                            const std::vector<std::string>& outputs,
                                            const py::dict& config) const = 0;
};

/// Convert a config struct to a Python dict, field by field.
template <class ConfigT> py::dict config_to_dict(const ConfigT& cfg) {
  py::dict d;
  boost::pfr::for_each_field_with_name(cfg, [&](std::string_view name, const auto& field) {
    d[py::str(std::string(name))] = py::cast(field);
  });
  return d;
}

/// Overlay a Python dict onto a config struct. Unknown keys and values that cannot be converted
/// to the field type raise Python exceptions, so typos fail when the graph is configured.
template <class ConfigT> ConfigT config_from_dict(const py::dict& d, const std::string& context) {
  ConfigT cfg{};
  std::set<std::string> used;
  boost::pfr::for_each_field_with_name(cfg, [&](std::string_view name_sv, auto& field) {
    std::string name(name_sv);
    if (!d.contains(name)) {
      return;
    }
    using FieldT = std::decay_t<decltype(field)>;
    try {
      field = py::cast<FieldT>(d[py::str(name)]);
    } catch (const py::cast_error&) {
      throw py::type_error(context + ": cannot convert value for config field '" + name +
                           "' to C++ type " + JTypeInfo::demangle<FieldT>());
    }
    used.insert(name);
  });
  for (auto item : d) {
    auto key = py::cast<std::string>(item.first);
    if (used.count(key) == 0) {
      throw py::key_error(context + ": unknown config field '" + key + "' for " +
                          JTypeInfo::demangle<ConfigT>());
    }
  }
  return cfg;
}

template <class FactoryT> class FactoryType : public FactoryTypeBase {
public:
  using ConfigT = typename FactoryT::ConfigType;

  explicit FactoryType(std::string name) : m_name(std::move(name)) {}

  std::string name() const override { return m_name; }
  std::string cpp_type() const override { return JTypeInfo::demangle<FactoryT>(); }

  py::dict config_defaults() const override { return config_to_dict(ConfigT{}); }

  py::dict signature() const override {
    FactoryT probe; // inputs/outputs register themselves in the constructor
    py::list ins, outs;
    for (const auto* in : probe.m_inputs) {
      ins.append(py::make_tuple(in->type_name, in->is_variadic));
    }
    for (const auto* out : probe.m_outputs) {
      outs.append(py::make_tuple(out->type_name, out->is_variadic));
    }
    py::dict d;
    d["inputs"]  = ins;
    d["outputs"] = outs;
    return d;
  }

  void validate_config(const py::dict& config, const std::string& context) const override {
    (void)config_from_dict<ConfigT>(config, context);
  }

  JFactoryGenerator* make_generator(JApplication* app, const std::string& plugin,
                                    const std::string& tag, const std::vector<std::string>& inputs,
                                    const std::vector<std::string>& outputs,
                                    const py::dict& config) const override {
    auto cfg = config_from_dict<ConfigT>(config, plugin + ":" + tag);
    return new PyOmniFactoryGeneratorT<FactoryT>(plugin, tag, inputs, outputs, std::move(cfg), app);
  }

private:
  std::string m_name;
};

/// Register FactoryT as attribute `name` of module m.
template <class FactoryT> void bind_factory(py::module_& m, const std::string& name) {
  m.attr(name.c_str()) = py::cast(
      std::static_pointer_cast<FactoryTypeBase>(std::make_shared<FactoryType<FactoryT>>(name)));
}

} // namespace eicrecon::python
