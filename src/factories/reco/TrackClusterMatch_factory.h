// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Tristan Protzman

#pragma once

#include <spdlog/logger.h>
#include "extensions/jana/JOmniAlgorithmFactory.h"

#include "algorithms/reco/TrackClusterMatch.h"
#include "algorithms/reco/TrackClusterMatchConfig.h"
#include "services/geometry/dd4hep/DD4hep_service.h"

namespace eicrecon {

class TrackClusterMatch_factory
    : public JOmniAlgorithmFactory<TrackClusterMatch_factory, eicrecon::TrackClusterMatch,
                                   TrackClusterMatchConfig> {
private:
  // Declare parameters
  ParameterRef<double> m_matching_distance{this, "matchingDistance", config().matching_distance};
};
} // namespace eicrecon
