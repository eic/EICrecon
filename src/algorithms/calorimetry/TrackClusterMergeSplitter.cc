// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Derek Anderson

#include <cmath>
#include <regex>
#include <utility>
#include <algorithm>
#include <stdexcept>
// dd4hep utilities
#include <DD4hep/Readout.h>
// edm4hep types
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
// edm4eic types
#include <edm4eic/ProtoCluster.h>
#include <edm4eic/TrackSegment.h>
#include <edm4eic/CalorimeterHit.h>

// algorithm definition
#include "TrackClusterMergeSplitter.h"



namespace eicrecon {

  // --------------------------------------------------------------------------
  //! Initialize algorithm
  // --------------------------------------------------------------------------
  void TrackClusterMergeSplitter::init(
    const dd4hep::Detector* detector,
    const dd4hep::rec::CellIDPositionConverter* converter
  ) {

    m_detector = detector;
    m_converter = converter;

  }  // end 'init(dd4hep::Detector*)'



  // --------------------------------------------------------------------------
  //! Process inputs
  // --------------------------------------------------------------------------
  void TrackClusterMergeSplitter::process(
    const TrackClusterMergeSplitter::Input& input,
    const TrackClusterMergeSplitter::Output& output
  ) const {

    // grab inputs/outputs
    const auto [in_protoclusters, in_projections] = input;
    auto [out_protoclusters] = output;

    // exit if no clusters in collection
    if (in_protoclusters -> size() == 0) {
      throw std::runtime_error("No proto-clusters in input collection!");
    }

    // reset bookkeeping containers
    reset_bookkeepers();

    // flag all clusters as not being consumed
    for (auto cluster : *in_protoclusters) {
      m_mapIsConsumed.insert(
        {cluster.getObjectID().index, false}
      );
    }
    trace("Initialized map of consumed clusters");

    // collect relevant projections
    get_projections(in_projections, (*in_protoclusters)[0].getHits(0));

    // match clusters to projections
    if (m_vecProject.size() == 0) {
      debug("No projections to match clusters to.");
    } else {
      match_clusters_to_tracks(in_protoclusters);
    }

    // determine what clusters to merge
    for (auto clustAndProject : m_mapClustProject) {

      // skip if cluster is already used
      if (m_mapIsConsumed[clustAndProject.first]) {
        continue;
      }

      // grab matched cluster-projections
      auto clustSeed = (*in_protoclusters)[clustAndProject.first];
      auto projSeed = m_vecProject[clustAndProject.second];

      // add cluster to lists and flag as used
      m_mapClustToMerge.insert(
        {clustAndProject.first, {}}
      );
      m_mapProjToMerge.insert(
        {clustAndProject.first, {clustAndProject.second}}
      );
      m_mapIsConsumed[clustAndProject.first] = true;

      // grab cluster energy and projection momentum
      const float eClustSeed = get_cluster_energy(clustSeed);
      const float eProjSeed = m_cfg.avgEP * edm4hep::utils::magnitude(projSeed.momentum);

      // check significance
      const float sigSeed = (eClustSeed - eProjSeed) / m_cfg.sigEP;
      debug("Seed energy = {}, expected energy = {}, significance = {}", eClustSeed, eProjSeed, sigSeed);

      // if above threshold, do nothing
      //   - otherwise, identify clusters to merge
      if (sigSeed > m_cfg.minSigCut) {
        continue;
      }

      // get eta, phi of seed
      const auto  posSeed = get_cluster_position(clustSeed);
      const float etaSeed = edm4hep::utils::eta(posSeed);
      const float phiSeed = edm4hep::utils::angleAzimuthal(posSeed);

      // loop over other clusters
      float eClustSum = eClustSeed;
      float sigSum = sigSeed;
      for (auto in_cluster : *in_protoclusters) {

        // grab index and ignore used clusters
        const int iCluster = in_cluster.getObjectID().index;
        if (m_mapIsConsumed[iCluster]) {
          continue;
        }

        // skip if seed cluster
        if (iCluster == clustAndProject.first) {
          continue;
        }

        // get eta, phi of cluster
        const auto  posClust = get_cluster_position(in_cluster);
        const float etaClust = edm4hep::utils::eta(posClust);
        const float phiClust = edm4hep::utils::angleAzimuthal(posClust);

        // get distance to seed
        const float drToSeed = std::hypot(
          etaSeed - etaClust,
          phiSeed - phiClust
        );

        // skip if outside window
        //   - otherwise, add to merge list
        if (drToSeed > m_cfg.drAdd) {
          continue;
        } else {
          m_mapClustToMerge[clustAndProject.first].push_back(iCluster);
          m_mapIsConsumed[iCluster] = true;
        }

        // if picked up cluster w/ matched track, add to list
        if (m_mapClustProject.count(iCluster)) {
          m_mapProjToMerge[clustAndProject.first].push_back(
            m_mapClustProject[iCluster]
          );
        }

        // increment sums and output debugging
        eClustSum += get_cluster_energy(in_cluster);
        sigSum = (eClustSum - eProjSeed) / m_cfg.sigEP;
        debug(
          "{} clusters to merge: current sum = {}, current significance = {}, {} track(s) pointing to merged cluster",
          m_mapClustToMerge[clustAndProject.first].size(),
          eClustSum,
          sigSum,
          m_mapProjToMerge[clustAndProject.first].size()
        );
      }  // end cluster loop
    }  // end matched cluster-projection loop

    // do cluster merging
    for (auto clustToMerge : m_mapClustToMerge) {

      // add clusters to be merged
      m_vecClust.clear();
      for (const int& iClustToMerge : clustToMerge.second) {
        m_vecClust.push_back( (*in_protoclusters)[iClustToMerge] );
      }
      m_vecClust.push_back( (*in_protoclusters)[clustToMerge.first] );

      // for each track pointing to merged cluster, merge clusters
      for (const int& iMatchedTrk : m_mapProjToMerge[clustToMerge.first]) {
        edm4eic::MutableProtoCluster merged_cluster = out_protoclusters->create();
        merge_clusters(
          m_vecProject[iMatchedTrk],
          m_vecClust,
          merged_cluster
        );
      }
    }  // end clusters to merge loop

    // copy unused clusters to output
    for (auto in_cluster : *in_protoclusters) {

      // ignore used clusters
      if (m_mapIsConsumed[in_cluster.getObjectID().index]) {
        continue;
      }

      // copy cluster and add to output collection
      edm4eic::MutableProtoCluster out_cluster = out_protoclusters->create();
      copy_cluster(in_cluster, out_cluster);

    }  // end cluster loop
    trace("Copied unused clusters into output collection");

  }  // end 'process(Input&, Output&)'



