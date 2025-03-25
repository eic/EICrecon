// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Tristan Protzman

#include "algorithms/reco/TrackClusterMatch.h"
#include "algorithms/reco/TrackClusterMatchConfig.h"
#include <spdlog/logger.h>

#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>

#include <edm4eic/Cluster.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/Track.h>
#include <edm4eic/TrackCollection.h>
#include <edm4eic/TrackPoint.h>
#include <edm4eic/TrackSegment.h>

#include <DD4hep/Detector.h>

namespace eicrecon {
    void TrackClusterMatch::init(std::shared_ptr<spdlog::logger> logger, const dd4hep::Detector* detector) {
        m_log = logger;
        m_detector = detector;
    }
    
    void TrackClusterMatch::execute(const TrackClusterMatch::Input& input, const TrackClusterMatch::Output& output) const {
        auto [tracks, clusters] = input;
        auto [matched_particles] = output;
        m_log->trace("We have {} tracks and {} clusters", tracks->size(), clusters->size());
        
        std::set<int> used_tracks;
        // Loop across each cluster, and find the cloeset projected track
        for (auto cluster : *clusters) {
            const double cluster_eta = edm4hep::utils::eta(cluster.getPosition());
            const double cluster_phi = edm4hep::utils::angleAzimuthal(cluster.getPosition());
            m_log->trace("Cluster at eta={}, phi={}", cluster_eta, cluster_phi);
            // TODO: Get the detector ID so I can check if a projection is in the appropriate calorimeter

            std::optional<edm4eic::TrackSegment> closest_segment;
            int closest_segment_id = -1;
            double closest_distance = m_cfg.matching_distance;
            // Loop through each track segment, and its points
            for (int closest_id = 0; auto track : *tracks) {
                if (used_tracks.count(closest_id) > 0) {
                    m_log->trace("Skipping track segment already used");
                    continue;
                }
                for (auto point : track.getPoints()) {
                    // Check if the point is at the calorimeter
                    int id = m_detector->volumeManager().lookupDetector(cluster.getHits()[0].getCellID()).id();
                    // m_log->trace("Track point at detector ID {}", id);
                    // m_log->trace("EcalBarrel ID is {}", m_detector->constant<uint32_t>("EcalBarrel_ID"));
                    // m_log->trace("HcalBarrel ID is {}", m_detector->constant<uint32_t>("HcalBarrel_ID"));
                    uint32_t ecal_barrel_id = 101;
                    uint32_t hcal_barrel_id = 111;
                    if (!((point.system == ecal_barrel_id) || (point.system == hcal_barrel_id)) ||  point.surface != 1) { // Just try the barrel ECal for now...
                        m_log->trace("Skipping track point not at the calorimeter");
                        continue;
                    }
                    const double track_eta = edm4hep::utils::eta(point.position);
                    const double track_phi = edm4hep::utils::angleAzimuthal(point.position);
                    m_log->trace("Track point at eta={}, phi={}", track_eta, track_phi);

                    double delta = distance(cluster.getPosition(), point.position);
                    m_log->trace("Distance between cluster and track point: {}", delta);
                    if (delta < closest_distance) {
                        closest_distance = delta;
                        closest_segment = track;
                        closest_segment_id = closest_id;
                    }
                }
                closest_id++;
            }
            // If we found a point, create a new particle
            if (closest_segment) {
                m_log->trace("Found a closest point at distance {}", closest_distance);
                auto particle = matched_particles->create();
                particle.setCluster(cluster);
                particle.setTrack(closest_segment.value().getTrack());
                used_tracks.insert(closest_segment_id);
                // compare object ID of track from track segment to track in output collection
            }
        }
    }

    double TrackClusterMatch::distance(const edm4hep::Vector3f& v1, const edm4hep::Vector3f& v2) const {
        double cluster_eta = edm4hep::utils::eta(v1);
        double cluster_phi = edm4hep::utils::angleAzimuthal(v1);
        double track_eta = edm4hep::utils::eta(v2);
        double track_phi = edm4hep::utils::angleAzimuthal(v2);
        double deta = cluster_eta - track_eta;
        double dphi = cluster_phi - track_phi;
        return std::hypot(deta, dphi);
    }
} 