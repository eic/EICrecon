// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Derek Anderson

#include <edm4eic/Cov3f.h>
#include <edm4eic/Track.h>
#include <edm4eic/TrackPoint.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <podio/ObjectID.h>
#include <podio/RelationRange.h>
#include <podio/detail/Link.h>
#include <podio/detail/LinkCollectionImpl.h>
#include <cstdint>
#include <cmath>
#include <cstddef>
#include <limits>
#include <map>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>

#include "TrackClusterSubtractor.h"
#include "algorithms/interfaces/CompareObjectID.h"
#include "algorithms/particle_flow/TrackClusterSubtractorConfig.h"

namespace eicrecon {

/*! Subtract energy of matched tracks via the following algorithm.
 *
 *    1. Build a map of each cluster onto a list of matched
 *       track projections.
 *    2. Flag any un-matched clusters as remnants.
 *    3. For each cluster, subtract the sum of momenta of
 *       all matched tracks scaled by the specified fraction
 *       from the cluster's energy.
 *    4. For each matched cluster:
 *       a. If subtracted energy is greater than zero, create
 *          a remnant cluster with the subtracted energy
 *       b. And create an expected cluster with energy equal
 *          to the difference between the total and the remnant.
 */
void TrackClusterSubtractor::process(const TrackClusterSubtractor::Input& input,
                                     const TrackClusterSubtractor::Output& output) const {

  // grab inputs/outputs
  const auto [in_matches, in_clusters, in_projections] = input;
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
  auto [out_remnants, out_expectants, out_links, out_matches] = output;
#else
  auto [out_remnants, out_expectants, out_matches] = output;
#endif

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
  std::map<edm4eic::Cluster, segment_vector, CompareObjectID<edm4eic::Cluster>>
      mapClusterToProjections;
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

  // --------------------------------------------------------------------------
  // 2. Any unmatched clusters are remnants by definition
  // --------------------------------------------------------------------------
  for (const auto& cluster : *in_clusters) {
    if (mapClusterToProjections.count(cluster) == 0) {
      auto remain_cluster = cluster.clone();
      out_remnants->push_back(remain_cluster);
    }
  }
  debug("Finished copying unmatched clusters to remnants, {} remnant clusters",
        out_remnants->size());

  // --------------------------------------------------------------------------
  // 3. Subtract energy for tracks
  // --------------------------------------------------------------------------
  for (const auto& [cluster, projections] : mapClusterToProjections) {

    // do subtraction
    const double eTrackSum = sum_track_energy(projections);
    const double eToSubtract = m_cfg.energyFractionToSubtract * eTrackSum;
    const double eSubtracted = cluster.getEnergy() - eToSubtract;
    trace("Subtracted {} GeV from cluster with {} GeV", eToSubtract, cluster.getEnergy());

    // if track sum is NOT greater than calorimeter energy within
    // tolerances, set remainder to nonzero value
    const bool isGreaterThan = is_track_energy_greater_than_calo(eSubtracted);
    const double eSubtractedToUse = isGreaterThan ? 0. : eSubtracted;

    // ------------------------------------------------------------------------
    // 4(a). If track energy not greater than calo, create output remnant
    // ------------------------------------------------------------------------
    if (!isGreaterThan) {
      auto remain_cluster = cluster.clone();
      remain_cluster.setEnergy(eSubtractedToUse);
      out_remnants->push_back(remain_cluster);
      trace("Created remnant cluster with {} GeV", remain_cluster.getEnergy());
    }

    // ------------------------------------------------------------------------
    // 4(b). Create cluster with energy equal to eTotal - eRemnant and match
    // ------------------------------------------------------------------------
    auto expect_cluster = cluster.clone();
    expect_cluster.setEnergy(cluster.getEnergy() - eSubtractedToUse);
    out_expectants->push_back(expect_cluster);
    trace("Created expected cluster with {} GeV (originally {} GeV)", expect_cluster.getEnergy(),
          cluster.getEnergy());

    // create a track-cluster match for expected clusters
    for (const auto& project : projections) {
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
      edm4eic::MutableTrackClusterLink link = out_links->create();
      link.setFrom(expect_cluster);
      link.setTo(project.getTrack());
      link.setWeight(1.0); // FIXME placeholder
      trace("Matched expected cluster {} to track {}", expect_cluster.getObjectID().index,
            project.getTrack().getObjectID().index);
#endif

      edm4eic::MutableTrackClusterMatch match = out_matches->create();
      match.setCluster(expect_cluster);
      match.setTrack(project.getTrack());
      match.setWeight(1.0); // FIXME placeholder
      trace("Matched expected cluster {} to track {}", expect_cluster.getObjectID().index,
            project.getTrack().getObjectID().index);
    }
  } // end cluster-to-projections loop
  debug("Finished subtraction, {} remnant clusters and {} expected clusters", out_remnants->size(),
        out_expectants->size());

} // end 'process(Input&, Output&)'

/*! Sums energy of tracks projected to the surface in the calorimeter
 *  specified by `surfaceToUse`. Uses PDG of track to select mass
 *  for energy; if not available, uses mass set by `defaultPDG`.
 *
 *  @param[in] projections vector of projections matched to
 *                         a cluster
 *
 *  @return Sum of energy
 */
double TrackClusterSubtractor::sum_track_energy(const segment_vector& projections) const {

  double energySum = 0.0;
  for (const auto& project : projections) {

    // measure momentum at specified surface
    double momentum = 0.0;
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
    energySum += std::sqrt((momentum * momentum) + (mass * mass));
  }

  trace("Sum of track energy = {} GeV", energySum);
  return energySum;

} // end 'sum_track_energy(segment_vector&)'

/*! Checks if the sum of tracks' energy is greater than a calorimeter cluster
 *  energy by checking if their difference is either:
 *
 *    1. smaller than than an epsilon (if doNSigmaCut is false), or
 *    2. within nSigmaMax of zero (if doNSigmaCut is true) based on
 *       the computed sum of tracks' variance and the calorimeter
 *       resolution.
 *
 *  \param[in] difference energy difference between the cluster and
 *                        sum of tracks
 *  \param[in] variance   the variance on the sume of track energy
 */
bool TrackClusterSubtractor::is_track_energy_greater_than_calo(const double difference) const {

  // if < 0, automatically return true
  if (difference < 0) {
    return true;
  }

  // do appropriate comparison
  bool isGreaterThan = false;
  if (m_cfg.doNSigmaCut) {

    // calculate n sigma squared
    const double totalVariance =
        (m_cfg.trackResolution * m_cfg.trackResolution) + (m_cfg.calorimeterResolution * m_cfg.calorimeterResolution);
    const uint32_t nSigma2 =
        static_cast<uint32_t>(std::floor((difference * difference) / totalVariance));
    const uint32_t nSigmaMax2 = m_cfg.nSigmaMax * m_cfg.nSigmaMax;

    isGreaterThan = (nSigma2 < nSigmaMax2);
    if (isGreaterThan) {
      trace("Within {} NSigma^2, track energy sum greater than calorimeter cluster: difference = {} "
            "GeV, nSigma^2 = {}",
            nSigmaMax2, difference, nSigma2);
    }
  } else {
    isGreaterThan = difference < std::numeric_limits<double>::epsilon();
    trace("Track energy sum greater than calorimeter cluster: difference = {} GeV", difference);
  }
  return isGreaterThan;

} // end 'is_track_energy_greater_than_calo(double)'
} // namespace eicrecon
