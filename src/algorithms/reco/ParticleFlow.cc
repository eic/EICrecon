// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Derek Anderson

#include <set>
#include <cmath>
#include <stdexcept>
#include <functional>
#include <Math/Point3D.h>
#include <JANA/JException.h>
// event data model related classes
#include <edm4eic/Track.h>
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

      std::cout << "Processing calo pair (" << iCaloPair << ")..." << std::endl;

      // run selected algorithm:
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
   *  The algorithm is:
   *
   *  FIXME there are several clear improvements that can be made here:
   *    - integrating more PID information: PFAlpha assumes everything is pions, and we can use
   *      the composition of the merged clusters (e.g. whether or not HCal clusters are used) for
   *      further discrimination
   *    - utilizing segmentation: PFAlpha makes no use of any radial segmentation in the
   *      calorimeters which have it
   */ 
  void ParticleFlow::do_pf_alpha(const uint16_t iCaloPair, const CaloInput inCalos, const CaloIDs idCalos) {

    /* TODO
     *   - sum energy in calos
     *   - subtract reco particle energy from calo energy sums
     *   - remove clusters with no energy after subtraction
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
            iSeed  = iProject;
            pSeed  = pECalFace;
            seedProj  = projection.first;
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
        PointAndFound seedAtECalFace = find_point_at_surface(seedProj, idECal, m_const.innerSurface);
        PointAndFound seedAtHCalFace = find_point_at_surface(seedProj, idHCal, m_const.innerSurface);
        m_log -> trace("Step 1(b): sum energy of track projections within specified radius at inner face of ecal and hcal");

        // add seed energy at ecal face to sum
        float pTrkSumAtECal = 0.;
        if (seedAtECalFace.second) {
          const float pTrkSeedAtECal = std::hypot(seedAtECalFace.first.momentum.x, seedAtECalFace.first.momentum.y, seedAtECalFace.first.momentum.z);
          if (pTrkSeedAtECal > 0.) pTrkSumAtECal += std::hypot(pTrkSeedAtECal, m_const.massPiCharged);
        }

        // add seed energy at hcal face to sum
        float pTrkSumAtHCal = 0.;
        if (seedAtHCalFace.second) {
          const float pTrkSeedAtHCal = std::hypot(seedAtHCalFace.first.momentum.x, seedAtHCalFace.first.momentum.y, seedAtHCalFace.first.momentum.z);
          if (pTrkSeedAtHCal > 0.) pTrkSumAtHCal += std::hypot(pTrkSeedAtHCal, m_const.massPiCharged);
        }
        m_log -> debug("Added seeds to sum of track energies: sum at ecal = {} GeV, sum at hcal = {} GeV", pTrkSumAtECal, pTrkSumAtHCal);
 
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
              const float dist        = calculate_dist_in_eta_phi(projECalFace.first.position, seedAtECalFace.first.position);
              const float pProjAtECal = std::hypot(projECalFace.first.momentum.x, projECalFace.first.momentum.y, projECalFace.first.momentum.z);
              if ((dist < m_cfg.ecalSumRadius[iCaloPair]) && (pProjAtECal > 0.)) {
                pTrkSumAtECal += std::hypot(pProjAtECal, m_const.massPiCharged);
              }
            }  // end if found point at ecal face

            // add track energy if projection is within hcalSumRadius of seed at hcal face
            if (projHCalFace.second) {
              const float dist        = calculate_dist_in_eta_phi(projHCalFace.first.position, seedAtHCalFace.first.position);
              const float pProjAtHCal = std::hypot(projHCalFace.first.momentum.x, projHCalFace.first.momentum.y, projHCalFace.first.momentum.z);
              if ((dist < m_cfg.hcalSumRadius[iCaloPair]) && (pProjAtHCal > 0.)) {
                pTrkSumAtHCal += std::hypot(pProjAtHCal, m_const.massPiCharged);
              }
            }  // end if found point at hcal face

            // flag projection as used and decrement no. of available projections
            m_projMap[projection.first] = true;
            --nAvailable;
          }  // end 2nd projection loop
        }  // end if (nAvailable >= 1)
        m_log -> debug("Added nearby projections to sum of track energies: sum at ecal = {} GeV, sum at hcal = {} GeV", pTrkSumAtECal, pTrkSumAtHCal);

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

    // check if at same radius
    //   - issue warning if not
    if (rhfA.rho() != rhfB.rho()) m_log -> error("calculating eta-phi distance between 2 points at different radii ({} vs. {})", rhfA.rho(), rhfB.rho());

    // calculate distance and return
    const float dist = std::hypot(rhfA.eta() - rhfB.eta(), rhfA.phi() - rhfA.phi());
    return dist;

  }  // end 'calculate_dist_in_eta_phi(edm4hep::Vector3f&, edm4hep::Vector3f&)'



  // --------------------------------------------------------------------------
  //! Merge Two Clusters
  // --------------------------------------------------------------------------
  /*! Helper function to merge two clusters into one.
   */
  ParticleFlow::MergedCluster ParticleFlow::merge_clusters(const MergedCluster& lhs, const MergedCluster& rhs) {

    // instantiate object to hold merging results and make sure members are empty
    MergedCluster merged;
    merged.pdg = 0;
    merged.mass = 0.;
    merged.charge = 0.;
    merged.energy = 0.;
    merged.momentum = {0., 0., 0.};
    merged.clusters.clear();

    // add lhs to output
    merged.energy     += lhs.energy;
    merged.momentum.x += lhs.momentum.x;
    merged.momentum.y += lhs.momentum.y;
    merged.momentum.z += lhs.momentum.z;
    for (const auto lhsClust : lhs.clusters) {
      merged.clusters.push_back(lhsClust);
    }

    // add rhs to output
    merged.energy     += rhs.energy;
    merged.momentum.x += rhs.momentum.x;
    merged.momentum.y += rhs.momentum.y;
    merged.momentum.z += rhs.momentum.z;
    for (const auto rhsClust : rhs.clusters) {
      merged.clusters.push_back(rhsClust);
    }

    // calculate mass of combined cluster
    // and return output
    merged.mass = std::sqrt((merged.energy * merged.energy) - std::hypot(merged.momentum.x, merged.momentum.y, merged.momentum.z));
    return merged;

  }  // end 'merge_clusters(MergedCluster&, MergedCluster&)'



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

}  // end eicrecon namespace
