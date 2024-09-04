// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Derek Anderson

#include <edm4eic/CalorimeterHit.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <fmt/core.h>
#include <podio/ObjectID.h>
#include <podio/RelationRange.h>
#include <stdint.h>
#include <cmath>
#include <gsl/pointers>
#include <stdexcept>
#include <utility>

// algorithm definition
#include "TrackClusterMergeSplitter.h"
#include "algorithms/calorimetry/TrackClusterMergeSplitterConfig.h"



namespace eicrecon {

  // --------------------------------------------------------------------------
  //! Initialize algorithm
  // --------------------------------------------------------------------------
  void TrackClusterMergeSplitter::init(const dd4hep::Detector* detector) {

    // grab detector id
    m_idCalo = detector -> constant<int>(m_cfg.idCalo);
    debug("Collecting projections to detector with system id {}", m_idCalo);

  }  // end 'init(dd4hep::Detector*)'



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
   *           signficance specified by `minSigCut`,
   *           merge all clusters within `drAdd`.
   *    4. Create a protocluster for each merged cluster
   *       and copy all unused protoclusters into output.
   *       - If multiple tracks point to the same merged
   *         cluster, create a new protocluster for each
   *         projection with hit weighted relative to
   *         the track momentum.
   */
  void TrackClusterMergeSplitter::process(
    const TrackClusterMergeSplitter::Input& input,
    const TrackClusterMergeSplitter::Output& output
  ) const {

    // grab inputs/outputs
    const auto [in_protoclusters, in_projections] = input;
    auto [out_protoclusters] = output;

    // exit if no clusters in collection
    if (in_protoclusters->size() == 0) {
      debug("No proto-clusters in input collection.");
      return;
    }

    // ------------------------------------------------------------------------
    // 1. Identify projections to calorimeter
    // ------------------------------------------------------------------------
    VecTrkPoint vecProject;
    get_projections(in_projections, vecProject);

    // ------------------------------------------------------------------------
    // 2. Match relevant projections to clusters
    // ------------------------------------------------------------------------
    MapOneToIndex mapClustProject;
    if (vecProject.size() == 0) {
      debug("No projections to match clusters to.");
      return;
    } else {
      match_clusters_to_tracks(in_protoclusters, vecProject, mapClustProject);
    }

    // ------------------------------------------------------------------------
    // 3. Loop over projection-cluster pairs to check if merging is needed
    // ------------------------------------------------------------------------
    SetOfIDs setUsedClust;
    MapClustToMerge mapClustToMerge;
    MapProjToMerge mapProjToMerge;
    for (auto clustAndProject : mapClustProject) {

      // skip if cluster is already used
      if (setUsedClust.count(clustAndProject.first)) {
        continue;
      }

      // grab matched cluster-projections
      auto clustSeed = (*in_protoclusters)[clustAndProject.first.index];
      auto projSeed = vecProject[clustAndProject.second];

      // add cluster to lists and flag as used
      mapClustToMerge.insert(
        {clustAndProject.first, {}}
      );
      mapProjToMerge.insert(
        {clustAndProject.first, {clustAndProject.second}}
      );
      setUsedClust.insert(clustAndProject.first);

      // grab cluster energy and projection momentum
      const float eClustSeed = get_cluster_energy(clustSeed);
      const float eProjSeed = m_cfg.avgEP * edm4hep::utils::magnitude(projSeed.momentum);

      // ----------------------------------------------------------------------
      // 3.i. Calculate significance
      // ----------------------------------------------------------------------
      const float sigSeed = (eClustSeed - eProjSeed) / m_cfg.sigEP;
      trace("Seed energy = {}, expected energy = {}, significance = {}", eClustSeed, eProjSeed, sigSeed);

      // ----------------------------------------------------------------------
      // 3.ii. If significance is above threshold, do nothing.
      //       Otherwise identify clusters to merge.
      // ----------------------------------------------------------------------
      if (sigSeed > m_cfg.minSigCut) {
        continue;
      }

      // get eta, phi of seed
      const auto posSeed = get_cluster_position(clustSeed);
      const float etaSeed = edm4hep::utils::eta(posSeed);
      const float phiSeed = edm4hep::utils::angleAzimuthal(posSeed);

      // loop over other clusters
      float eClustSum = eClustSeed;
      float sigSum = sigSeed;
      for (auto in_cluster : *in_protoclusters) {

        // grab id and ignore used clusters
        const podio::ObjectID idCluster = in_cluster.getObjectID();
        if (setUsedClust.count(idCluster)) {
          continue;
        }

        // get eta, phi of cluster
        const auto posClust = get_cluster_position(in_cluster);
        const float etaClust = edm4hep::utils::eta(posClust);
        const float phiClust = edm4hep::utils::angleAzimuthal(posClust);

        // get distance to seed
        const float drToSeed = std::hypot(
          etaSeed - etaClust,
          std::remainder(phiSeed - phiClust, 2. * M_PI)
        );

        // --------------------------------------------------------------------
        // If inside merging-window, add to list of clusters to merge
        // --------------------------------------------------------------------
        if (drToSeed > m_cfg.drAdd) {
          continue;
        } else {
          mapClustToMerge[clustAndProject.first].push_back(idCluster);
          setUsedClust.insert(idCluster);
        }

        // --------------------------------------------------------------------
        // if picked up cluster w/ matched track, add projection to list
        // --------------------------------------------------------------------
        if (mapClustProject.count(idCluster)) {
          mapProjToMerge[clustAndProject.first].push_back(
            mapClustProject[idCluster]
          );
        }

        // increment sums and output debugging
        eClustSum += get_cluster_energy(in_cluster);
        sigSum = (eClustSum - eProjSeed) / m_cfg.sigEP;
        trace(
          "{} clusters to merge: current sum = {}, current significance = {}, {} track(s) pointing to merged cluster",
          mapClustToMerge[clustAndProject.first].size(),
          eClustSum,
          sigSum,
          mapProjToMerge[clustAndProject.first].size()
        );
      }  // end cluster loop
    }  // end matched cluster-projection loop

    // ------------------------------------------------------------------------
    // 4. Create an output protocluster for each merged cluster and for
    //    each track pointing to merged cluster
    // ------------------------------------------------------------------------
    VecCluster vecClust;
    for (auto clustToMerge : mapClustToMerge) {

      // add clusters to be merged
      vecClust.clear();
      for (const podio::ObjectID& idClustToMerge : clustToMerge.second) {
        vecClust.push_back( (*in_protoclusters)[idClustToMerge.index] );
      }
      vecClust.push_back( (*in_protoclusters)[clustToMerge.first.index] );

      // for each track pointing to merged cluster, merge clusters
      for (const int& iMatchedTrk : mapProjToMerge[clustToMerge.first]) {
        edm4eic::MutableProtoCluster merged_cluster = out_protoclusters->create();
        merge_clusters(
          vecProject[iMatchedTrk],
          vecClust,
          merged_cluster
        );
      }
    }  // end clusters to merge loop

    // ------------------------------------------------------------------------
    // copy unused clusters to output
    // ------------------------------------------------------------------------
    for (auto in_cluster : *in_protoclusters) {

      // ignore used clusters
      if (setUsedClust.count(in_cluster.getObjectID())) {
        continue;
      }

      // copy cluster and add to output collection
      edm4eic::MutableProtoCluster out_cluster = out_protoclusters->create();
      copy_cluster(in_cluster, out_cluster);

    }  // end cluster loop

  }  // end 'process(Input&, Output&)'



