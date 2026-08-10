// Copyright 2023, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.
// Created by Nathan Brei

#pragma once

#include <JANA/Components/JOmniFactoryGeneratorT.h>
#include <JANA/JApplicationFwd.h>

#include <string>
#include <utility>
#include <vector>

namespace eicrecon {

// Fallthrough to JANA's built-in JOmniFactoryGeneratorT, but allow for unused app argument in constructor
template <class FactoryT>
class JOmniFactoryGeneratorT : public jana::components::JOmniFactoryGeneratorT<FactoryT> {
public:
  using FactoryConfigType = typename FactoryT::ConfigType;
  using TypedWiring = typename jana::components::JOmniFactoryGeneratorT<FactoryT>::TypedWiring;

  explicit JOmniFactoryGeneratorT() = default;

  explicit JOmniFactoryGeneratorT(std::string tag, std::vector<std::string> input_names,
                                  std::vector<std::string> output_names, FactoryConfigType configs,
                                  JApplication* /* app */ = nullptr)
      : jana::components::JOmniFactoryGeneratorT<FactoryT>(tag, input_names, output_names,
                                                           configs) {}

  explicit JOmniFactoryGeneratorT(std::string tag, std::vector<std::string> input_names,
                                  std::vector<std::string> output_names,
                                  JApplication* /* app */ = nullptr)
      : jana::components::JOmniFactoryGeneratorT<FactoryT>(tag, input_names, output_names) {}

  explicit JOmniFactoryGeneratorT(TypedWiring&& wiring)
      : jana::components::JOmniFactoryGeneratorT<FactoryT>(std::move(wiring)) {}
};

} // namespace eicrecon
