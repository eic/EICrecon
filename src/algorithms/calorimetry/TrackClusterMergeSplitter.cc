// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Derek Anderson

#define EICRECON_TRACKCLUSTERMERGESPLITTER_CC

#include <cmath>
#include <regex>
#include <utility>
#include <algorithm>
// dd4hep utilities
#include <DD4hep/Readout.h>
// jana utilities
#include <JANA/JException.h>
// edm4hep types
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
// edm4eic types
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
    const auto [in_clusters, in_projections] = input;
    auto [out_clusters] = output;

    // exit if no clusters in collection
    if (in_clusters -> size() == 0) {
      throw JException("No clusters in input collection!");
    }

    // reset bookkeeping containers
    reset_bookkeepers();

    // flag all clusters as not being consumed
    for (auto cluster : *in_clusters) {
      m_mapIsConsumed.insert(
        {cluster.getObjectID().index, false}
      );
    }
    trace("Initialized map of consumed clusters");

    // collect relevant projections
    get_projections(in_projections, (*in_clusters)[0].getHits(0));

    // match clusters to projections
    if (m_vecProject.size() == 0) {
      debug("No projections to match clusters to.");
    } else {
      match_clusters_to_tracks(in_clusters);
    }

    // determine what clusters to merge
    for (auto clustAndProject : m_mapClustProject) {

      // skip if cluster is already used
      if (m_mapIsConsumed[clustAndProject.first]) {
        continue;
      }

      // grab matched cluster-projections
      auto clustSeed = (*in_clusters)[clustAndProject.first];
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
      const float eClustSeed = clustSeed.getEnergy();
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
      const float etaSeed = edm4hep::utils::eta(clustSeed.getPosition());
      const float phiSeed = std::atan2(clustSeed.getPosition().y, clustSeed.getPosition().x);

      // loop over other clusters
      float eClustSum = eClustSeed;
      float sigSum = sigSeed;
      for (auto in_cluster : *in_clusters) {

        // grab index and ignore used clusters
        const int iCluster = in_cluster.getObjectID().index;
        if (m_mapIsConsumed[iCluster]) {
          continue;
        }

        // skip if seed cluster
        if (iCluster == clustAndProject.first) {
          continue;
        }

        // get eta, phi of seed
        const float etaClust = edm4hep::utils::eta(in_cluster.getPosition());
        const float phiClust = std::atan2(in_cluster.getPosition().y, in_cluster.getPosition().x);

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
        eClustSum += in_cluster.getEnergy();
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
        m_vecClust.push_back( (*in_clusters)[iClustToMerge] );
      }
      m_vecClust.push_back( (*in_clusters)[clustToMerge.first] );

      // for each track pointing to merged cluster, merge clusters
      for (const int& iMatchedTrk : m_mapProjToMerge[clustToMerge.first]) {
        edm4eic::MutableCluster merged_cluster = out_clusters->create();
        merge_clusters(
          m_vecProject[iMatchedTrk],
          m_vecClust,
          merged_cluster
        );
      }
    }  // end clusters to merge loop

    // copy unused clusters to output
    for (auto in_cluster : *in_clusters) {

      // ignore used clusters
      if (m_mapIsConsumed[in_cluster.getObjectID().index]) {
        continue;
      }

      // copy cluster and add to output collection
      edm4eic::MutableCluster out_cluster = out_clusters->create();
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
    const edm4eic::ClusterCollection* clusters
  ) const {


    // loop over relevant projections
    for (uint32_t iProject = 0; iProject < m_vecProject.size(); ++iProject) {

      // grab projection
      auto project = m_vecProject[iProject];

      // get eta, phi of projection
      const float etaProj = edm4hep::utils::eta(project.position);
      const float phiProj = atan2(project.position.y, project.position.x);

      // find closest cluster
      bool  foundMatch = false;
      float dMatch = std::numeric_limits<float>::max();
      int   iMatch = std::numeric_limits<int>::max();
      for (auto cluster : *clusters) {

        // get eta, phi of cluster
        const float etaClust = edm4hep::utils::eta(cluster.getPosition());
        const float phiClust = std::atan2(cluster.getPosition().y, cluster.getPosition().x);

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
    edm4eic::MutableCluster& merged_clust
  ) const {

    // get track eta, phi
    const float etaTrk = edm4hep::utils::eta(matched_trk.position);
    const float phiTrk = atan2(matched_trk.position.y, matched_trk.position.x);

    // grab hits from each cluster to merge
    float eTotal = 0.;
    float tClust = std::numeric_limits<float>::max();
    float tErrClust = std::numeric_limits<float>::max();
    for (auto old_clust : to_merge) {
      for (auto hit : old_clust.getHits()) {

        // get hit eta, phi
        const float etaHit = edm4hep::utils::eta(hit.getPosition());
        const float phiHit = std::atan2(hit.getPosition().y, hit.getPosition().x);

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

        // set time of cluster to be time of earliest hit
        if (hit.getTime() < tClust) {
          tClust = hit.getTime();
          tErrClust = hit.getTimeError();
        }

        // set hit one-to-many relation
        merged_clust.addToHits( hit );
        merged_clust.addToHitContributions( eContrib );
      }  // end hit loop

      // set remaining one-to-many relations
      merged_clust.addToClusters( old_clust );
      for (auto old_pid : old_clust.getParticleIDs()) {
        merged_clust.addToParticleIDs( old_pid );
      }
      debug("Merged input cluster {} into output cluster {}", old_clust.getObjectID().index, merged_clust.getObjectID().index);

    }  // end of cluster loop

    // set cluster energ/time information
    merged_clust.setEnergy( eTotal / m_cfg.sampFrac );
    merged_clust.setTime( tClust );
    merged_clust.setTimeError( tErrClust );

    // grab initial position of cluster
    edm4hep::Vector3f posMerged = merged_clust.getPosition();

    // now calculate center-of-gravity
    double sumWeights = 0.;
    for (size_t iHit = 0; iHit < merged_clust.getHits().size(); iHit++) {

      // calculate weight
      const double logWeight = m_cfg.logBase + std::log(merged_clust.getHitContributions()[iHit] / eTotal);
      const double posWeight = std::max(0., logWeight);

      // increment sums
      posMerged = posMerged + (posWeight * merged_clust.getHits()[iHit].getPosition());
      sumWeights += posWeight;
    }

    // set center-of-gravity and intrinsic theta, phi
    if (sumWeights > 0.) {
      merged_clust.setPosition( posMerged / sumWeights );
      trace("Calculated center of gravity for merged cluster");
    } else {
      error("Sum of weights 0 for merged cluster {}!", merged_clust.getObjectID().index);
    }
    merged_clust.setIntrinsicTheta( edm4hep::utils::anglePolar(merged_clust.getPosition()) );
    merged_clust.setIntrinsicPhi( edm4hep::utils::angleAzimuthal(merged_clust.getPosition()) );

  }  // end 'merge_clusters(std::vector<edm4eic::Cluster>&, edm4eic::MutableCluster&)'





  // --------------------------------------------------------------------------
  //! Copy cluster onto new one
  // --------------------------------------------------------------------------
  void TrackClusterMergeSplitter::copy_cluster(
    const edm4eic::Cluster& old_clust,
    edm4eic::MutableCluster& new_clust
  ) const {

    // set main variables
    new_clust.setType( old_clust.getType() );
    new_clust.setEnergy( old_clust.getEnergy() );
    new_clust.setTime( old_clust.getTime() );
    new_clust.setTimeError( old_clust.getTimeError() );
    new_clust.setNhits( old_clust.getHits().size() );
    new_clust.setPosition( old_clust.getPosition() );
    new_clust.setPositionError( old_clust.getPositionError() );
    new_clust.setIntrinsicTheta( old_clust.getIntrinsicTheta() );
    new_clust.setIntrinsicPhi( old_clust.getIntrinsicPhi() );
    new_clust.setIntrinsicDirectionError( old_clust.getIntrinsicDirectionError() );

    // set vector members
    for (float old_parameter : old_clust.getShapeParameters()) {
      new_clust.addToShapeParameters( old_parameter );
    }
    for (float old_contribution : old_clust.getHitContributions()) {
      new_clust.addToHitContributions( old_contribution );
    }
    for (float old_energy : old_clust.getSubdetectorEnergies()) {
      new_clust.addToSubdetectorEnergies( old_energy );
    }

    // set one-to-many relations
    for (auto old_hit : old_clust.getHits()) {
      new_clust.addToHits( old_hit );
    }
    for (auto old_pid : old_clust.getParticleIDs()) {
      new_clust.addToParticleIDs( old_pid );
    }
    new_clust.addToClusters( old_clust );
    debug("Copied input cluster {} onto output cluster {}", old_clust.getObjectID().index, new_clust.getObjectID().index);

  }  // end 'copy_cluster(edm4eic::Cluster&, edm4eic::MutableCluster&)'

}  // end eicrecon namespace
