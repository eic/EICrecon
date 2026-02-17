// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Derek Anderson, Dmitry Kalinkin

#include <edm4hep/MCParticle.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <fmt/core.h>
#include <podio/ObjectID.h>
#include <podio/RelationRange.h>
#include <cmath>
#include <cstddef>
#include <gsl/pointers>

#include "TrackClusterMergeSplitter.h"
#include "algorithms/calorimetry/TrackClusterMergeSplitterConfig.h"

namespace eicrecon {

// --------------------------------------------------------------------------
//! Process inputs
// --------------------------------------------------------------------------
/*! Merges and splits clusters based on matched tracks
 *  according to the following algorithm:
 *    1. Build map of clusters onto matched track
 *       projections.
 *    2. For each cluster-track pair:
 *       a.  Calculate significance of pair's E/p wrt provided
 *           average and RMS of E/p
 *       b. If significance is less than `minSigCut`,
 *           merge all clusters within `drAdd`.
 *    3. Create a protocluster for each merged cluster
 *       - If multiple tracks point to same merged
 *         protocluster, create new protocluster for
 *         each projection with hits weighted relative
 *         to track.
 *    4. Convert any unmatched clusters into protoclusters.
 */
void TrackClusterMergeSplitter::process(const TrackClusterMergeSplitter::Input& input,
                                        const TrackClusterMergeSplitter::Output& output) const {

  // grab inputs/outputs
  const auto [in_match, in_cluster, in_project] = input;
#if EDM4EIC_VERSION_MAJOR >= 8 && EDM4EIC_VERSION_MINOR >= 4
  auto [out_proto, out_match] = output;
#else
  auto [out_proto] = output;
#endif

  // exit if no clusters in collection
  if (in_cluster->size() == 0) {
    debug("No clusters in input collection.");
    return;
  }

  // emit debugging message if no matched tracks in collection
  if (in_match->size() == 0) {
    debug("No matched tracks in collection.");
    return;
  }

  // --------------------------------------------------------------------------
  // 1. Build map of clusters onto tracks/projections
  // --------------------------------------------------------------------------
  MapToVecSeg mapProjToSplit;
  for (const auto& match : *in_match) {
    for (const auto& project : *in_project) {

      // pick out corresponding projection from track
      if (match.getTrack() != project.getTrack()) {
        continue;
      } else {
        mapProjToSplit[match.getCluster()].push_back(project);
      }
    }
  } // end track-cluster match loop

  // ------------------------------------------------------------------------
  // 2. Loop over projection-cluster pairs to check if merging is needed
  // ------------------------------------------------------------------------
  SetClust setUsedClust;
  MapToVecClust mapClustToMerge;
  for (auto& [clust_seed, vecMatchProj] : mapProjToSplit) {

    // at this point, track-cluster matches are 1-to-1
    // so grab matched track and get its projection to
    // a specific point
    std::optional<edm4eic::TrackPoint> project_seed;
    for (auto point : vecMatchProj.front().getPoints()) {
      if (point.surface == m_cfg.surfaceToUse) {
        project_seed = point;
      }
    }
    if (!project_seed)
      continue;

    // skip if cluster is already used
    if (setUsedClust.contains(clust_seed)) {
      continue;
    }

    // add cluster to list and flag as used
    mapClustToMerge[clust_seed].push_back(clust_seed);
    setUsedClust.insert(clust_seed);

    // grab cluster energy and projection momentum
    const float eClustSeed = clust_seed.getEnergy();
    const float eProjSeed  = m_cfg.avgEP * edm4hep::utils::magnitude(project_seed.value().momentum);

    // ----------------------------------------------------------------------
    // 2(a). Calculate significance
    // ----------------------------------------------------------------------
    const float sigSeed = (eClustSeed - eProjSeed) / m_cfg.sigEP;
    trace("Seed energy = {}, expected energy = {}, significance = {}", eClustSeed, eProjSeed,
          sigSeed);

    // ----------------------------------------------------------------------
    // 2(b). If significance is above threshold, do nothing.
    //       Otherwise identify clusters to merge.
    // ----------------------------------------------------------------------
    if (sigSeed > m_cfg.minSigCut) {
      continue;
    }

    // get eta, phi of seed
    const float etaSeed = edm4hep::utils::eta(clust_seed.getPosition());
    const float phiSeed = edm4hep::utils::angleAzimuthal(clust_seed.getPosition());

    // loop over other clusters
    float eClustSum = eClustSeed;
    float sigSum    = sigSeed;
    for (auto cluster : *in_cluster) {

      // ignore used clusters
      if (setUsedClust.contains(cluster)) {
        continue;
      }

      // get eta, phi of cluster
      const float etaClust = edm4hep::utils::eta(cluster.getPosition());
      const float phiClust = edm4hep::utils::angleAzimuthal(cluster.getPosition());

      // get distance to seed
      const float drToSeed =
          std::hypot(etaSeed - etaClust, std::remainder(phiSeed - phiClust, 2. * M_PI));

      // --------------------------------------------------------------------
      // If inside merging-window, add to list of clusters to merge
      // --------------------------------------------------------------------
      if (drToSeed > m_cfg.drAdd) {
        continue;
      }
      mapClustToMerge[clust_seed].push_back(cluster);
      setUsedClust.insert(cluster);

      // --------------------------------------------------------------------
      // if picked up cluster w/ matched track, add projection to list
      // --------------------------------------------------------------------
      if (mapProjToSplit.contains(cluster)) {
        vecMatchProj.insert(vecMatchProj.end(), mapProjToSplit[cluster].begin(),
                            mapProjToSplit[cluster].end());
      }

      // increment sums and output debugging
      eClustSum += cluster.getEnergy();
      sigSum = (eClustSum - eProjSeed) / m_cfg.sigEP;
      trace("{} clusters to merge: current sum = {}, current significance = {}, {} track(s) "
            "pointing to merged cluster",
            mapClustToMerge[clust_seed].size(), eClustSum, sigSum, vecMatchProj.size());
    } // end cluster loop
  } // end matched cluster-projection loop

  // ------------------------------------------------------------------------
  // 3. Create an output protocluster for each merged cluster
  //    and for each track pointing to merged cluster
  // ------------------------------------------------------------------------
  for (auto& [clust_seed, vecClustToMerge] : mapClustToMerge) {

    // create a cluster for each projection to merged cluster
    VecProto new_protos;
    for (const auto& project : mapProjToSplit[clust_seed]) {
      new_protos.push_back(out_proto->create());
    }

    // merge & split as needed
    merge_and_split_clusters(vecClustToMerge, mapProjToSplit[clust_seed], new_protos);

#if EDM4EIC_VERSION_MAJOR >= 8 && EDM4EIC_VERSION_MINOR >= 4
    // and finally create a track-protocluster match for each pair
    for (std::size_t iProj = 0; const auto& project : mapProjToSplit[clust_seed]) {
      auto match = out_match->create();
      match.setTo(new_protos[iProj]);
      match.setFrom(project.getTrack());
      match.setWeight(1.0); // FIXME placeholder, should encode goodness of match
      trace("Matched output cluster {} to track {}", new_protos[iProj].getObjectID().index,
            project.getTrack().getObjectID().index);
      ++iProj;
    }
#endif
  } // end clusters to merge loop

  // ------------------------------------------------------------------------
  // 4. Convert unused clusters to protoclusters
  // ------------------------------------------------------------------------
  for (const auto& cluster : *in_cluster) {

    // ignore used clusters
    if (setUsedClust.contains(cluster)) {
      continue;
    }

    // copy cluster and add to output collection
    edm4eic::MutableProtoCluster proto = out_proto->create();
    add_cluster_to_proto(cluster, proto);
    trace("Copied input cluster {} onto output cluster {}", cluster.getObjectID().index,
          proto.getObjectID().index);

  } // end cluster loop

} // end 'process(Input&, Output&)'

// --------------------------------------------------------------------------
//! Merge identified clusters and split if needed
// --------------------------------------------------------------------------
/*! If multiple tracks are pointing to merged cluster, a new
 *  protocluster is created for each track w/ hits weighted by
 *  its distance to the track and the track's momentum.
 */
void TrackClusterMergeSplitter::merge_and_split_clusters(const VecClust& to_merge,
                                                         const VecSeg& to_split,
                                                         VecProto& new_protos) const {

  // if only 1 matched track, no need to split
  // otherwise split merged cluster for each
  // matched track
  if (to_split.size() == 1) {
    for (const auto& old_clust : to_merge) {
      add_cluster_to_proto(old_clust, new_protos.front());
    }
    return;
  } else {
    trace("Splitting merged cluster across {} tracks", to_split.size());
  }

  // calculate weights for splitting
  VecWeights weights(to_split.size());
  for (const auto& old_clust : to_merge) {
    for (const auto& hit : old_clust.getHits()) {

      // calculate a weight for each projection
      double wTotal = 0.;
      for (std::size_t iProj = 0; const auto& projToSplit : to_split) {

        // get track at specific point
        std::optional<edm4eic::TrackPoint> proj;
        for (auto point : projToSplit.getPoints()) {
          if (point.surface == m_cfg.surfaceToUse) {
            proj = point;
          }
        }
        if (!proj)
          continue;

        // get track eta, phi
        const float etaProj = edm4hep::utils::eta(proj.value().position);
        const float phiProj = edm4hep::utils::angleAzimuthal(proj.value().position);

        // get hit eta, phi
        const float etaHit = edm4hep::utils::eta(hit.getPosition());
        const float phiHit = edm4hep::utils::angleAzimuthal(hit.getPosition());

        // get track momentum, distance to hit
        const float mom = edm4hep::utils::magnitude(proj.value().momentum);
        const float dist =
            std::hypot(etaHit - etaProj, std::remainder(phiHit - phiProj, 2. * M_PI));

        // get weight
        const float weight = std::exp(-1. * dist / m_cfg.transverseEnergyProfileScale) * mom;

        // set weight & increment sum of weights
        weights[iProj][hit] = weight;
        wTotal += weight;
        ++iProj;
      }

      // normalize weights over all projections
      for (std::size_t iProj = 0; iProj < to_split.size(); ++iProj) {
        weights[iProj][hit] /= wTotal;
      }
    } // end hits to merge loop

    // merge cluster into split
    for (std::size_t iProj = 0; iProj < to_split.size(); ++iProj) {
      add_cluster_to_proto(old_clust, new_protos[iProj], weights[iProj]);
    }
  } // end clusters to merge loop

} // end 'merge_and_split_clusters(VecClust&, VecSeg&, std::vector<edm4eic::MutableProtoCluster>&)'

// --------------------------------------------------------------------------
//! Add a cluster's hits to a protocluster
// --------------------------------------------------------------------------
void TrackClusterMergeSplitter::add_cluster_to_proto(
    const edm4eic::Cluster& clust, edm4eic::MutableProtoCluster& proto,
    std::optional<MapToWeight> split_weights) const {
  // loop over hits to add
  for (const auto& hit : clust.getHits()) {

    // get weight if needed
    double weight = 1.0;
    if (split_weights.has_value()) {
      weight = split_weights.value()[hit];
    }

    // add to protocluster
    proto.addToHits(hit);
    proto.addToWeights(weight);
  } // end hit loop

} // end 'add_cluster_to_proto(...)'

} // namespace eicrecon
