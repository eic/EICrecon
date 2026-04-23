// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Tristan Protzman

#include <Eigen/Core>
#include <edm4eic/Track.h>
#include <fmt/core.h>
#include <podio/RelationRange.h>
#include <cmath>
#include <cstdint>
#include <gsl/pointers>
#include <limits>
#include <optional>
#include <stdexcept>
#include <vector>

#include <DD4hep/Detector.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/TrackPoint.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>

#include "algorithms/reco/HungarianAlgorithm.h"
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

  const int num_clusters = clusters->size();
  const int num_tracks   = tracks->size();

  if (num_clusters == 0 || num_tracks == 0) {
    trace("No matches possible: {} clusters, {} tracks", num_clusters, num_tracks);
    return;
  }

  // Build vectors of valid track segments with projection points at the calorimeter
  std::vector<edm4eic::TrackSegment> valid_tracks;
  std::vector<edm4hep::Vector3f> track_positions;

  uint32_t calo_id = m_geo.detector()->constant<int>(m_cfg.calo_id);

  for (auto track : *tracks) {
    std::optional<edm4hep::Vector3f> best_position;

    // Find the track point at the calorimeter surface
    for (auto point : track.getPoints()) {
      bool is_calo    = point.system == calo_id;
      bool is_surface = point.surface == 1;

      if (is_calo && is_surface) {
        best_position = point.position;
        break; // Use first matching point
      }
    }

    if (best_position) {
      valid_tracks.push_back(track);
      track_positions.push_back(*best_position);
    }
  }

  const int num_valid_tracks = valid_tracks.size();

  if (num_valid_tracks == 0) {
    trace("No valid track projections found at calorimeter");
    return;
  }

  trace("Found {} valid track projections at calorimeter", num_valid_tracks);

  // Build cost matrix: rows = clusters, cols = tracks
  // Use heuristic filtering to avoid expensive distance calculations for far-away pairs
  Eigen::MatrixXd cost_matrix(num_clusters, num_valid_tracks);

  // Use a generous fast-check threshold (e.g., 3x matching distance)
  // to quickly filter out pairs that are obviously too far
  const double fast_check_threshold = 3.0 * m_cfg.matching_distance;

  for (int i = 0; i < num_clusters; i++) {
    auto cluster                         = (*clusters)[i];
    const edm4hep::Vector3f& cluster_pos = cluster.getPosition();

    for (int j = 0; j < num_valid_tracks; j++) {
      // Fast preliminary check using simple coordinate differences
      const edm4hep::Vector3f& track_pos = track_positions[j];
      double dx                          = cluster_pos.x - track_pos.x;
      double dy                          = cluster_pos.y - track_pos.y;
      double dz                          = cluster_pos.z - track_pos.z;
      double fast_dist                   = std::sqrt(dx * dx + dy * dy + dz * dz);

      // Only compute expensive eta/phi distance if fast check passes
      if (fast_dist < fast_check_threshold) {
        cost_matrix(i, j) = distance(cluster_pos, track_pos);
      } else {
        // Set to large value for pairs that are obviously too far
        cost_matrix(i, j) = std::numeric_limits<double>::infinity();
      }
    }
  }

  trace("Built cost matrix of size {}x{}", num_clusters, num_valid_tracks);

  // Solve the assignment problem using Hungarian algorithm
  std::vector<int> assignment = HungarianAlgorithm::solve(cost_matrix);

  trace("Hungarian algorithm completed");

  // Create matches for assignments that meet the distance threshold
  for (int i = 0; i < num_clusters; i++) {
    int track_idx = assignment[i];

    if (track_idx == -1) {
      trace("Cluster {} unmatched", i);
      continue;
    }

    double match_distance = cost_matrix(i, track_idx);

    if (match_distance <= m_cfg.matching_distance) {
      auto cluster = (*clusters)[i];
      auto track   = valid_tracks[track_idx];

      auto particle = matched_particles->create();
      particle.setCluster(cluster);
      particle.setTrack(track.getTrack());

      trace("Matched cluster {} to track {} with distance {}", i, track_idx, match_distance);
    } else {
      trace("Cluster {} matched to track {} but distance {} exceeds threshold {}", i, track_idx,
            match_distance, m_cfg.matching_distance);
    }
  }

  trace("Created {} matched particles", matched_particles->size());
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
