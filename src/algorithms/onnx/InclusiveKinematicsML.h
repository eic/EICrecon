// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022, 2023 Sylvester Joosten, Dmitry Romanov, Wouter Deconinck

#pragma once

#include <algorithms/algorithm.h>
#include <edm4eic/InclusiveKinematicsCollection.h>
#include <spdlog/logger.h>
#include <memory>
#include <string>
#include <string_view>

namespace eicrecon {

  using InclusiveKinematicsMLAlgorithm = algorithms::Algorithm<
    algorithms::Input<
      edm4eic::InclusiveKinematicsCollection,
      edm4eic::InclusiveKinematicsCollection
    >,
    algorithms::Output<
      edm4eic::InclusiveKinematicsCollection
    >
  >;

  class InclusiveKinematicsML
  : public InclusiveKinematicsMLAlgorithm {

  public:
    InclusiveKinematicsML(std::string_view name)
      : InclusiveKinematicsMLAlgorithm{name,
                            {"inclusiveKinematicsElectron", "inclusiveKinematicsDA"},
                            {"inclusiveKinematicsML"},
                            "Determine inclusive kinematics using combined ML method."} {}

    void init(std::shared_ptr<spdlog::logger>& logger); 
    void process(const Input&, const Output&) const final;

  private:
    std::shared_ptr<spdlog::logger> m_log;
  };

} // namespace eicrecon
