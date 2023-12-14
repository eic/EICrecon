// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Derek Anderson

#include <cmath>
#include <stdexcept>
#include <functional>
#include <Math/Point3D.h>
#include <JANA/JException.h>
// event data model related classes
#include <edm4eic/MutableReconstructedParticle.h>
// class definition
#include "ParticleFlow.h"

namespace eicrecon {

  // --------------------------------------------------------------------------
  //! Algorithm Initialization
  // -------------------------------------------------------------------------
  void ParticleFlow::init(std::shared_ptr<spdlog::logger> logger) {

    // initialize logger
    m_log = logger;
    m_log -> trace("Initialized particle flow algorithm");

  }  // end 'init(std::shared_ptr<spdlog::logger>)'



  // --------------------------------------------------------------------------
  //! Primary Algorithm Call
  // --------------------------------------------------------------------------
  /*! The particular algorithm to be run for a pair of calorimeters is specified
   *  by the `flowAlgo` option. Returns collection of particle flow objects, i.e.
   *  tracks or  combinations of calorimeter clusters.
   */ 
  std::unique_ptr<edm4eic::ReconstructedParticleCollection> ParticleFlow::process(
        TrkInput     inTrks,
        VecCaloInput vecInCalos,
        VecCaloIDs   vecCaloIDs
  ) {

    // set inputs
    m_inTrks     = inTrks;
    m_vecInCalos = vecInCalos;
    m_vecCaloIDs = vecCaloIDs;
    m_log -> trace("Running particle flow algorithm");

    // instantiate collection to hold produced reco particles
    m_outPFO = std::make_unique<edm4eic::ReconstructedParticleCollection>();

    // initialize track map
    initialize_track_map(m_inTrks.first, m_trkMap);

    // loop over pairs of input calos
    for (size_t iCaloPair = 0; iCaloPair < m_const.nCaloPairs; iCaloPair++) {

      // run selected algorithm
      //   - if unknown option is selected, throw exception
      switch (m_cfg.flowAlgo[iCaloPair]) {

        case FlowAlgo::Alpha:
          m_log -> trace("Running PF Alpha algorithm for calorimeter pair #{}", iCaloPair);
          do_pf_alpha(iCaloPair, m_vecInCalos[iCaloPair], m_vecCaloIDs[iCaloPair]);
          break;

        default:
          m_log -> error("Unknown PF algorithm option ({}) selected!", m_cfg.flowAlgo[iCaloPair]);
          throw JException("invalid argument");
          break;
      }
    }  // end calo pair loop

    // save unused tracks and return output collection
    save_unused_tracks_to_output();
    m_log -> trace("Finished running particle flow algorithm");

    // return output collection
    return std::move(m_outPFO);

  }  // end 'process(ParticleFlow::TrkInput, ParticleFlow::VecCaloInput, ParticleFlow::VecCaloIDs)'



