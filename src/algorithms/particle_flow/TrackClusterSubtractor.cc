// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Derek Anderson

#include <edm4eic/Track.h>
#include <edm4eic/TrackPoint.h>
#include <edm4hep/utils/vector_utils.h>
#include <fmt/core.h>
#include <podio/ObjectID.h>
#include <podio/RelationRange.h>
#include <cmath>
#include <gsl/pointers>
#include <limits>
#include <map>
#include <vector>

#include "TrackClusterSubtractor.h"
#include "algorithms/interfaces/CompareObjectID.h"
#include "algorithms/particle_flow/TrackClusterSubtractorConfig.h"

namespace eicrecon {

/*! Subtract energy of matched tracks via the following algorithm.
 *    1. Build a map of each cluster onto a list of matched
 *       track projections.
 *    2. For each cluster, subtract the sum of momenta of
 *       all matched tracks scaled by the specified fraction
 *       from the cluster's energy.
 *    3. For each matched cluster:
 *       a. If subtracted energy is greater than zero, create
 *          a remnant cluster with the subtracted energy
 *       b. And create an expected cluster with energy equal
 *          to the difference between the total and the remnant.
 *    4. Flag any un-matched clusters as remnants.
 */
void TrackClusterSubtractor::process(const TrackClusterSubtractor::Input& input,
                                     const TrackClusterSubtractor::Output& output) const {

  // grab inputs/outputs
  const auto [in_matches, in_clusters, in_projections] = input;
  auto [out_remnants, out_expecteds, out_matches]     = output;

  // exit if no clusters in collection
  if (in_clusters->size() == 0) {
    debug("No clusters in collection");
    return;
  }

  // emit debugging message if no matched tracks in collection
  if (in_matches->size() == 0) {
    debug("No matched tracks in collection.");
  }

  // --------------------------------------------------------------------------
  // 1. Build map of clusters onto projections
  // --------------------------------------------------------------------------
  std::map<edm4eic::Cluster, segment_vector, CompareObjectID<edm4eic::Cluster>> mapClusterToProjections;
  for (const auto& match : *in_matches) {
    for (const auto& project : *in_projections) {

      // pick out corresponding projection from track
      if (match.getTrack() != project.getTrack()) {
        continue;
      } else {
        mapClusterToProjections[match.getCluster()].push_back(project);
      }

    } // end projection loop
  } // end track-cluster match loop
  debug("Built map of clusters-onto-tracks, size = {}", mapClusterToProjections.size());

  // now identify any clusters without matching tracks
  std::vector<edm4eic::Cluster> vecNoMatchClust;
  for (const auto& cluster : *in_clusters) {
    if (mapClusterToProjections.count(cluster) == 0) {
      vecNoMatchClust.push_back(cluster);
    }
  }
  debug("Built vector of unmatched clusters, size = {}", vecNoMatchClust.size());

  // --------------------------------------------------------------------------
  // 2. Subtract energy for tracks
  // --------------------------------------------------------------------------
  for (const auto& [cluster, projections] : mapClusterToProjections) {

    // do subtraction
    const double eToSubtract = m_cfg.energyFractionToSubtract * sum_track_energy(projections);
    const double eSubtracted = cluster.getEnergy() - eToSubtract;
    trace("Subtracted {} GeV from cluster with {} GeV", eToSubtract, cluster.getEnergy());

    // check if consistent with zero,
    // set eSub accordingly
    const bool isZero = is_zero(eSubtracted);
    const double eSubtractedToUse = isZero ? 0. : eSubtracted;

    // ------------------------------------------------------------------------
    // 3(a). If difference not consistent with zero, create output remnant
    // ------------------------------------------------------------------------
    if (!isZero) {
      auto remain_cluster = cluster.clone();
      remain_cluster.setEnergy(eSubtractedToUse);
      out_remnants->push_back(remain_cluster);
      trace("Created remnant cluster with {} GeV", remain_cluster.getEnergy());
    }

    // ------------------------------------------------------------------------
    // 3(b). Create cluster with energy equal to eTotal - eRemnant and match
    // ------------------------------------------------------------------------
    auto expect_cluster = cluster.clone();
    expect_cluster.setEnergy(cluster.getEnergy() - eSubtractedToUse);
    out_expecteds->push_back(expect_cluster);
    trace("Created subtracted cluster with {} GeV (originally {} GeV)", expect_cluster.getEnergy(),
          cluster.getEnergy());

    // create a track-cluster match for expected clusters
    for (const auto& project : projections) {
      edm4eic::MutableTrackClusterMatch match = out_matches->create();
      match.setCluster(expect_cluster);
      match.setTrack(project.getTrack());
      match.setWeight(1.0); // FIXME placeholder
      trace("Matched expected cluster {} to track {}", expect_cluster.getObjectID().index,
            project.getTrack().getObjectID().index);
    }
  } // end cluster-to-projections loop
  debug("Finished subtraction, {} remnant clusters and {} expected clusters", out_remnants->size(),
        out_expecteds->size());

  // --------------------------------------------------------------------------
  // 4. Any unmatched clusters are remnants by definition
  // --------------------------------------------------------------------------
  for (const auto& cluster : vecNoMatchClust) {
    auto remain_cluster = cluster.clone();
    out_remnants->push_back(remain_cluster);
  }
  debug("Finished copying unmatched clusters to remnants, {} remnant clusters",
        out_remnants->size());

} // end 'process(Input&, Output&)'

/*! Sums energy of tracks projected to the surface in the
 *  calorimeter specified by `surfaceToUse`. Uses PDG of
 *  track to select mass for energy; if not available,
 *  uses mass set by `defaultPDG`.
 */
double TrackClusterSubtractor::sum_track_energy(const segment_vector& projections) const {

  double eSum = 0.;
  for (const auto& project : projections) {

    // measure momentum at specified surface
    double momentum = 0.;
    for (const auto& point : project.getPoints()) {
      if (point.surface != m_cfg.surfaceToUse) {
        continue;
      } else {
        momentum = edm4hep::utils::magnitude(point.momentum);
        break;
      }
    }

    // get mass based on track pdg
    int pdgToUse = project.getTrack().getPdg();
    if (pdgToUse == 0) {
      pdgToUse = m_cfg.defaultPDG;
    }
    const double mass = m_parSvc.particle(pdgToUse).mass;

    // increment sum
    eSum += std::sqrt((momentum * momentum) + (mass * mass));
  }

  // output debugging and exit
  trace("Sum of track energy = {} GeV", eSum);
  return eSum;

} // end 'sum_track_energy(segment_vector&)'

/*! Checks if provided difference is consistent with zero,
 *  either checking if difference is within an epsilon
 *  (if `doNSigmaCut` is false), or if difference is within
 *  `nSigmaMax` of zero (if `doNSigmaCut` is true) based on
 *  the provided tracker and calorimeter resolutions.
 */
bool TrackClusterSubtractor::is_zero(const double difference) const {

  // if < 0, automatically return true
  if (difference < 0) {
    return true;
  }

  // do appropriate comparison
  bool isZero = false;
  if (m_cfg.doNSigmaCut) {

    // calculate n sigma squared
    const double resolution2 = (m_cfg.trackResolution * m_cfg.trackResolution) + (m_cfg.calorimeterResolution * m_cfg.calorimeterResolution);
    const uint32_t nSigma2 = static_cast<uint32_t>(std::floor((difference * difference) / resolution2));
    const uint32_t nSigmaMax2 = m_cfg.nSigmaMax * m_cfg.nSigmaMax;

    isZero = (nSigma2 < nSigmaMax2);
    trace("Difference of {} GeV consistent with zero: nSigma2 = {} < {}", difference, nSigma2,
          nSigmaMax2);
  } else {
    isZero = std::abs(difference) < std::numeric_limits<double>::epsilon();
    trace("Difference of {} GeV consistent with zero within an epsilon", difference);
  }
  return isZero;

} // end 'is_zero(double)'
} // namespace eicrecon
