// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Derek Anderson

#include <DD4hep/Detector.h>
#include <edm4eic/CalorimeterHit.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
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
void TrackClusterMergeSplitter::init() {

  // grab detector id
  m_idCalo = m_geo.detector()->constant<int>(m_cfg.idCalo);
  debug("Collecting projections to detector with system id {}", m_idCalo);

} // end 'init(dd4hep::Detector*)'

// --------------------------------------------------------------------------
//! Process inputs
// --------------------------------------------------------------------------
/*! Primary algorithm call: algorithm ingests a collection
   *  protoclusters and a collection of track projections.
   *  It then decides to merge or split protoclusters according
   *  to the following algorithm:
   *    1. Identify all tracks projections pointing to the
   *       specified calorimeter.
   *    2. Match relevant track projections to protoclusters
   *       based on distance between projection and the energy-
   *       weighted barycenter of the protocluster;
   *    3. For each cluster-track pair:
   *       i.  Calculate the significance of the pair's
   *           E/p relative to the provided mean E/p and
   *           its RMS; and
   *       ii. If the significance is less than the
   *           significance specified by `minSigCut`,
   *           merge all clusters within `drAdd`.
   *    4. Create a protocluster for each merged cluster
   *       and copy all unused protoclusters into output.
   *       - If multiple tracks point to the same merged
   *         cluster, create a new protocluster for each
   *         projection with hit weighted relative to
   *         the track momentum.
   */
void TrackClusterMergeSplitter::process(const TrackClusterMergeSplitter::Input& input,
                                        const TrackClusterMergeSplitter::Output& output) const {

  // grab inputs/outputs
  const auto [in_protoclusters, in_projections] = input;
  auto [out_protoclusters]                      = output;

  // exit if no clusters in collection
  if (in_protoclusters->empty()) {
    debug("No proto-clusters in input collection.");
    return;
  }

  // ------------------------------------------------------------------------
  // 1. Identify projections to calorimeter
  // ------------------------------------------------------------------------
  VecProj vecProject;
  get_projections(in_projections, vecProject);

  // ------------------------------------------------------------------------
  // 2. Match relevant projections to clusters
  // ------------------------------------------------------------------------
  MapToVecProj mapProjToSplit;
  if (vecProject.empty()) {
    debug("No projections to match clusters to.");
    return;
  }
  match_clusters_to_tracks(in_protoclusters, vecProject, mapProjToSplit);

  // ------------------------------------------------------------------------
  // 3. Loop over projection-cluster pairs to check if merging is needed
  // ------------------------------------------------------------------------
  SetClust setUsedClust;
  MapToVecClust mapClustToMerge;
  for (auto& [clustSeed, vecMatchProj] : mapProjToSplit) {

    // at this point, track-cluster matches are 1-to-1
    // so grab matched track
    auto projSeed = vecMatchProj.front();

    // skip if cluster is already used
    if (setUsedClust.contains(clustSeed)) {
      continue;
    }

    // add cluster to list and flag as used
    mapClustToMerge[clustSeed].push_back(clustSeed);
    setUsedClust.insert(clustSeed);

    // grab cluster energy and projection momentum
    const float eClustSeed = get_cluster_energy(clustSeed);
    const float eProjSeed  = m_cfg.avgEP * edm4hep::utils::magnitude(projSeed.momentum);

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
    const auto posSeed  = get_cluster_position(clustSeed);
    const float etaSeed = edm4hep::utils::eta(posSeed);
    const float phiSeed = edm4hep::utils::angleAzimuthal(posSeed);

    // loop over other clusters
    float eClustSum = eClustSeed;
    float sigSum    = sigSeed;
    for (auto in_cluster : *in_protoclusters) {

      // ignore used clusters
      if (setUsedClust.contains(in_cluster)) {
        continue;
      }

      // get eta, phi of cluster
      const auto posClust  = get_cluster_position(in_cluster);
      const float etaClust = edm4hep::utils::eta(posClust);
      const float phiClust = edm4hep::utils::angleAzimuthal(posClust);

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
      eClustSum += get_cluster_energy(in_cluster);
      sigSum = (eClustSum - eProjSeed) / m_cfg.sigEP;
      trace("{} clusters to merge: current sum = {}, current significance = {}, {} track(s) "
            "pointing to merged cluster",
            mapClustToMerge[clustSeed].size(), eClustSum, sigSum, vecMatchProj.size());
    } // end cluster loop
  } // end matched cluster-projection loop

  // ------------------------------------------------------------------------
  // 4. Create an output protocluster for each merged cluster and for
  //    each track pointing to merged cluster
  // ------------------------------------------------------------------------
  for (auto& [clustSeed, vecClustToMerge] : mapClustToMerge) {
    merge_and_split_clusters(vecClustToMerge, mapProjToSplit[clustSeed], out_protoclusters);
  } // end clusters to merge loop

  // ------------------------------------------------------------------------
  // copy unused clusters to output
  // ------------------------------------------------------------------------
  for (auto in_cluster : *in_protoclusters) {

    // ignore used clusters
    if (setUsedClust.contains(in_cluster)) {
      continue;
    }

    // copy cluster and add to output collection
    edm4eic::MutableProtoCluster out_cluster = in_cluster.clone();
    out_protoclusters->push_back(out_cluster);
    trace("Copied input cluster {} onto output cluster {}", in_cluster.getObjectID().index,
          out_cluster.getObjectID().index);

  } // end cluster loop

} // end 'process(Input&, Output&)'