  // --------------------------------------------------------------------------
  //! PF Alpha Algorithm
  // --------------------------------------------------------------------------
  /*! A rudimentary energy flow algorithm. This algorithm serves as a baseline against
   *  which we can compare subsequent iterations.
   *
   *  In broad strokes, the algorithm goes like below.
   *    (1) Subtract projected track energy from ecal and hcal clusters by:
   *        (a) Identify a seed, the projection with highest momentum at inner face
   *            of the ecal
   *        (b) Sum the energy of all track projections within ecalSumRadius or
   *            hcalSumRadius of the seed at the inner faces of the ecal and hcal
   *        (c) Sum energy of all clusters in within ecalSumRadius or hcalSumRadius
   *            of the seed
   *        (d) If the projection sum is less than the cluster sum, subtract
   *            the projection sum from the clusters and pass them on to step 2.
   *        --> Repeat 1(a) - 1(d) until all projections have been used.
   *    (2) Combine remaining clusters by:
   *
   *  FIXME there are several clear improvements that can be made here:
   *    - integrating more PID information: PFAlpha assumes everything is pions, and we can use
   *      the composition of the merged clusters (e.g. whether or not HCal clusters are used) for
   *      further discrimination
   *    - utilizing segmentation: PFAlpha makes no use of any radial segmentation in the
   *      calorimeters which have it
   *    - using vertex information: the cluster momenta are calculated assuming the vertex
   *      is at the origin
   */ 
  void ParticleFlow::do_pf_alpha(const uint16_t iCaloPair, const CaloInput inCalos, const CaloIDs idCalos) {

    /* TODO
     *   - combine leftover clusters into neutral particles
     */

    // grab detector ids
    const uint32_t idECal = idCalos.first;
    const uint32_t idHCal = idCalos.second;

    // initialize cluster maps
    initialize_cluster_map(inCalos.first, m_ecalClustMap);
    initialize_cluster_map(inCalos.second, m_hcalClustMap);
    m_log -> debug("{} ECal and {} HCal clusters to process", m_ecalClustMap.size(), m_hcalClustMap.size());

    // initialize projection map
    initialize_projection_map(m_inTrks.second, {idECal, idHCal}, m_projMap);
    m_log -> debug("{} projections to process", m_projMap.size());

    // make sure cluster vectors are clear
    m_ecalClustVec.clear();
    m_hcalClustVec.clear();

    // step 1: subtract all track energy from ecal and hcal clusters
    //   within radius ecalSumRadius and hcalSumRadius respectively
    m_log -> trace("Step 1: subtract projected track energy from ecal and hcal clusters");

    size_t nAvailable = m_projMap.size();
    if (m_projMap.size() >= 1) {
      do {

        edm4eic::TrackSegment seedProj;
        m_log -> trace("Step 1(a): find highest momentum projection to act as seed");

        // 1st loop over projections
        bool     foundSeed = false;
        float    pSeed     = -1;
        unsigned iSeed     = 0;
        for (unsigned iProject = 0; const auto projection : m_projMap) {

          // ignore used projections
          if (projection.second) continue;

          // find projection to inner face of ecal
          //   - if none, continue
          PointAndFound pntECalFace = find_point_at_surface(projection.first, idECal, m_const.innerSurface);
          if (!pntECalFace.second) continue;

          // check if highest momentum found so far
          const float pECalFace = std::hypot(pntECalFace.first.momentum.x, pntECalFace.first.momentum.y, pntECalFace.first.momentum.z);
          if (pECalFace > pSeed) {
            foundSeed = true;
            iSeed    = iProject;
            pSeed    = pECalFace;
            seedProj = projection.first;
          }
          ++iProject;
        }  // end 1st projection loop

        // if didn't find seed projection, continue on to step 2
        if (foundSeed) {
          m_log -> debug("Seed track projection found: projection #{} with momentum {} GeV/c", iSeed, pSeed);
        } else {
          m_log -> debug("Did not find seed track projection out of {} remainining", nAvailable);
          break;
        }

        // step 1b: sum the energy of all tracks in ecalSumRadius or hcalSumRadius
        //   - FIXME this could be done more carefully: the seed may not have a
        //     point at the HCal face, and there's no guarantee that a projection
        //     will be within the sum radius at both the ecal face AND the hcal face
        PointAndFound seedAtECalFace = find_point_at_surface(seedProj, idECal, m_const.innerSurface);
        PointAndFound seedAtHCalFace = find_point_at_surface(seedProj, idHCal, m_const.innerSurface);
        m_log -> trace("Step 1(b): sum energy of track projections within specified radius at inner face of ecal and hcal");

        // add seed energy at ecal face to sum
        ProjectionBundle ecalTrkSum;
        if (seedAtECalFace.second) {
          ecalTrkSum.energy = calculate_energy_at_point(seedAtECalFace.first, m_const.massPiCharged);
          ecalTrkSum.projections.push_back(seedAtECalFace.first);
        }

        // add seed energy at hcal face to sum
        ProjectionBundle hcalTrkSum;
        if (seedAtHCalFace.second) {
          hcalTrkSum.energy = calculate_energy_at_point(seedAtHCalFace.first, m_const.massPiCharged);
          hcalTrkSum.projections.push_back(seedAtHCalFace.first);
        }
        m_log -> debug("Added seeds to sum of track energies: sum at ecal = {} GeV, sum at hcal = {} GeV", ecalTrkSum.energy, hcalTrkSum.energy);
 
        // flag seed as used and decrement no. of available projections
        m_projMap[seedProj] = true;
        --nAvailable;

        // add energy of any remaining projections to sum
        if (nAvailable >= 1) {
          for (unsigned iProject = 0; auto projection : m_projMap) {

            // ignore used projections and the seed
            if (projection.second || (iProject == iSeed)) continue;

            PointAndFound projECalFace = find_point_at_surface(projection.first, idECal, m_const.innerSurface);
            PointAndFound projHCalFace = find_point_at_surface(projection.first, idHCal, m_const.innerSurface);

            // add track energy if projection is within ecalSumRadius of seed at ecal face
            if (projECalFace.second) {
              const float dist = calculate_dist_in_eta_phi(projECalFace.first.position, seedAtECalFace.first.position);
              if (dist < m_cfg.ecalSumRadius[iCaloPair]) {
                ecalTrkSum.energy += calculate_energy_at_point(projECalFace.first, m_const.massPiCharged);
                ecalTrkSum.projections.push_back(projECalFace.first);
              }
            }  // end if found point at ecal face

            // add track energy if projection is within hcalClustSumRadius of seed at hcal face
            if (projHCalFace.second) {
              const float dist = calculate_dist_in_eta_phi(projHCalFace.first.position, seedAtHCalFace.first.position);
              if (dist < m_cfg.hcalSumRadius[iCaloPair]) {
                hcalTrkSum.energy += calculate_energy_at_point(projHCalFace.first, m_const.massPiCharged);
                hcalTrkSum.projections.push_back(projHCalFace.first);
              }
            }  // end if found point at hcal face

            // flag projection as used and decrement no. of available projections
            m_projMap[projection.first] = true;
            --nAvailable;
          }  // end 2nd projection loop
        }  // end if (nAvailable >= 1)
        m_log -> debug("Added nearby projections to sum of track energies: sum at ecal = {} GeV, sum at hcal = {} GeV", ecalTrkSum.energy, hcalTrkSum.energy);

        // step 1c: subtract sum of track energy 
        MergedCluster ecalClustSum;
        MergedCluster hcalClustSum;
        m_log -> trace("Step 1(c): sum energy in of all calo clusters within a specified radius");

        // sum energy in ecal
        if (seedAtECalFace.second) {
          for (auto ecalClust : m_ecalClustMap) {

            // ignore used clusters
            if (ecalClust.second) continue;

            // if in ecalSumRadius, add to sum
            const float dist = calculate_dist_in_eta_phi(ecalClust.first.getPosition(), seedAtECalFace.first.position);
            if (dist < m_cfg.ecalSumRadius[iCaloPair]) {
              ecalClustSum.energy += ecalClust.first.getEnergy();
              ecalClustSum.clusters.push_back(ecalClust.first);
            }
          }  // end ecal clust loop
        }  // end if found seed point at ecal face
        m_log -> debug("Summed energy in ecal: sum = {} GeV, no. of clusters used = {}", ecalClustSum.energy, ecalClustSum.clusters.size());

        // sum energy in hcal
        if (seedAtHCalFace.second) {
          for (auto hcalClust : m_hcalClustMap) {

            // ignore used clusters
            if (hcalClust.second) continue;

            // if in hcalSumRadius, add to sum
            const float dist = calculate_dist_in_eta_phi(hcalClust.first.getPosition(), seedAtHCalFace.first.position);
            if (dist < m_cfg.hcalSumRadius[iCaloPair]) {
              hcalClustSum.energy += hcalClust.first.getEnergy();
              hcalClustSum.clusters.push_back(hcalClust.first);
            }
          }  // end hcal clust loop
        }  // end if found seed point at hcal face
        m_log -> debug("Summed energy in hcal: sum = {} GeV, no. of clusters used = {}", hcalClustSum.energy, hcalClustSum.clusters.size());

        // if the projection sum is less than the calo sum,
        //   - subtract energy of nearest projection from clusters
        //   - and add subtracted cluster to vector for merging
        //
        // FIXME this could be done more carefully: it would be better
        // to subtract only the energy of the projection closest to
        // the cluster
        m_log -> trace("Step 1(d): subtract projection sum from calo sum");

        if (ecalTrkSum.energy < ecalClustSum.energy) {
          for (const auto clust : ecalClustSum.clusters) {
            const float eSub = clust.getEnergy() - ecalTrkSum.energy;
            if (eSub > 0.) {
              MergedCluster forMerging;
              forMerging.energy = eSub;
              forMerging.weighted_position = clust.getPosition();
              forMerging.clusters.push_back(clust);
              m_ecalClustVec.push_back(forMerging);
              m_log -> trace("Adding subtracted ecal cluster to list for merging; subtracted energy = {}", eSub);
            }
          }  // end clust loop
        } else {
          for (const auto clust : ecalClustSum.clusters) {
            m_ecalClustMap[clust] = true;
          }
        } 
        m_log -> debug("Added {} ecal clusters to list for merging", m_ecalClustVec.size());

        // now do the same for the hcal
        if (hcalTrkSum.energy < hcalClustSum.energy) {
          for (const auto clust : hcalClustSum.clusters) {
            const float eSub = clust.getEnergy() - hcalTrkSum.energy;
            if (eSub > 0.) {
              MergedCluster forMerging;
              forMerging.energy = eSub;
              forMerging.weighted_position = clust.getPosition();
              forMerging.clusters.push_back(clust);
              m_hcalClustVec.push_back(forMerging);
              m_log -> trace("Adding subtracted hcal cluster to list for merging; subtracted energy = {}", eSub);
            }
          }  // end clust loop
        } else {
          for (const auto clust : hcalClustSum.clusters) {
            m_hcalClustMap[clust] = true;
          }
        }
        m_log -> debug("Added {} hcal clusters to list for merging", m_hcalClustVec.size());

      }  while (nAvailable > 0);
    }  // end if (nAvailable >= 1)
    m_log -> trace("Step 1 complete: subtracted tracks from calorimeter clusters");

    /* cluster merging goes here */

  }  // end 'do_pf_alpha(uint16_t, CaloInput, CaloIDs)'



