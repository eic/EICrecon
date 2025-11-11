// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Derek Anderson

#include <edm4hep/MCParticle.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <fmt/core.h>
#include <podio/ObjectID.h>
#include <podio/RelationRange.h>
#include <cmath>
#include <cstddef>
#include <gsl/pointers>

// algorithm definition
#include "TrackClusterMergeSplitter.h"
#include "algorithms/calorimetry/TrackClusterMergeSplitterConfig.h"

namespace eicrecon {

// --------------------------------------------------------------------------
//! Initialize algorithm
// --------------------------------------------------------------------------
void TrackClusterMergeSplitter::init(const dd4hep::Detector* detector) {

  // grab detector id
  m_idCalo = detector->constant<int>(m_cfg.idCalo);
  debug("Collecting projections to detector with system id {}", m_idCalo);

} // end 'init(dd4hep::Detector*)'

// --------------------------------------------------------------------------
//! Process inputs
// --------------------------------------------------------------------------
/*! Primary algorithm call: algorithm ingests a collection
 *  clusters and a collection of track projections. It then
 *  decides to merge or split clusters according to the
 *  following algorithm:
 *    1. Identify all tracks projections pointing to the
 *       specified calorimeter.
 *    2. Match relevant track projections to clusters
 *       based on distance between projection and the energy-
 *       weighted barycenter of the cluster;
 *    3. For each cluster-track pair:
 *       i.  Calculate the significance of the pair's
 *           E/p relative to the provided mean E/p and
 *           its RMS; and
 *       ii. If the significance is less than the
 *           significance specified by `minSigCut`,
 *           merge all clusters within `drAdd`.
 *    4. Create a cluster for each merged cluster
 *       and copy all unused clusters into output.
 *       - If multiple tracks point to the same merged
 *         cluster, create a new cluster for each
 *         projection with hit weighted relative to
 *         the track momentum.
 */
void TrackClusterMergeSplitter::process(const TrackClusterMergeSplitter::Input& input,
                                        const TrackClusterMergeSplitter::Output& output) const {

  // grab inputs/outputs
  const auto [in_matches, in_clusters, in_projections] = input;
#if EDM4EIC_VERSION_MAJOR >= 8 && EDM4EIC_VERSION_MINOR >= 4
  auto [out_protos, out_matches] = output;
#else
  auto [out_protos] = output;
#endif

  // exit if no clusters in collection
  if (in_clusters->size() == 0) {
    debug("No clusters in input collection.");
    return;
  }

  // emit debugging message if no matched tracks in collection
  if (in_matches->size() == 0) {
    debug("No matched tracks in collection.");
    return;
  }

  // --------------------------------------------------------------------------
  // 1. Build map of clusters onto tracks/projections
  // --------------------------------------------------------------------------
  MapToVecProj mapProjToSplit;
  for (const auto& match : *in_matches) {
    for (const auto& project : *in_projections) {

      // pick out corresponding projection from track
      if (match.getTrack() != project.getTrack()) {
        continue;
      } else {
        mapProjToSplit[match.getCluster()].push_back(project);
      }

    }
  }  // end track-cluster match loop

  // ------------------------------------------------------------------------
  // 3. Loop over projection-cluster pairs to check if merging is needed
  // ------------------------------------------------------------------------
  SetClust setUsedClust;
  MapToVecClust mapClustToMerge;
  for (auto& [clustSeed, vecMatchProj] : mapProjToSplit) {

    // at this point, track-cluster matches are 1-to-1
    // so grab matched track and get its projection to
    // a specific point
    std::optional<edm4eic::TrackPoint> projSeed;
    for (auto point : vecMatchProj.front().getPoints()) {
      if (point.surface == m_cfg.surfaceToUse) {
        projSeed = point;
      }
    }
    if (!projSeed) continue;

    // skip if cluster is already used
    if (setUsedClust.contains(clustSeed)) {
      continue;
    }

    // add cluster to list and flag as used
    mapClustToMerge[clustSeed].push_back(clustSeed);
    setUsedClust.insert(clustSeed);

    // grab cluster energy and projection momentum
    const float eClustSeed = clustSeed.getEnergy();
    const float eProjSeed = m_cfg.avgEP * edm4hep::utils::magnitude(projSeed.value().momentum);

    // ----------------------------------------------------------------------
    // 3.i. Calculate significance
    // ----------------------------------------------------------------------
    const float sigSeed = (eClustSeed - eProjSeed) / m_cfg.sigEP;
    trace("Seed energy = {}, expected energy = {}, significance = {}", eClustSeed, eProjSeed,
          sigSeed);

    // ----------------------------------------------------------------------
    // 3.ii. If significance is above threshold, do nothing.
    //       Otherwise identify clusters to merge.
    // ----------------------------------------------------------------------
    if (sigSeed > m_cfg.minSigCut) {
      continue;
    }

    // get eta, phi of seed
    const float etaSeed = edm4hep::utils::eta(clustSeed.getPosition());
    const float phiSeed = edm4hep::utils::angleAzimuthal(clustSeed.getPosition());

    // loop over other clusters
    float eClustSum = eClustSeed;
    float sigSum = sigSeed;
    for (auto in_cluster : *in_clusters) {

      // ignore used clusters
      if (setUsedClust.contains(in_cluster)) {
        continue;
      }

      // get eta, phi of cluster
      const float etaClust = edm4hep::utils::eta(in_cluster.getPosition());
      const float phiClust = edm4hep::utils::angleAzimuthal(in_cluster.getPosition());

      // get distance to seed
      const float drToSeed =
          std::hypot(etaSeed - etaClust, std::remainder(phiSeed - phiClust, 2. * M_PI));

      // --------------------------------------------------------------------
      // If inside merging-window, add to list of clusters to merge
      // --------------------------------------------------------------------
      if (drToSeed > m_cfg.drAdd) {
        continue;
      }
      mapClustToMerge[clustSeed].push_back(in_cluster);
      setUsedClust.insert(in_cluster);

      // --------------------------------------------------------------------
      // if picked up cluster w/ matched track, add projection to list
      // --------------------------------------------------------------------
      if (mapProjToSplit.contains(in_cluster)) {
        vecMatchProj.insert(vecMatchProj.end(), mapProjToSplit[in_cluster].begin(),
                            mapProjToSplit[in_cluster].end());
      }

      // increment sums and output debugging
      eClustSum += in_cluster.getEnergy();
      sigSum = (eClustSum - eProjSeed) / m_cfg.sigEP;
      trace("{} clusters to merge: current sum = {}, current significance = {}, {} track(s) "
            "pointing to merged cluster",
            mapClustToMerge[clustSeed].size(), eClustSum, sigSum, vecMatchProj.size());
    } // end cluster loop
  }   // end matched cluster-projection loop

  // ------------------------------------------------------------------------
  // 4. Create an output protocluster for each merged cluster
  //    and for each track pointing to merged cluster
  // ------------------------------------------------------------------------
  for (auto& [clustSeed, vecClustToMerge] : mapClustToMerge) {

    // create a cluster for each projection to merged cluster
    std::vector<edm4eic::MutableProtoCluster> new_protos;
    for (const auto& proj : mapProjToSplit[clustSeed]) {
      new_protos.push_back(out_protos->create());
    }

    // merge & split as needed
    merge_and_split_clusters(vecClustToMerge, mapProjToSplit[clustSeed], new_protos);

#if EDM4EIC_VERSION_MAJOR >= 8 && EDM4EIC_VERSION_MINOR >= 4
    // and finally create a track-protocluster match for each pair
    for (std::size_t iProj = 0; const auto& project : mapProjToSplit[clustSeed]) {
      auto match = out_matches->create();
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
  // copy unused clusters to output
  // ------------------------------------------------------------------------
  for (auto in_cluster : *in_clusters) {

    // ignore used clusters
    if (setUsedClust.contains(in_cluster)) {
      continue;
    }

    // copy cluster and add to output collection
    edm4eic::MutableProtoCluster out_proto = out_protos->create();
    /* TODO fill in hits here */
    trace("Copied input cluster {} onto output cluster {}", in_cluster.getObjectID().index,
          out_proto.getObjectID().index);

  } // end cluster loop

} // end 'process(Input&, Output&)'

// --------------------------------------------------------------------------
//! Merge identified clusters and split if needed
// --------------------------------------------------------------------------
/*! If multiple tracks are pointing to merged cluster, a new
 *  protocluster is created for each track w/ hits weighted by
 *  its distance to the track and the track's momentum.
 */
void TrackClusterMergeSplitter::merge_and_split_clusters(
    const VecClust& to_merge, const VecProj& to_split,
    std::vector<edm4eic::MutableProtoCluster>& new_protos) const {

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
        if (!proj) continue;

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

} // end 'merge_and_split_clusters(VecClust&, VecProj&, std::vector<edm4eic::MutableProtoCluster>&)'

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