  // --------------------------------------------------------------------------
  //! Reset bookkeeping containers
  // --------------------------------------------------------------------------
  void TrackClusterMergeSplitter::reset_bookkeepers() const {

    m_mapIsConsumed.clear();
    m_mapClustProject.clear();
    m_mapClustToMerge.clear();
    m_mapProjToMerge.clear();
    m_vecProject.clear();
    m_vecClust.clear();
    trace("Reset bookkeeping containers");

  }  // end 'reset_bookkeepers()'



  // --------------------------------------------------------------------------
  //! Collect projections pointing to calorimeter
  // --------------------------------------------------------------------------
  void TrackClusterMergeSplitter::get_projections(
    const edm4eic::TrackSegmentCollection* projections,
    const edm4eic::CalorimeterHit& hit
  ) const {

    // return if projections are empty
    if (projections -> size() == 0) {
      debug("No projections in input collection.");
      return;
    }

    // get readout
    auto readout = m_converter -> findReadout(
      m_converter -> findDetElement(
        m_converter -> position(hit.getCellID())
      )
    );

    // grab detector id
    const int id = m_detector -> constant<int>(
      std::regex_replace(readout.name(), std::regex("Hits"), "_ID")
    );
    debug("Collecting projections to detector with system id {}", id);

    // collect projections
    for (auto project : *projections) {
      for (auto point : project.getPoints()) {
        if (
          (point.system  == id) &&
          (point.surface == 1)
        ) {
          m_vecProject.push_back(point);
        }
      }  // end point loop
    }  // end projection loop
    trace("Collected relevant projections: {} to process", m_vecProject.size());

  }  // end 'get_projections(edm4eic::CalorimeterHit&, edm4eic::TrackSegmentCollection&)'



  // --------------------------------------------------------------------------
  //! Match clusters to track projections
  // --------------------------------------------------------------------------
  /*! FIXME this might be better handled in a separate algorithm
   */
  void TrackClusterMergeSplitter::match_clusters_to_tracks(
    const edm4eic::ProtoClusterCollection* clusters
  ) const {


    // loop over relevant projections
    for (uint32_t iProject = 0; iProject < m_vecProject.size(); ++iProject) {

      // grab projection
      auto project = m_vecProject[iProject];

      // get eta, phi of projection
      const float etaProj = edm4hep::utils::eta(project.position);
      const float phiProj = edm4hep::utils::angleAzimuthal(project.position);

      // find closest cluster
      bool  foundMatch = false;
      float dMatch = std::numeric_limits<float>::max();
      int   iMatch = std::numeric_limits<int>::max();
      for (auto cluster : *clusters) {

        // get eta, phi of cluster
        const auto  posClust = get_cluster_position(cluster);
        const float etaClust = edm4hep::utils::eta(posClust);
        const float phiClust = edm4hep::utils::angleAzimuthal(posClust);

        // calculate distance to centroid
        const float dist = std::hypot(
          etaProj - etaClust,
          phiProj - phiClust
        );

        // if closer, set match to current projection
        if (dist <= dMatch) {
          foundMatch = true;
          dMatch = dist;
          iMatch = cluster.getObjectID().index;
        }
      }  // end cluster loop

      // record match if found
      if (foundMatch) {
        m_mapClustProject.insert(
          {iMatch, iProject}
        );
        debug("Matched cluster to track projection: eta-phi distance = {}", dMatch);
      }
    }  // end cluster loop
    trace ("Finished matching clusters to track projections: {} matches", m_mapClustProject.size());

  }  // end 'match_clusters_to_tracks(edm4eic::ClusterCollection*)'



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
          phiHit - phiTrk
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