  // --------------------------------------------------------------------------
  //! Cluster Map Initialization
  // --------------------------------------------------------------------------
  /*! Helper function to initialize a map of calorimeter clusers onto whether or
   *  not they've been used.
   *
   *  TODO it might be useful to sort the cluster in order for decreasing
   *  energy for more complex algorithms.
   */
  void ParticleFlow::initialize_cluster_map(const edm4eic::ClusterCollection* clusters, ClustMap& map) {

    // make sure map is clear
    map.clear();

    // create map of cluser
    for (const auto cluster : (*clusters)) {
      map[cluster] = false;
    }

  }  // end 'initialize_cluster_map(edm4eic::ClusterCollection*, ClustMap&)'



  // --------------------------------------------------------------------------
  //! Track Map Initialization
  // --------------------------------------------------------------------------
  /*! Helper function to initialize map of tracks onto whether or not they've
   *  been used.
   *
   *  TODO it might be useful to sort tracks in order of decreasing momentum
   *  for more complex algorithms
   */
  void ParticleFlow::initialize_track_map(const edm4eic::ReconstructedParticleCollection* tracks, TrackMap& map) {

    // make sure track map is clear
    map.clear();

    // add track to map onto whether or not it's been used
    for (const auto track : (*tracks)) {
      map[track] = false;
    }

  }  // end 'intialize_track_map(edm4eic::ReconstructedParticleCollection*, TrackMap&)'



