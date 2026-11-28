// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck
//
// Optional observer of the wirings that JOmniFactoryGeneratorT turns into factories.
//
// When an observer is installed (by the Python frontend, to compare the wirings of the C++
// plugins with those defined in Python), JOmniFactoryGeneratorT::GenerateFactories reports each
// wiring with its effective plugin name, tag, collection names and config struct. Without an
// observer this costs one pointer check per generated factory set.

#pragma once

#include <functional>
#include <string>
#include <typeinfo>
#include <vector>

namespace eicrecon {

struct JOmniFactoryWiringRecord {
  std::string plugin;
  std::string tag;
  const std::type_info* factory_type      = nullptr;
  const std::vector<std::string>* inputs  = nullptr;
  const std::vector<std::string>* outputs = nullptr;
  const void* config                      = nullptr; // points to a FactoryT::ConfigType
  const void* generator                   = nullptr; // identifies the generator
};

using JOmniFactoryObserver = std::function<void(const JOmniFactoryWiringRecord&)>;

/// The installed observer, or nullptr. An inline variable with default visibility, so that all
/// plugins and the Python extension modules share one instance.
inline JOmniFactoryObserver* g_omnifactory_observer = nullptr;

} // namespace eicrecon
