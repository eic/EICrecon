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
    m_idCalo = detector -> constant<int>(m_cfg.idCalo);
    debug("Collecting projections to detector with system id {}", m_idCalo);

  }  // end 'init(dd4hep::Detector*)'



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
  void TrackClusterMergeSplitter::process(
    const TrackClusterMergeSplitter::Input& input,
    const TrackClusterMergeSplitter::Output& output
  ) const {

    // grab inputs/outputs
    const auto [in_clusters, in_projections] = input;
#if EDM4EIC_VERSION_MAJOR >= 8
    auto [out_clusters, out_matches] = output;
#else
    auto [out_clusters] = output;
#endif

    // exit if no clusters in collection
    if (in_clusters->size() == 0) {
      debug("No proto-clusters in input collection.");
      return;
    }

    // ------------------------------------------------------------------------
    // 1. Identify projections to calorimeter
    // ------------------------------------------------------------------------
    VecTrk  vecTrack;
    VecProj vecProject;
    get_projections(in_projections, vecProject, vecTrack);

    // ------------------------------------------------------------------------
    // 2. Match relevant projections to clusters
    // ------------------------------------------------------------------------
    MapToVecTrk  mapTrkToMatch;
    MapToVecProj mapProjToSplit;
    if (vecProject.size() == 0) {
      debug("No projections to match clusters to.");
      return;
    } else {
      match_clusters_to_tracks(in_clusters, vecProject, vecTrack, mapProjToSplit, mapTrkToMatch);
    }

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
      if (setUsedClust.count(clustSeed)) {
        continue;
      }

      // add cluster to list and flag as used
      mapClustToMerge[clustSeed].push_back( clustSeed );
      setUsedClust.insert( clustSeed );

      // grab cluster energy and projection momentum
      const float eClustSeed = clustSeed.getEnergy();
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
      const float etaSeed = edm4hep::utils::eta(clustSeed.getPosition());
      const float phiSeed = edm4hep::utils::angleAzimuthal(clustSeed.getPosition());

      // loop over other clusters
      float eClustSum = eClustSeed;
      float sigSum = sigSeed;
      for (auto in_cluster : *in_clusters) {

        // ignore used clusters
        if (setUsedClust.count(in_cluster)) {
          continue;
        }

        // get eta, phi of cluster
        const float etaClust = edm4hep::utils::eta(in_cluster.getPosition());
        const float phiClust = edm4hep::utils::angleAzimuthal(in_cluster.getPosition());

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
          mapClustToMerge[clustSeed].push_back( in_cluster );
          setUsedClust.insert( in_cluster );
        }

        // --------------------------------------------------------------------
        // if picked up cluster w/ matched track, add projection to list
        // --------------------------------------------------------------------
        if (mapProjToSplit.count(in_cluster)) {
          vecMatchProj.insert(
            vecMatchProj.end(),
            mapProjToSplit[in_cluster].begin(),
            mapProjToSplit[in_cluster].end()
          );
        }

        // increment sums and output debugging
        eClustSum += in_cluster.getEnergy();
        sigSum = (eClustSum - eProjSeed) / m_cfg.sigEP;
        trace(
          "{} clusters to merge: current sum = {}, current significance = {}, {} track(s) pointing to merged cluster",
          mapClustToMerge[clustSeed].size(),
          eClustSum,
          sigSum,
          vecMatchProj.size()
        );
      }  // end cluster loop
    }  // end matched cluster-projection loop

    // ------------------------------------------------------------------------
    // 4. Create an output cluster for each merged cluster and for
    //    each track pointing to merged cluster
    // ------------------------------------------------------------------------
    for (auto& [clustSeed, vecClustToMerge] : mapClustToMerge) {

      // create a cluster for each projection to merged cluster
      std::vector<edm4eic::MutableCluster> new_clusters;
      for (const auto& proj : mapProjToSplit[clustSeed]) {
        new_clusters.push_back( out_clusters->create() );
      }

      // merge & split as needed 
      merge_and_split_clusters(
        vecClustToMerge,
        mapProjToSplit[clustSeed],
        new_clusters
      );

#if EDM4EIC_VERSION_MAJOR >= 8
      // and finally create a track-cluster match for each pair
      for (std::size_t iTrk = 0; const auto& trk : mapTrkToMatch[clustSeed]) {
        edm4eic::MutableTrackClusterMatch match = out_matches->create();
        match.setCluster( new_clusters[iTrk] );
        match.setTrack( trk );
        match.setWeight( 1.0 );  // FIXME placeholder
        trace("Matched output cluster {} to track {}", new_clusters[iTrk].getObjectID().index, trk.getObjectID().index);
      }
#endif
    }  // end clusters to merge loop

    // ------------------------------------------------------------------------
    // copy unused clusters to output
    // ------------------------------------------------------------------------
    for (auto in_cluster : *in_clusters) {

      // ignore used clusters
      if (setUsedClust.count(in_cluster)) {
        continue;
      }

      // copy cluster and add to output collection
      edm4eic::MutableCluster out_cluster = in_cluster.clone();
      out_clusters->push_back(out_cluster);
      trace("Copied input cluster {} onto output cluster {}",
        in_cluster.getObjectID().index,
        out_cluster.getObjectID().index
      );
    }  // end cluster loop

  }  // end 'process(Input&, Output&)'



  // --------------------------------------------------------------------------
  //! Collect projections pointing to calorimeter
  // --------------------------------------------------------------------------
  /*! FIXME remove this once cluster-track matching has been centralized
   */
  void TrackClusterMergeSplitter::get_projections(
    const edm4eic::TrackSegmentCollection* projections,
    VecProj& relevant_projects,
    VecTrk& relevant_trks
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
          relevant_trks.push_back(project.getTrack());
          break;
        }
      }  // end point loop
    }  // end projection loop
    debug("Collected relevant projections: {} to process", relevant_projects.size());

  }  // end 'get_projections(edm4eic::CalorimeterHit&, edm4eic::TrackSegmentCollection&, VecTrkPoint&)'



  // --------------------------------------------------------------------------
  //! Match clusters to track projections
  // --------------------------------------------------------------------------
  /*! FIXME remove this once cluster-track matching has been centralized
   */
  void TrackClusterMergeSplitter::match_clusters_to_tracks(
    const edm4eic::ClusterCollection* clusters,
    const VecProj& projections,
    const VecTrk& tracks,
    MapToVecProj& matched_projects,
    MapToVecTrk& matched_tracks
  ) const {


    // loop over relevant projections
    for (uint32_t iProject = 0; iProject < projections.size(); ++iProject) {

      // grab projection
      auto project = projections[iProject];

      // get eta, phi of projection
      const float etaProj = edm4hep::utils::eta(project.position);
      const float phiProj = edm4hep::utils::angleAzimuthal(project.position);

      // to store matched cluster
      edm4eic::Cluster match;

      // find closest cluster
      bool foundMatch = false;
      float dMatch = m_cfg.drAdd;
      for (auto cluster : *clusters) {

        // get eta, phi of cluster
        const float etaClust = edm4hep::utils::eta(cluster.getPosition());
        const float phiClust = edm4hep::utils::angleAzimuthal(cluster.getPosition());

        // calculate distance to centroid
        const float dist = std::hypot(
          etaProj - etaClust,
          std::remainder(phiProj - phiClust, 2. * M_PI)
        );

        // if closer, set match to current projection
        if (dist <= dMatch) {
          foundMatch = true;
          dMatch = dist;
          match = cluster;
        }
      }  // end cluster loop

      // record match if found
      if (foundMatch) {
        matched_projects[match].push_back(project);
        matched_tracks[match].push_back(tracks[iProject]);
        trace("Matched cluster to track projection: eta-phi distance = {}", dMatch);
      }
    }  // end projection loop
    debug("Finished matching clusters to track projections: {} matches", matched_projects.size());

  }  // end 'match_clusters_to_tracks(edm4eic::ClusterCollection*, VecProj&, VecTrk&, MapToVecProj&, MapToVecTrk&)'



  // --------------------------------------------------------------------------
  //! Merge identified clusters and split if needed
  // --------------------------------------------------------------------------
  /*! If multiple tracks are pointing to merged cluster, a new protocluster
   *  is created for each track w/ hits weighted by its distance to the track
   *  and the track's momentum.
   */
  void TrackClusterMergeSplitter::merge_and_split_clusters(
    const VecClust& to_merge,
    const VecProj& to_split,
    std::vector<edm4eic::MutableCluster>& new_clusters
  ) const {

    // if only 1 matched track, no need to split
    // otherwise split merged cluster for each
    // matched track
    if (to_split.size() == 1) {
      make_cluster(to_merge, new_clusters.front());
      return;
    } else {
      trace("Splitting merged cluster across {} tracks", to_split.size());
    }

    // calculate weights for splitting
    VecMatrix weights(to_split.size(), MatrixF(to_merge.size()));
    for (std::size_t iClust = 0; const auto& old_clust : to_merge) {

      // set aside enough space for each hit
      for (std::size_t iProj = 0; iProj < to_split.size(); ++iProj) {
        weights[iProj][iClust].resize( old_clust.getHits().size() );
      }

      // now loop through each combination of projection & hit
      for (std::size_t iHit = 0; const auto& hit : old_clust.getHits()) {
        for (std::size_t iProj = 0; const auto& proj : to_split) {

          // get track eta, phi
          const float etaProj = edm4hep::utils::eta(proj.position);
          const float phiProj = edm4hep::utils::angleAzimuthal(proj.position);

          // get hit eta, phi
          const float etaHit = edm4hep::utils::eta(hit.getPosition());
          const float phiHit = edm4hep::utils::angleAzimuthal(hit.getPosition());

          // get track momentum, distance to hit
          const float mom  = edm4hep::utils::magnitude(proj.momentum);
          const float dist = std::hypot(
            etaHit - etaProj,
            std::remainder(phiHit - phiProj, 2. * M_PI)
          );

          // set weight
          weights[iProj][iClust][iHit] = std::exp(-1. * dist / m_cfg.transverseEnergyProfileScale) * mom ;
          ++iProj;
        }

        // normalize weights over all projections
        float wTotal = 0.;
        for (const MatrixF& matrix : weights) {
          wTotal += matrix[iClust][iHit];
        }
        for (MatrixF& matrix : weights) {
          matrix[iClust][iHit] /= wTotal;
        }
        ++iHit;
      }  // end hits to merge loop
      ++iClust;
    }  // end clusters to merge loop

   // make new clusters with relevant splitting weights
   for (std::size_t iProj = 0; auto& new_clust : new_clusters) {
     make_cluster(to_merge, new_clust, weights[iProj]);
     ++iProj;
   }

  }  // end 'merge_and_split_clusters(VecClust&, VecProj&, std::vector<edm4eic::MutableCluster>&)'



  // --------------------------------------------------------------------------
  //! Make new cluster out of old ones
  // --------------------------------------------------------------------------
  void TrackClusterMergeSplitter::make_cluster(
    const VecClust& old_clusts,
    edm4eic::MutableCluster& new_clust,
    std::optional<MatrixF> split_weights
  ) const {

    // determine total no. of hits
    std::size_t nHits = 0;
    for (const auto& old_clust : old_clusts) {
      nHits += old_clust.getNhits();
    }
    new_clust.setNhits(nHits);

    float eClust = 0.;
    float wClust = 0.;
    float tClust = 0.;
    for (std::size_t iClust = 0; const auto& old_clust : old_clusts) {
      for (std::size_t iHit = 0; const auto& hit : old_clust.getHits()) {

        // get weight and update if needed
        float weight = old_clust.getHitContributions()[iHit] / hit.getEnergy();
        if (split_weights.has_value()) {
          weight *= split_weights.value()[iClust][iHit];
        }

        // update running tallies
        eClust += hit.getEnergy() * weight;
        tClust += (hit.getTime() - tClust) * (hit.getEnergy() / eClust);
        wClust += weight;

        // add hits and increment counter
        new_clust.addToHits(hit);
        new_clust.addToHitContributions(hit.getEnergy() * weight);
        ++iHit;
      }  // end hit loop
      trace("Merged input cluster {} into output cluster {}", old_clust.getObjectID().index, new_clust.getObjectID().index);
      ++iClust;
    }  // end cluster loop

    // update cluster position by taking energy-weighted
    // average of positions of clusters to merge
    edm4hep::Vector3f rClust = new_clust.getPosition();
    for (const auto& old_clust : old_clusts) {
      rClust = rClust + ((old_clust.getEnergy() / eClust) * old_clust.getPosition());
    }

    /* TODO add shape calc here */

    // set parameters
    new_clust.setEnergy(eClust);
    new_clust.setEnergyError(0.);
    new_clust.setTime(tClust);
    new_clust.setTimeError(0.);
    new_clust.setPosition(rClust);
    new_clust.setPositionError({});

  }  // end 'merge_cluster(VecClust&)'

}  // end eicrecon namespace