  // --------------------------------------------------------------------------
  //! Collect projections pointing to calorimeter
  // --------------------------------------------------------------------------
  void TrackClusterMergeSplitter::get_projections(
    const edm4eic::TrackSegmentCollection* projections,
    VecTrkPoint& relevant_projects
  ) const {

    // return if projections are empty
    if (projections->size() == 0) {
      debug("No projections in input collection.");
      return;
    }

    // collect projections
    for (auto project : *projections) {
      for (auto point : project.getPoints()) {
        if (
          (point.system  == m_idCalo) &&
          (point.surface == 1)
        ) {
          relevant_projects.push_back(point);
        }
      }  // end point loop
    }  // end projection loop
    trace("Collected relevant projections: {} to process", relevant_projects.size());

  }  // end 'get_projections(edm4eic::CalorimeterHit&, edm4eic::TrackSegmentCollection&, VecTrkPoint&)'



  // --------------------------------------------------------------------------
  //! Match clusters to track projections
  // --------------------------------------------------------------------------
  /*! FIXME remove this once cluster-track matching has been centralized
   */
  void TrackClusterMergeSplitter::match_clusters_to_tracks(
    const edm4eic::ProtoClusterCollection* clusters,
    const VecTrkPoint& projections,
    MapOneToIndex& matches
  ) const {


    // loop over relevant projections
    for (uint32_t iProject = 0; iProject < projections.size(); ++iProject) {

      // grab projection
      auto project = projections[iProject];

      // get eta, phi of projection
      const float etaProj = edm4hep::utils::eta(project.position);
      const float phiProj = edm4hep::utils::angleAzimuthal(project.position);

      // to store id of matched cluster
      podio::ObjectID idMatch;

      // find closest cluster
      bool foundMatch = false;
      float dMatch = m_cfg.drAdd;
      for (auto cluster : *clusters) {

        // get eta, phi of cluster
        const auto  posClust = get_cluster_position(cluster);
        const float etaClust = edm4hep::utils::eta(posClust);
        const float phiClust = edm4hep::utils::angleAzimuthal(posClust);

        // calculate distance to centroid
        const float dist = std::hypot(
          etaProj - etaClust,
          std::remainder(phiProj - phiClust, 2. * M_PI)
        );

        // if closer, set match to current projection
        if (dist <= dMatch) {
          foundMatch = true;
          dMatch = dist;
          idMatch = cluster.getObjectID();
        }
      }  // end cluster loop

      // record match if found
      if (foundMatch) {
        matches.insert(
          {idMatch, iProject}
        );
        trace("Matched cluster to track projection: eta-phi distance = {}", dMatch);
      }
    }  // end cluster loop
    debug("Finished matching clusters to track projections: {} matches", matches.size());

  }  // end 'match_clusters_to_tracks(edm4eic::ClusterCollection*, VecTrkPoint&, MapOneToIndex&)'



