// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Tristan Protzman

#include <edm4eic/Track.h>
#include <fmt/core.h>
#include <podio/RelationRange.h>
#include <cstdint>
#include <gsl/pointers>
#include <optional>
#include <set>
#include <stdexcept>
#include <vector>

#include <DD4hep/Detector.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/TrackPoint.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>

#include "algorithms/reco/TrackClusterMatch.h"
#include "algorithms/reco/TrackClusterMatchConfig.h"

namespace eicrecon {
void TrackClusterMatch::process(const TrackClusterMatch::Input& input,
                                const TrackClusterMatch::Output& output) const {
  auto [tracks, clusters]  = input;
  auto [matched_particles] = output;
  trace("We have {} tracks and {} clusters", tracks->size(), clusters->size());

  // Validate the configuration
  if (m_cfg.matching_distance <= 0) {
    throw std::runtime_error(fmt::format("Invalid matching distance: {}", m_cfg.matching_distance));
  }
  if (m_cfg.calo_id.empty()) {
    throw std::runtime_error("Calorimeter ID must be set in the configuration");
  }

  std::set<int> used_tracks;
  // Loop across each cluster, and find the cloeset projected track
  for (auto cluster : *clusters) {
    const double cluster_eta = edm4hep::utils::eta(cluster.getPosition());
    const double cluster_phi = edm4hep::utils::angleAzimuthal(cluster.getPosition());
    trace("Cluster at eta={}, phi={}", cluster_eta, cluster_phi);
    // TODO: Get the detector ID so I can check if a projection is in the appropriate calorimeter

    std::optional<edm4eic::TrackSegment> closest_segment;
    int closest_segment_id  = -1;
    double closest_distance = m_cfg.matching_distance;
    // Loop through each track segment, and its points
    for (int closest_id = 0; auto track : *tracks) {
      if (used_tracks.contains(closest_id)) {
        trace("Skipping track segment already used");
        continue;
      }
      for (auto point : track.getPoints()) {
        // Check if the point is at the calorimeter
        // int id = m_detector->volumeManager().lookupDetector(cluster.getHits()[0].getCellID()).id(); // TODO: Find programmatic way to get detector cluster is from
        uint32_t calo_id = m_geo.detector()->constant<int>(m_cfg.calo_id);
        bool is_calo     = point.system == calo_id;
        bool is_surface  = point.surface == 1;

        if (!is_calo || !is_surface) {
          trace("Skipping track point not at the calorimeter");
          continue;
        }
        const double track_eta = edm4hep::utils::eta(point.position);
        const double track_phi = edm4hep::utils::angleAzimuthal(point.position);
        trace("Track point at eta={}, phi={}", track_eta, track_phi);

        double delta = distance(cluster.getPosition(), point.position);
        trace("Distance between cluster and track point: {}", delta);
        if (delta < closest_distance) {
          closest_distance   = delta;
          closest_segment    = track;
          closest_segment_id = closest_id;
        }
      }
      closest_id++;
    }
    // If we found a point, create a new particle
    if (closest_segment) {
      trace("Found a closest point at distance {}", closest_distance);
      auto particle = matched_particles->create();
      particle.setCluster(cluster);
      particle.setTrack(closest_segment.value().getTrack());
      used_tracks.insert(closest_segment_id);
    }
  }
  trace("Matched {} particles", matched_particles->size());
}

double TrackClusterMatch::distance(const edm4hep::Vector3f& v1, const edm4hep::Vector3f& v2) {
  double cluster_eta = edm4hep::utils::eta(v1);
  double cluster_phi = edm4hep::utils::angleAzimuthal(v1);
  double track_eta   = edm4hep::utils::eta(v2);
  double track_phi   = edm4hep::utils::angleAzimuthal(v2);
  double deta        = cluster_eta - track_eta;
  double dphi        = Phi_mpi_pi(cluster_phi - track_phi);
  return std::hypot(deta, dphi);
}
} // namespace eicrecon
