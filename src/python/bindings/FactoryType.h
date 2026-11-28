// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck
//
// Generic Python binding for JOmniFactory types (see docs/design/python_frontend.md).
//
// A FactoryType<T> is a type-erased handle on one JOmniFactory class T. It is created in a
// Python extension module (one per factory domain, eicrecon._<domain>), and is used from
// Python to (1) introspect the factory signature and config defaults and (2) create a
// JFactoryGenerator for one wiring. The config struct T::ConfigType is translated to and from
// Python values by Boost.PFR field-name reflection (ConfigConversion.h), so no hand-written
// binding per config struct is needed.

#pragma once

#include <JANA/JApplication.h>
#include <JANA/JFactoryGenerator.h>
#include <JANA/Utils/JTypeInfo.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // IWYU pragma: keep

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <typeinfo>
#include <vector>

#include "ConfigConversion.h"
#include "extensions/jana/JOmniFactory.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"

namespace eicrecon::python {

namespace py = pybind11;

/// Same as JOmniFactoryGeneratorT, but with an explicit plugin name. JComponentManager::Add()
/// overwrites the plugin name with the plugin currently being loaded, which is empty for
/// generators added from Python before JApplication::Initialize(). The plugin name is part of the
/// parameter prefix ("<plugin>:<tag>:<param>") and of the algorithm name that seeds random
/// numbers, so it is restored here to keep results and -P overrides identical to C++ wiring.
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
/// registered type between eicrecon._core and the binding modules.
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
  /// Convert a C++ config struct (of this factory's ConfigType) to a dict
  virtual py::dict config_to_dict(const void* config) const = 0;
  /// Validate config and create a generator; ownership passes to the caller (JApplication)
  virtual JFactoryGenerator* make_generator(JApplication* app, const std::string& plugin,
                                            const std::string& tag,
                                            const std::vector<std::string>& inputs,
                                            const std::vector<std::string>& outputs,
                                            const py::dict& config) const = 0;
};

/// All factory types bound in any loaded module, keyed by mangled C++ type name. Inline function
/// with a static local and default visibility: one instance across all extension modules.
struct FactoryTypeRegistry {
  std::mutex mutex;
  std::map<std::string, std::shared_ptr<FactoryTypeBase>> types;
};
inline FactoryTypeRegistry& factory_type_registry() {
  static FactoryTypeRegistry registry;
  return registry;
}

/// Overlay a dict onto a default-constructed config struct; raises Python exceptions
template <class ConfigT> ConfigT config_from_dict(const py::dict& d, const std::string& context) {
  ConfigT cfg{};
  try {
    struct_update(cfg, d, "", false);
  } catch (const ConversionError& e) {
    std::string msg = e.what();
    if (msg.find("unknown config field") != std::string::npos) {
      throw py::key_error(context + ": " + msg);
    }
    throw py::type_error(context + ": " + msg);
  }
  return cfg;
}

template <class FactoryT> class FactoryType : public FactoryTypeBase {
public:
  using ConfigT = typename FactoryT::ConfigType;

  explicit FactoryType(std::string name) : m_name(std::move(name)) {}

  std::string name() const override { return m_name; }
  std::string cpp_type() const override { return JTypeInfo::demangle<FactoryT>(); }

  py::dict config_defaults() const override { return struct_to_dict(ConfigT{}); }

  py::dict config_to_dict(const void* config) const override {
    return struct_to_dict(*static_cast<const ConfigT*>(config));
  }

  py::dict signature() const override {
    FactoryT probe; // inputs/outputs register themselves in the constructor
    // through the base: some factories declare members that hide m_inputs/m_outputs
    const ::JOmniFactory<FactoryT, ConfigT>& base = probe;
    py::list ins, outs;
    for (const auto* in : base.m_inputs) {
      ins.append(py::make_tuple(in->type_name, in->is_variadic));
    }
    for (const auto* out : base.m_outputs) {
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

/// Register FactoryT as attribute `name` of module m (and in the process-wide registry).
/// Templated factories are also reachable as m.types["<C++ type>"].
template <class FactoryT> void bind_factory(py::module_& m, const std::string& name) {
  auto& registry = factory_type_registry();
  std::shared_ptr<FactoryTypeBase> type;
  {
    std::lock_guard<std::mutex> lock(registry.mutex);
    auto& slot = registry.types[typeid(FactoryT).name()];
    if (!slot) {
      slot = std::make_shared<FactoryType<FactoryT>>(name);
    }
    type = slot;
  }
  if (!py::hasattr(m, "types")) {
    m.attr("types") = py::dict();
  }
  m.attr("types")[py::str(JTypeInfo::demangle<FactoryT>())] = py::cast(type);
  m.attr(name.c_str())                                      = py::cast(type);
}

} // namespace eicrecon::python
