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
#include "algorithms/particle/TrackClusterSubtractorConfig.h"

namespace eicrecon {

// ----------------------------------------------------------------------------
//! Process inputs
// ----------------------------------------------------------------------------
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
  const auto [in_match, in_cluster, in_project] = input;
  auto [out_remnant, out_expect, out_match] = output;

  // exit if no clusters in collection
  if (in_cluster->size() == 0) {
    debug("No clusters in collection");
    return;
  }

  // emit debugging message if no matched tracks in collection
  if (in_match->size() == 0) {
    debug("No matched tracks in collection.");
  }

  // --------------------------------------------------------------------------
  // 1. Build map of clusters onto projections
  // --------------------------------------------------------------------------
  MapToVecSeg mapClustToProj;
  for (const auto& match : *in_match) {
    for (const auto& project : *in_project) {

      // pick out corresponding projection from track
      if (match.getTrack() != project.getTrack()) {
        continue;
      } else {
        mapClustToProj[match.getCluster()].push_back(project);
      }

    } // end projection loop
  } // end track-cluster match loop
  debug("Built map of clusters-onto-tracks, size = {}", mapClustToProj.size());

  // now identify any clusters without matching tracks
  VecClust vecNoMatchClust;
  for (const auto& cluster : *in_cluster) {
    if (mapClustToProj.count(cluster) == 0) {
      vecNoMatchClust.push_back(cluster);
    }
  }
  debug("Built vector of unmatched clusters, size = {}", vecNoMatchClust.size());

  // --------------------------------------------------------------------------
  // 2. Subtract energy for tracks
  // --------------------------------------------------------------------------
  for (const auto& [cluster, projects] : mapClustToProj) {

    // do subtraction
    const double eToSub = m_cfg.fracEnergyToSub * sum_track_energy(projects);
    const double eSub = cluster.getEnergy() - eToSub;
    trace("Subtracted {} GeV from cluster with {} GeV", eToSub, cluster.getEnergy());

    // check if consistent with zero,
    // set eSub accordingly
    const bool isZero      = is_zero(eSub);
    const double eSubToUse = isZero ? 0. : eSub;

    // ------------------------------------------------------------------------
    // 3(a). If difference not consistent with zero, create output remnant
    // ------------------------------------------------------------------------
    if (!isZero) {
      auto remain_clust = cluster.clone();
      remain_clust.setEnergy(eSubToUse);
      out_remnant->push_back(remain_clust);
      trace("Created remnant cluster with {} GeV", remain_clust.getEnergy());
    }

    // ------------------------------------------------------------------------
    // 3(b). Create cluster with energy equal to eTotal - eRemnant and match
    // ------------------------------------------------------------------------
    auto expect_clust = cluster.clone();
    expect_clust.setEnergy(cluster.getEnergy() - eSubToUse);
    out_expect->push_back(expect_clust);
    trace("Created subtracted cluster with {} GeV (originally {} GeV)",
          expect_clust.getEnergy(),
          cluster.getEnergy());

    // create a track-cluster match for expected clusters
    for (const auto& project : projects) {
      edm4eic::MutableTrackClusterMatch match = out_match->create();
      match.setCluster(expect_clust);
      match.setTrack(project.getTrack());
      match.setWeight(1.0); // FIXME placeholder
      trace("Matched expected cluster {} to track {}",
            expect_clust.getObjectID().index,
            project.getTrack().getObjectID().index);
    }

  } // end cluster-to-projections loop
  debug("Finished subtraction, {} remnant clusters and {} expected clusters",
        out_remnant->size(),
        out_expect->size());

  // --------------------------------------------------------------------------
  // 4. Any unmatched clusters are remnants by definition
  // --------------------------------------------------------------------------
  for (const auto& cluster : vecNoMatchClust) {
    auto remain_clust = cluster.clone();
    out_remnant->push_back(remain_clust);
  }
  debug("Finished copying unmatched clusters to remnants, {} remnant clusters", out_remnant->size());

} // end 'process(Input&, Output&)'

// ----------------------------------------------------------------------------
//! Sum energy of tracks
// ----------------------------------------------------------------------------
/*! Sums energy of tracks projected to the surface in the
 *  calorimeter specified by `surfaceToUse`. Uses PDG of
 *  track to select mass for energy; if not available,
 *  uses mass set by `defaultMassPdg`.
 */
double TrackClusterSubtractor::sum_track_energy(const VecSeg& projects) const {

  double eSum = 0.;
  for (const auto& project : projects) {

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
    double mass = m_parSvc.particle(m_cfg.defaultMassPdg).mass;
    if (project.getTrack().getPdg() != 0) {
      mass = m_parSvc.particle(project.getTrack().getPdg()).mass;
    }

    // increment sum
    eSum += std::sqrt((momentum * momentum) + (mass * mass));
  }

  // output debugging and exit
  trace("Sum of track energy = {} GeV", eSum);
  return eSum;

} // end 'sum_track_energy(VecSeg&)'

// --------------------------------------------------------------------------
//! Is difference consistent with zero?
// --------------------------------------------------------------------------
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

  // calculate nSigma
  const double totReso =
      std::sqrt((m_cfg.trkReso * m_cfg.trkReso) + (m_cfg.calReso * m_cfg.calReso));
  const double nSigma = difference / totReso;

  // do appropriate comparison
  bool isZero = false;
  if (m_cfg.doNSigmaCut) {
    isZero = (nSigma < m_cfg.nSigmaMax);
    trace("Difference of {} GeV consistent with zero: nSigma = {} < {}", difference, nSigma,
          m_cfg.nSigmaMax);
  } else {
    isZero = std::abs(difference) < std::numeric_limits<double>::epsilon();
    trace("Difference of {} GeV consistent with zero within an epsilon", difference);
  }
  return isZero;

} // end 'is_zero(double)'

} // namespace eicrecon