// --------------------------------------------------------------------------
//! Collect projections pointing to calorimeter
// --------------------------------------------------------------------------
void TrackClusterMergeSplitter::get_projections(const edm4eic::TrackSegmentCollection* projections,
                                                VecProj& relevant_projects) const {

  // return if projections are empty
  if (projections->empty()) {
    debug("No projections in input collection.");
    return;
  }

  // collect projections
  for (auto project : *projections) {
    for (auto point : project.getPoints()) {
      if ((point.system == m_idCalo) && (point.surface == 1)) {
        relevant_projects.push_back(point);
      }
    } // end point loop
  } // end projection loop
  debug("Collected relevant projections: {} to process", relevant_projects.size());

} // end 'get_projections(edm4eic::CalorimeterHit&, edm4eic::TrackSegmentCollection&, VecTrkPoint&)'

// --------------------------------------------------------------------------
//! Match clusters to track projections
// --------------------------------------------------------------------------
/*! FIXME remove this once cluster-track matching has been centralized
   */
void TrackClusterMergeSplitter::match_clusters_to_tracks(
    const edm4eic::ProtoClusterCollection* clusters, const VecProj& projections,
    MapToVecProj& matches) const {

  // loop over relevant projections
  for (auto project : projections) {

    // grab projection
    // get eta, phi of projection
    const float etaProj = edm4hep::utils::eta(project.position);
    const float phiProj = edm4hep::utils::angleAzimuthal(project.position);

    // to store matched cluster
    edm4eic::ProtoCluster match;

    // find closest cluster
    bool foundMatch = false;
    float dMatch    = m_cfg.drAdd;
    for (auto cluster : *clusters) {

      // get eta, phi of cluster
      const auto posClust  = get_cluster_position(cluster);
      const float etaClust = edm4hep::utils::eta(posClust);
      const float phiClust = edm4hep::utils::angleAzimuthal(posClust);

      // calculate distance to centroid
      const float dist =
          std::hypot(etaProj - etaClust, std::remainder(phiProj - phiClust, 2. * M_PI));

      // if closer, set match to current projection
      if (dist <= dMatch) {
        foundMatch = true;
        dMatch     = dist;
        match      = cluster;
      }
    } // end cluster loop

    // record match if found
    if (foundMatch) {
      matches[match].push_back(project);
      trace("Matched cluster to track projection: eta-phi distance = {}", dMatch);
    }
  } // end cluster loop
  debug("Finished matching clusters to track projections: {} matches", matches.size());

} // end 'match_clusters_to_tracks(edm4eic::ClusterCollection*, VecTrkPoint&, MapToVecProj&)'

// --------------------------------------------------------------------------
//! Merge identified clusters and split if needed
// --------------------------------------------------------------------------
/*! If multiple tracks are pointing to merged cluster, a new protocluster
   *  is created for each track w/ hits weighted by its distance to the track
   *  and the track's momentum.
   */