  // --------------------------------------------------------------------------
  //! Track Projection Map Initialization
  // --------------------------------------------------------------------------
  /*! Helper function to initialize map of track projections pointing into
   *  the systems defined in sysToUse onto whether or not they've been used.
   *
   *  TODO it might be useful to enable sorting the projections by their
   *  momentum at specific surfaces (e.g. the front of the ecal for a given
   *  rapidity range)
   */
  void ParticleFlow::initialize_projection_map(const edm4eic::TrackSegmentCollection* projections, const std::vector<uint32_t> sysToUse, ProjectMap& map) {

    if (sysToUse.size() == 0) {
      m_log -> error("Error! Trying to initialize projection map without specifying to what system(s)!");
      throw JException("empty list of systems to project to");
    }

    // make sure track map is clear
    map.clear();

    // loop over projections
    for (const auto projection : (*projections)) {

      // check if segment has a point inside a specified system
      bool isPointingToSys = false;
      for (const auto point : projection.getPoints()) {
        for (const auto sys : sysToUse) {
          if (point.system == sys) {
            isPointingToSys = true;
            break;
          }
        }
      }  // end point loop
 
      // if pointing to a specified system, add projection to map onto whether or not it's been used
      if (isPointingToSys) {
        map[projection] = false;
      }
    }  // end projection loop

  }  // end 'intialize_track_projection_map(const edm4eic::TrackSegmentCollection*, std::vector<uint32_t>, ProjectMap&)'



