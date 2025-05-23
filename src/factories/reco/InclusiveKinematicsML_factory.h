// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Wouter Deconinck

#pragma once

#include <JANA/JEvent.h>
#include <edm4eic/InclusiveKinematicsCollection.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "algorithms/onnx/InclusiveKinematicsML.h"
#include "extensions/jana/JOmniFactory.h"

namespace eicrecon {

class InclusiveKinematicsML_factory
    : public JOmniFactory<InclusiveKinematicsML_factory, InclusiveKinematicsMLConfig> {

public:
  using AlgoT = eicrecon::InclusiveKinematicsML;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4eic::InclusiveKinematics> m_inclusive_kinematics_electron_input{this};
  PodioInput<edm4eic::InclusiveKinematics> m_inclusive_kinematics_da_input{this};
  PodioOutput<edm4eic::InclusiveKinematics> m_inclusive_kinematics_output{this};

  ParameterRef<std::string> m_modelPath{this, "modelPath", config().modelPath};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void ChangeRun(int32_t /* run_number */) {}

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_inclusive_kinematics_electron_input(), m_inclusive_kinematics_da_input()},
                    {m_inclusive_kinematics_output().get()});
  }
};

} // namespace eicrecon
