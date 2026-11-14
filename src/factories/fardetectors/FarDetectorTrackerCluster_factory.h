// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 - 2025, Simon Gardner

#pragma once

#include "algorithms/fardetectors/FarDetectorTrackerCluster.h"
#include "services/geometry/dd4hep/DD4hep_service.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

class FarDetectorTrackerCluster_factory
    : public JOmniAlgorithmFactory<FarDetectorTrackerCluster_factory,
                                   eicrecon::FarDetectorTrackerCluster,
                                   FarDetectorTrackerClusterConfig> {
private:
  ParameterRef<std::string> m_readout{this, "readoutClass", config().readout};
  ParameterRef<std::string> m_x_field{this, "xField", config().x_field};
  ParameterRef<std::string> m_y_field{this, "yField", config().y_field};
  ParameterRef<double> m_hit_time_limit{this, "hitTimeLimit", config().hit_time_limit};
};

} // namespace eicrecon
