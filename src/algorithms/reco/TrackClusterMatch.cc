// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Tristan Protzman

#include "algorithms/reco/TrackClusterMatch.h"
#include "algorithms/reco/TrackClusterMatchConfig.h"
#include <spdlog/logger.h>

#include <edm4eic/TrackCollection.h>
#include <edm4eic/ClusterCollection.h>

namespace eicrecon {
    void TrackClusterMatch::init(std::shared_ptr<spdlog::logger> logger) {
        m_log = logger;
    }
    
    void TrackClusterMatch::execute(const TrackClusterMatch::Input& input, const TrackClusterMatch::Output& output) const {
        auto [tracks, clusters] = input;
        auto [matched_particles] = output;
        m_log->trace("We have {} tracks and {} clusters", tracks->size(), clusters->size());
    }
} 