  // --------------------------------------------------------------------------
  //! Add Track to PFO Collection
  // --------------------------------------------------------------------------
  /*! Helper function to add a track to output collection of
   *  particle flow objects (ReconstructedParticle's).
   */
  void ParticleFlow::add_track_to_output(const edm4eic::ReconstructedParticle& track) {

    // create pfo in output collection
    edm4eic::MutableReconstructedParticle track_pfo = m_outPFO->create();
    track_pfo.setType( track.getType() );
    track_pfo.setEnergy( track.getEnergy() );
    track_pfo.setMomentum( track.getMomentum() );
    track_pfo.setReferencePoint( track.getReferencePoint() );
    track_pfo.setCharge( track.getCharge() );
    track_pfo.setMass( track.getMass() );
    track_pfo.setGoodnessOfPID( track.getGoodnessOfPID() );
    track_pfo.setCovMatrix( track.getCovMatrix() );
    track_pfo.setPDG( track.getPDG() );

    // link track to output pfo
    track_pfo.addToParticles(track);

  }  // end 'add_track_to_output(edm4eic::ReconstructedParticle&)'



  // --------------------------------------------------------------------------
  //! Add Merged Cluster to PFO Collection
  // --------------------------------------------------------------------------
  /*! Helper function to add a merged cluster to output collection
   *  of particle flow objects (i.e. ReconstructedParticle's).
   */
  void ParticleFlow::add_clust_to_output(const MergedCluster& merged) {

    // create pfo in output collection
    edm4eic::MutableReconstructedParticle clust_pfo = m_outPFO->create();
    clust_pfo.setEnergy( merged.energy );
    clust_pfo.setMomentum( merged.momentum );
    clust_pfo.setCharge( merged.charge );
    clust_pfo.setMass( merged.mass );
    clust_pfo.setPDG( merged.pdg );

    // link clusters in merged cluster to output pfo
    for (const auto cluster : merged.clusters) {
      clust_pfo.addToClusters(cluster);
    }

  }  // end 'add_clust_to_output(MergedCluster&)'



  // --------------------------------------------------------------------------
  //! Save Unused Tracks to PFO Collection
  // --------------------------------------------------------------------------
  /*! Helper function which loops over list of tracks, and adds any which are
   *  flagged as not having been used yet.
   */ 
  void ParticleFlow::save_unused_tracks_to_output() {

    for (const auto trkAndIsUsed : m_trkMap) {
      if (!trkAndIsUsed.second) add_track_to_output(trkAndIsUsed.first);
    }

  }  // end 'save_tracks_to_output()'



