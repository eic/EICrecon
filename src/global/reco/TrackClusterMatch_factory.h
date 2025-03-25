// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Tristan Protzman


#pragma once

#include <spdlog/logger.h>
#include "extensions/jana/JOmniFactory.h"

#include "algorithms/reco/TrackClusterMatch.h"
#include "algorithms/reco/TrackClusterMatchConfig.h"
#include "services/geometry/dd4hep/DD4hep_service.h"



namespace eicrecon {
    class TrackClusterMatch_factory : public JOmniFactory<TrackClusterMatch_factory, TrackClusterMatchConfig> {
    private:
        // Underlying algorithm
        std::unique_ptr<eicrecon::TrackClusterMatch> m_algo;
        
        // Declare inputs
        PodioInput<edm4eic::TrackSegment> m_tracks {this};
        PodioInput<edm4eic::Cluster> m_clusters {this};
        
        // Declare outputs
        PodioOutput<edm4eic::TrackClusterMatch> m_matched_particles {this};
        
        // Declare parameters
        ParameterRef<double> m_matching_distance {this, "matchingDistance", config().matching_distance};
        
        // Helpers
        std::shared_ptr<spdlog::logger> m_log;
        Service<DD4hep_service> m_GeoSvc {this};
        const dd4hep::Detector* m_detector;
        
    public:
        void Configure() {
            m_algo = std::make_unique<eicrecon::TrackClusterMatch>(GetPrefix());
            m_log = logger();
            m_detector = m_GeoSvc().detector();
            m_algo->init(m_log, m_detector);
            m_log->trace("Configured TrackClusterMatch with matching distance {}", m_matching_distance());
        }
        
        void ChangeRun(int64_t run_number) {}
        
        void Process(int64_t run_number, uint64_t event_number) {
            m_log->trace("Processing event {} in run {}", event_number, run_number);
            m_algo->execute({m_tracks(), m_clusters()}, {m_matched_particles().get()});
            m_log->trace("Matched {} particles", m_matched_particles()->size());
        }
    };
} 
    