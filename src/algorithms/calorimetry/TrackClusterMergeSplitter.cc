// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Derek Anderson

#define EICRECON_TRACKCLUSTERMERGESPLITTER_CC

#include <cmath>
#include <regex>
#include <utility>
#include <iostream>  // TEST
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

    // collect relevant projections
    get_projections(in_projections, (*in_clusters)[0].getHits(0));

    // match clusters to projections
    if (m_vecProject.size() == 0) {
      debug("No projections to match clusters to.");
    } else {
      match_clusters_to_tracks(in_clusters);
    }

    /* TODO
     *   - Add merging step
     *   - Splitting step
     */

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

    m_mapClustProject.clear();
    m_mapIsConsumed.clear();
    m_vecProject.clear();
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
        const bool isInSystem  = (point.system  == id);
        const bool isAtFace    = (point.surface == 1);
        if (isInSystem && isAtFace) {
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
      const double projEta = edm4hep::utils::eta(project.position);
      const double projPhi = atan2(project.position.y, project.position.x);

      // find closest cluster
      bool  foundMatch = false;
      float dMatch = std::numeric_limits<float>::max();
      int   iMatch = std::numeric_limits<int>::max();
      for (auto cluster : *clusters) {

        // get eta, phi of cluster
        const float clustEta = edm4hep::utils::eta(cluster.getPosition());
        const float clustPhi = std::atan2(cluster.getPosition().y, cluster.getPosition().x);

        // calculate distance to centroid
        const float dist = std::hypot(
          projEta - clustEta,
          projPhi - clustPhi
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
    trace("Copied input cluster {} onto output cluster {}", old_clust.getObjectID().index, new_clust.getObjectID().index);

  }  // end 'copy_cluster(edm4eic::Cluster&, edm4eic::MutableCluster&)'

}  // end eicrecon namespace
