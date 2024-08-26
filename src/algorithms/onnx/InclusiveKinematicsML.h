// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022, 2023 Sylvester Joosten, Dmitry Romanov, Wouter Deconinck

#pragma once

#include <algorithms/algorithm.h>
#include <cstdint>
#include <edm4eic/InclusiveKinematicsCollection.h>
#include <onnxruntime_cxx_api.h>
#include <string>
#include <string_view>
#include <vector>

#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/onnx/InclusiveKinematicsMLConfig.h"

namespace eicrecon {

using InclusiveKinematicsMLAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4eic::InclusiveKinematicsCollection,
                                            edm4eic::InclusiveKinematicsCollection>,
                          algorithms::Output<edm4eic::InclusiveKinematicsCollection>>;

class InclusiveKinematicsML : public InclusiveKinematicsMLAlgorithm,
                              public WithPodConfig<InclusiveKinematicsMLConfig> {

public:
  InclusiveKinematicsML(std::string_view name)
      : InclusiveKinematicsMLAlgorithm{name,
                                       {"inclusiveKinematicsElectron", "inclusiveKinematicsDA"},
                                       {"inclusiveKinematicsML"},
                                       "Determine inclusive kinematics using combined ML method."} {
  }

  void init() final;
  void process(const Input&, const Output&) const final;

private:
  mutable Ort::Env m_env{nullptr};
  mutable Ort::Session m_session{nullptr};

  std::vector<std::string> m_input_names;
  std::vector<const char*> m_input_names_char;
  std::vector<std::vector<std::int64_t>> m_input_shapes;

  std::vector<std::string> m_output_names;
  std::vector<const char*> m_output_names_char;
  std::vector<std::vector<std::int64_t>> m_output_shapes;
};

} // namespace eicrecon