  // --------------------------------------------------------------------------
  //! Merge identified clusters
  // --------------------------------------------------------------------------
  void TrackClusterMergeSplitter::merge_clusters(
    const edm4eic::TrackPoint& matched_trk,
    const VecCluster& to_merge,
    edm4eic::MutableProtoCluster& merged_clust
  ) const {

    // get track eta, phi
    const float etaTrk = edm4hep::utils::eta(matched_trk.position);
    const float phiTrk = edm4hep::utils::angleAzimuthal(matched_trk.position);

    // grab hits from each cluster to merge
    float eTotal = 0.;
    for (auto old_clust : to_merge) {
      for (auto hit : old_clust.getHits()) {

        // get hit eta, phi
        const float etaHit = edm4hep::utils::eta(hit.getPosition());
        const float phiHit = edm4hep::utils::angleAzimuthal(hit.getPosition());

        // get distance to track
        const float dist = std::hypot(
          etaHit - etaTrk,
          std::remainder(phiHit - phiTrk, 2. * M_PI)
        );

        // recalculate weighted energy wrt track
        const float weight = std::exp(-1. * dist / m_cfg.distScale);
        const float eContrib = hit.getEnergy() * weight;

        // increment sum and print debugging message
        eTotal += eContrib;
        trace("Added hit: eHit = {}, weight = {}, eContrib = {}, running total = {}", hit.getEnergy(), weight, eContrib, eTotal);

        // set hit one-to-many/vector relations
        merged_clust.addToHits( hit );
        merged_clust.addToWeights( weight );

      }  // end hit loop
      debug("Merged input cluster {} into output cluster {}", old_clust.getObjectID().index, merged_clust.getObjectID().index);

    }  // end of cluster loop

  }  // end 'merge_clusters(std::vector<edm4eic::Cluster>&, edm4eic::MutableCluster&)'





  // --------------------------------------------------------------------------
  //! Copy cluster onto new one
  // --------------------------------------------------------------------------
  void TrackClusterMergeSplitter::copy_cluster(
    const edm4eic::ProtoCluster& old_clust,
    edm4eic::MutableProtoCluster& new_clust
  ) const {

    // set one-to-many relations
    for (auto hit : old_clust.getHits()) {
      new_clust.addToHits( hit );
    }

    // set vector members
    for (auto weight : old_clust.getWeights()) {
      new_clust.addToWeights( weight );
    }
    debug("Copied input cluster {} onto output cluster {}", old_clust.getObjectID().index, new_clust.getObjectID().index);

  }  // end 'copy_cluster(edm4eic::Cluster&, edm4eic::MutableCluster&)'



  // --------------------------------------------------------------------------
  //! Grab current energy of protocluster
  // --------------------------------------------------------------------------
  float TrackClusterMergeSplitter::get_cluster_energy(const edm4eic::ProtoCluster& clust) const {

    float eClust = 0.;
    for (auto hit : clust.getHits()) {
      eClust += hit.getEnergy();
    }
    return eClust / m_cfg.sampFrac;

  }  // end 'get_cluster_energy(edm4eic::ProtoCluster&)'



  // --------------------------------------------------------------------------
  //! Get current center of protocluster
  // --------------------------------------------------------------------------
  edm4hep::Vector3f TrackClusterMergeSplitter::get_cluster_position(const edm4eic::ProtoCluster& clust) const {

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

  }  // end 'get_cluster_position(edm4eic::ProtoCluster&)'

}  // end eicrecon namespace