void TrackClusterMergeSplitter::merge_and_split_clusters(
    const VecClust& to_merge, const VecProj& to_split,
    edm4eic::ProtoClusterCollection* out_protoclusters) const {

  // if only 1 matched track, no need to split
  if (to_split.size() == 1) {
    edm4eic::MutableProtoCluster new_clust = out_protoclusters->create();
    for (const auto& old_clust : to_merge) {
      for (const auto& hit : old_clust.getHits()) {
        new_clust.addToHits(hit);
        new_clust.addToWeights(1.);
      }
      trace("Merged input cluster {} into output cluster {}", old_clust.getObjectID().index,
            new_clust.getObjectID().index);
    }
    return;
  }

  // otherwise split merged cluster for each matched track
  std::vector<edm4eic::MutableProtoCluster> new_clusters;
  for (const auto& proj [[maybe_unused]] : to_split) {
    new_clusters.push_back(out_protoclusters->create());
  }
  trace("Splitting merged cluster across {} tracks", to_split.size());

  // loop over all hits from all clusters to merge
  std::vector<float> weights(to_split.size(), 1.);
  for (const auto& old_clust : to_merge) {
    for (const auto& hit : old_clust.getHits()) {

      // calculate hit's weight for each track
      for (std::size_t iProj = 0; const auto& proj : to_split) {

        // get track eta, phi
        const float etaProj = edm4hep::utils::eta(proj.position);
        const float phiProj = edm4hep::utils::angleAzimuthal(proj.position);

        // get hit eta, phi
        const float etaHit = edm4hep::utils::eta(hit.getPosition());
        const float phiHit = edm4hep::utils::angleAzimuthal(hit.getPosition());

        // get track momentum, distance to hit
        const float mom = edm4hep::utils::magnitude(proj.momentum);
        const float dist =
            std::hypot(etaHit - etaProj, std::remainder(phiHit - phiProj, 2. * M_PI));

        // set weight
        weights[iProj] = std::exp(-1. * dist / m_cfg.transverseEnergyProfileScale) * mom;
        ++iProj;
      }

      // normalize weights
      float wTotal = 0.;
      for (const float weight : weights) {
        wTotal += weight;
      }
      for (float& weight : weights) {
        weight /= wTotal;
      }

      // add hit to each split merged cluster w/ relevant weight
      for (std::size_t iProj = 0; auto& new_clust : new_clusters) {
        new_clust.addToHits(hit);
        new_clust.addToWeights(weights[iProj]);
      }

    } // end hits to merge loop
  } // end clusters to merge loop

} // end 'merge_and_split_clusters(VecClust&, VecProj&, edm4eic::MutableCluster&)'

// --------------------------------------------------------------------------
//! Grab current energy of protocluster
// --------------------------------------------------------------------------
float TrackClusterMergeSplitter::get_cluster_energy(const edm4eic::ProtoCluster& clust) const {

  float eClust = 0.;
  for (auto hit : clust.getHits()) {
    eClust += hit.getEnergy();
  }
  return eClust / m_cfg.sampFrac;

} // end 'get_cluster_energy(edm4eic::ProtoCluster&)'

// --------------------------------------------------------------------------
//! Get current center of protocluster
// --------------------------------------------------------------------------
edm4hep::Vector3f
TrackClusterMergeSplitter::get_cluster_position(const edm4eic::ProtoCluster& clust) const {

  // grab total energy
  const float eClust = get_cluster_energy(clust) * m_cfg.sampFrac;

  // calculate energy-weighted center
  float wTotal = 0.;
  edm4hep::Vector3f position(0., 0., 0.);
  for (auto hit : clust.getHits()) {

    // calculate weight
    float weight = hit.getEnergy() / eClust;
    wTotal += weight;

    // update cluster position
    position = position + (hit.getPosition() * weight);
  }

  float norm = 1.;
  if (wTotal == 0.) {
    warning("Total weight of 0 in position calculation!");
  } else {
    norm = wTotal;
  }
  return position / norm;

} // end 'get_cluster_position(edm4eic::ProtoCluster&)'

} // namespace eicrecon