  // --------------------------------------------------------------------------
  //! Calculate Energy at Point
  // --------------------------------------------------------------------------
  /*! Helper function to calculate energy of a track projection at a
   *  point.
   */ 
  float ParticleFlow::calculate_energy_at_point(const edm4eic::TrackPoint& point, const float mass) {

    const float momentum = edm4hep::utils::magnitude(point.momentum);
    const float energy   = (momentum > 0.) ? std::hypot(momentum, mass) : 0.;
    return energy;

  }  // end 'calculate_energy_at_point(edm4hep::TrackPoint&, float)'



  // --------------------------------------------------------------------------
  //! Calculate Distance in Eta-Phi Between Two Points
  // --------------------------------------------------------------------------
  /*! Helper function to calculate distance in the eta-phi plane between
   *  two points in cartesian coordinates.
   */ 
  float ParticleFlow::calculate_dist_in_eta_phi(const edm4hep::Vector3f& pntA, const edm4hep::Vector3f& pntB) {

    ROOT::Math::XYZPoint xyzA(pntA.x, pntA.y, pntA.z);
    ROOT::Math::XYZPoint xyzB(pntB.x, pntB.y, pntB.z);

    // translate cartesian coordinates to cylindrical
    ROOT::Math::RhoEtaPhiPoint rhfA(xyzA);
    ROOT::Math::RhoEtaPhiPoint rhfB(xyzB);

    // calculate distance and return
    const float dist = std::hypot(rhfA.eta() - rhfB.eta(), rhfA.phi() - rhfA.phi());
    return dist;

  }  // end 'calculate_dist_in_eta_phi(edm4hep::Vector3f&, edm4hep::Vector3f&)'



  // --------------------------------------------------------------------------
  //! Merge Clusters (MergedCluster Input)
  // --------------------------------------------------------------------------
  /*! Helper function to merge many MergedCluster objects into one.
   */
  ParticleFlow::MergedCluster ParticleFlow::merge_clusters(const std::vector<MergedCluster>& vecToMerge) {

    // instantiate object to hold merging results
    MergedCluster merged;

    // add each cluster to merging result and return
    for (const MergedCluster toMerge : vecToMerge) {
      merged += toMerge;
    }
    return merged;

  }  // end 'merge_clusters(std::vector<MergedCluster>&)'



  // --------------------------------------------------------------------------
  //! Identify Point at a Surface
  // --------------------------------------------------------------------------
  /*! Helper function to identify a point at a particular surface in a given system.
   */ 
  ParticleFlow::PointAndFound ParticleFlow::find_point_at_surface(const edm4eic::TrackSegment projection, const uint32_t system, const uint64_t surface) {

    // instantiate object to hold point and whether or not it was found
    PointAndFound pointAndWasFound;

    // select from points comprising track projection
    pointAndWasFound.second = false;
    for (const auto point : projection.getPoints()) {
      const bool isInSystem = (point.system == system);
      const bool isAtSurface = (point.surface == surface);
      if (isInSystem && isAtSurface) {
        pointAndWasFound.first  = point;
        pointAndWasFound.second = true;
        break;
      }
    }  // end point loop  
    return pointAndWasFound;

  }  // end 'find_point_at_surface(edm4eic::TrackSegment, uint32_t, uint64_t)'



  // --------------------------------------------------------------------------
  //! Calculate Momentum for a Cluster
  // --------------------------------------------------------------------------
  /*! Helper function to calculate momentum for a cluster relative to a
   *  given vertex.
   */ 
  edm4hep::Vector3f ParticleFlow::calculate_momentum(const MergedCluster& clust, const edm4hep::Vector3f vertex) {

    // get displacement vector and magnitudes
    const auto  displace   = clust.weighted_position - vertex;
    const float rMagnitude = edm4hep::utils::magnitude(displace);
    const float pMagnitude = (clust.energy >= clust.mass) ? std::sqrt((clust.energy * clust.energy) - (clust.mass * clust.mass)) : 0.;

    // calculate momentum and return
    const edm4hep::Vector3f momentum = (pMagnitude / rMagnitude) * displace;
    return momentum;

  }  // end 'calculate_momentum(edm4eic::Cluster, edm4hep::Vector3f)'

}  // end eicrecon namespace
