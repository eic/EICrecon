// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Sebouh Paul

// An algorithm for producing a single cluster with a log-energy-weighted position from a collection of hits.  
//
// Author: Sebouh Paul
// Date: 12/04/2023


#pragma once
#include <DD4hep/Detector.h>
#include <DD4hep/IDDescriptor.h>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4eic/ProtoClusterCollection.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <fmt/core.h>
#include <spdlog/logger.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <exception>
#include <functional>
#include <memory>
#include <set>
#include <vector>

#include "algorithms/interfaces/WithPodConfig.h"
#include "LogWeightRecoConfig.h"

namespace eicrecon {

  class LogWeightReco : public WithPodConfig<LogWeightRecoConfig> {

  public:
    void init(const dd4hep::Detector* detector, std::shared_ptr<spdlog::logger>& logger);
     std::unique_ptr<edm4eic::ClusterCollection> process(const edm4eic::ProtoClusterCollection &proto) ;
    
  private:
    const dd4hep::Detector* m_detector;
    std::shared_ptr<spdlog::logger> m_log;

  };

} // namespace eicrecon
