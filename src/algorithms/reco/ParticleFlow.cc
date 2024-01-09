// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Derek Anderson

#include <cmath>
#include <limits>
//#include <ranges>
#include <range/v3/view.hpp>
#include <stdexcept>
#include <functional>
#include <Math/Point3D.h>
#include <JANA/JException.h>
#include <DD4hep/DetElement.h>
// event data model related classes
#include <edm4eic/MutableReconstructedParticle.h>
// class definition and utilities
#include "ParticleFlow.h"

namespace eicrecon {

  // --------------------------------------------------------------------------
  //! Algorithm Initialization
  // -------------------------------------------------------------------------
  void ParticleFlow::init(const dd4hep::Detector* detector, std::shared_ptr<spdlog::logger>& logger) {

    // initialize logger
    m_log = logger;
    m_log -> trace("Initialized particle flow algorithm");

    // initialize detector
    m_detector = detector;
    m_log -> trace("Initialized detector");

  }  // end 'init(dd4hep::Detector*, std::shared_ptr<spdlog::logger>&)'



  // --------------------------------------------------------------------------
  //! Primary Algorithm Call
  // --------------------------------------------------------------------------
  /*! The particular algorithm to be run for a pair of calorimeters is specified
   *  by the `flowAlgo` option. Returns collection of particle flow objects, i.e.
   *  tracks or  combinations of calorimeter clusters.
   */
  std::unique_ptr<edm4eic::ReconstructedParticleCollection> ParticleFlow::process(
    const edm4eic::TrackSegmentCollection* inputProjections,
    const edm4eic::ClusterCollection* inputECalClusters,
    const edm4eic::ClusterCollection* inputHCalClusters
  ) {

    /* TODO check input types here */

    // set inputs
    m_inTrks  = inputProjections;
    m_inCalos = std::make_pair(inputECalClusters, inputHCalClusters);
    m_log -> trace("Set input collections");

    // instantiate collection to hold produced reco particles
    m_outPFO = std::make_unique<edm4eic::ReconstructedParticleCollection>();
    m_log -> trace("Running particle flow algorithm");

    // run selected algorithm
    //   - if unknown option is selected, throw exception
    switch (m_cfg.flowAlgo) {

      case FlowAlgo::Alpha:
        m_log -> trace("Running PF Alpha algorithm");
        do_pf_alpha(m_inCalos);
        break;

      default:
        m_log -> error("Unknown PF algorithm option ({}) selected!", m_cfg.flowAlgo);
        throw JException("invalid argument");
        break;
    }
    m_log -> trace("Finished running particle flow algorithm");

    // return output collection
    return std::move(m_outPFO);

  }  // end 'process(edm4eic::ReconstructedParticleCollection, edm4eic::TrackSgementCollection, edm4eic::ClusterCollection x6)'



  // --------------------------------------------------------------------------
  //! PF Alpha Algorithm
  // --------------------------------------------------------------------------
  /*! A rudimentary energy flow algorithm. This algorithm serves as a baseline against
   *  which we can compare subsequent iterations.
   *
   *  The algorithm proceeds like so:
   *    (1) Subtract projected track energy from ecal and hcal clusters
   *        (a) Identify seed projection with highest momentum at inner face
   *            of the ecal
   *        (b) Sum the energy of all track projections within ecalSumRadius or
   *            hcalSumRadius of the seed at the inner faces of the ecal and hcal
   *        (c) Sum energy of all clusters in within ecalSumRadius or hcalSumRadius
   *            of the seed
   *        (d) If the projection sum is less than the cluster sum, subtract
   *            the projection sum from the clusters and pass them on to step 2.
   *        (e) Repeat 1(a) - 1(d) until all projections have been used.
   *    (2) Combine remaining calo clusters
   *        (a) 1st combine nearby ecal + hcal clusters
   *            (i)   Identify highest-energy ecal cluster as seed
   *            (ii)  Add all ecal clusters within ecalSumRadius
   *            (iii) Add all hcal clusters within hcalSumRadius
   *            (iv)  Save combined cluster to output
   *            (v)   Repeat 2(a)(i) - 2(a)(iv) until no ecal clusters are left
   *        (b) 2nd combine remaining hcal clusters
   *            (i)   Identify highest-energy hcal cluster as seed
   *            (ii)  Add all hcal clusters within hcalSumRadius
   *            (iii) Save combined cluster to output
   *            (iv)  Repeat 2(b)(i) - 2(b)(iii) until no hcal clusters are left
   *    (3) Save all tracks to output
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
  void ParticleFlow::do_pf_alpha(const CaloInput inCalos) {

    // grab detector ids
    const uint32_t idECal = PFTools::get_detector_id(m_detector, m_cfg.ecalDetName);
    const uint32_t idHCal = PFTools::get_detector_id(m_detector, m_cfg.hcalDetName);
    m_log -> debug("Set calo pair ids: ecal = {}, hcal = {}", idECal, idHCal);

    // initialize cluster sets
    initialize_cluster_set(inCalos.first, m_ecalClustSet, m_cfg.minECalEnergy);
    initialize_cluster_set(inCalos.second, m_hcalClustSet, m_cfg.minHCalEnergy);
    m_log -> debug("Starting PFAlpha: {} ECal and {} HCal clusters to process", m_ecalClustSet.size(), m_hcalClustSet.size());

    // initialize projection set
    initialize_projection_set(m_inTrks, {idECal, idHCal}, m_projectSet, m_cfg.minTrkMomentum);
    m_log -> debug("{} projections to process", m_projectSet.size());

    // ------------------------------------------------------------------------
    // step 1
    // ------------------------------------------------------------------------
    m_log -> trace("Step 1: subtract projected track energy from ecal and hcal clusters");
    while (m_projectSet.size() >= 1) {

      // make sure vectors of nearby projections are clear
      m_nearbyECalProjectVec.clear();
      m_nearbyHCalProjectVec.clear();

      // --------------------------------------------------------------------
      // step 1(a)
      // --------------------------------------------------------------------
      m_log -> trace("Step 1(a): find highest momentum projection to act as seed");

      // 1st loop over projections
      bool foundSeed = false;
      float pSeed = std::numeric_limits<float>::min();
      edm4eic::TrackSegment seedProj;
      for (const auto projection : m_projectSet) {

        // find projection to inner face of ecal
        //   - if none, continue
        std::optional<edm4eic::TrackPoint> pntECalFace = PFTools::find_point_at_surface(projection, idECal, PFTools::consts.innerSurface);
        if (!pntECalFace.has_value()) continue;

        // check if highest momentum found so far
        const float pECalFace = edm4hep::utils::magnitude(pntECalFace.value().momentum);
        if (pECalFace > pSeed) {
          foundSeed = true;
          pSeed     = pECalFace;
          seedProj  = projection;
          m_log -> trace("Updating projection seed: new momentum is {} GeV/c", pSeed);
        }
      }  // end 1st projection loop

      // if didn't find seed projection, continue on to step 2
      if (foundSeed) {
        m_projectSet.erase(seedProj);
        m_log -> trace("Seed track projection found with momentum {} GeV/c", pSeed);
      } else {
        m_log -> warn("Did not find seed track projection out of {} remainining", m_projectSet.size());
        break;
      }

      // --------------------------------------------------------------------
      // step 1(b)
      // --------------------------------------------------------------------
      m_log -> trace("Step 1(b): sum energy of track projections within specified radius at inner face of ecal and hcal");

      std::optional<edm4eic::TrackPoint> seedAtECalFace = PFTools::find_point_at_surface(seedProj, idECal, PFTools::consts.innerSurface);
      std::optional<edm4eic::TrackPoint> seedAtHCalFace = PFTools::find_point_at_surface(seedProj, idHCal, PFTools::consts.innerSurface);

      // add seed projection to list of nearby projections at ecal face
      if (seedAtECalFace.has_value()) {
        m_nearbyECalProjectVec.push_back(seedAtECalFace.value());
      }

      // add seed projection to list of nearby projections at hcal face
      if (seedAtHCalFace.has_value()) {
        m_nearbyHCalProjectVec.push_back(seedAtHCalFace.value());
      }
      m_log -> debug("Added seeds to sum of track energies: sum at ecal = {} GeV, sum at hcal = {} GeV", PFTools::calculate_sum_of_energies(m_nearbyECalProjectVec), PFTools::calculate_sum_of_energies(m_nearbyHCalProjectVec));

      //if no more projections, continue
      if (m_projectSet.size() < 1) continue;

      // otherwise add energy of all projections within radius
      //   - FIXME this could be done more carefully:
      //       > the seed may not have a point at the HCal face
      //       > and there's no guarantee that a projection will be within the
      //         sum radius at both the ecal face AND the hcal face
      for (auto projection : m_projectSet) {

        std::optional<edm4eic::TrackPoint> projECalFace = PFTools::find_point_at_surface(projection, idECal, PFTools::consts.innerSurface);
	std::optional<edm4eic::TrackPoint> projHCalFace = PFTools::find_point_at_surface(projection, idHCal, PFTools::consts.innerSurface);

        // add track energy if projection is within ecalSumRadius of seed at ecal face
        if (projECalFace.has_value()) {
          const float dist = PFTools::calculate_dist_in_eta_phi(projECalFace.value().position, seedAtECalFace.value().position);
          if (dist < m_cfg.ecalSumRadius) {
            m_nearbyECalProjectVec.push_back(projECalFace.value());
          }
        }  // end if found point at ecal face

        // add track energy if projection is within hcalClustSumRadius of seed at hcal face
        if (projHCalFace.has_value()) {
          const float dist = PFTools::calculate_dist_in_eta_phi(projHCalFace.value().position, seedAtHCalFace.value().position);
          if (dist < m_cfg.hcalSumRadius) {
            m_nearbyHCalProjectVec.push_back(projHCalFace.value());
          }
        }  // end if found point at hcal face

        // if point at ECal or HCal face used, remove from set
        if (projECalFace.has_value() || projHCalFace.has_value()) {
          m_projectSet.erase(projection);
	}

      }  // end 2nd projection loop
      m_log -> debug("Added nearby projections to sum of track energies: sum at ecal = {} GeV, sum at hcal = {} GeV", PFTools::calculate_sum_of_energies(m_nearbyECalProjectVec), PFTools::calculate_sum_of_energies(m_nearbyHCalProjectVec));

      // --------------------------------------------------------------------
      // step 1(c)
      // --------------------------------------------------------------------
      m_log -> trace("Step 1(c): sum energy in of all calo clusters within a specified radius");

      // make sure cluster vectors are clear
      m_ecalClustsToMergeVec.clear();
      m_hcalClustsToMergeVec.clear();

      // sum energy in ecal
      if (seedAtECalFace.has_value()) {
        for (auto ecalClust : m_ecalClustSet) {

          // if in ecalSumRadius, add to sum
          const float dist = PFTools::calculate_dist_in_eta_phi(ecalClust.getPosition(), seedAtECalFace.value().position);
          if (dist < m_cfg.ecalSumRadius) {
            m_ecalClustsToMergeVec.push_back(ecalClust);
	    m_ecalClustSet.erase(ecalClust);
          }
        }  // end ecal clust loop
      }  // end if found seed point at ecal face
      m_log -> debug("Summed energy in ecal: sum = {} GeV, total no. of clusters used = {}", PFTools::calculate_sum_of_energies(m_ecalClustsToMergeVec), m_ecalClustsToMergeVec.size());

      // sum energy in hcal
      if (seedAtHCalFace.has_value()) {
        for (auto hcalClust : m_hcalClustSet) {

          // if in hcalSumRadius, add to sum
          const float dist = PFTools::calculate_dist_in_eta_phi(hcalClust.getPosition(), seedAtHCalFace.value().position);
          if (dist < m_cfg.hcalSumRadius) {
            m_hcalClustsToMergeVec.push_back(hcalClust);
	    m_hcalClustSet.erase(hcalClust);
          }
        }  // end hcal clust loop
      }  // end if found seed point at hcal face
      m_log -> debug("Summed energy in hcal: sum = {} GeV, total no. of clusters used = {}", PFTools::calculate_sum_of_energies(m_hcalClustsToMergeVec), m_hcalClustsToMergeVec.size());

      // --------------------------------------------------------------------
      // step 1(d)
      // --------------------------------------------------------------------
      m_log -> trace("Step 1(d): if calo sum is more than track sum, subtract nearby projection energy from each calo cluster");

      // do subtraction for the ecal
      for (const auto clust : m_ecalClustsToMergeVec) {

	// calculate energy sums
	const float ecalProjSum  = m_cfg.ecalFracSub * PFTools::calculate_sum_of_energies(m_nearbyECalProjectVec);
	const float ecalClustSum = PFTools::calculate_sum_of_energies(m_ecalClustsToMergeVec);

        // do subtraction if track sum is less than clust sum
	if (ecalProjSum < ecalClustSum) {

          // get energy of of nearest projection
	  const auto  project = PFTools::get_nearest_projection(m_nearbyECalProjectVec, clust.getPosition());
	  const float eProj   = PFTools::calculate_energy_at_point(project.value(), PFTools::consts.massPiCharged);
	  const float eToSub  = m_cfg.ecalFracSub * eProj;

          // if energy to subtract less than cluster, subtract and add to input set
          if (eToSub < clust.getEnergy()) {
            m_ecalClustSet.insert( PFTools::make_subtracted_cluster(clust, eToSub) );
            m_log -> trace("Adding subtracted ecal cluster to list for merging; energy = {} GeV, subtracted energy = {} GeV", clust.getEnergy(), clust.getEnergy() - eToSub);
          }
        }
      }  // end clust loop
      m_log -> debug("Finished ecal subtraction: {} clusters remain to merge", m_ecalClustSet.size());

      // now do the same for the hcal
      for (const auto clust : m_hcalClustsToMergeVec) {

	// calculate energy sums
	const float hcalProjSum  = m_cfg.hcalFracSub * PFTools::calculate_sum_of_energies(m_nearbyECalProjectVec);
	const float hcalClustSum = PFTools::calculate_sum_of_energies(m_hcalClustsToMergeVec);

        // do subtraction if track sum is less than clust sum
	if (hcalProjSum < hcalClustSum) {

          // get energy of of nearest projection
	  const auto  project = PFTools::get_nearest_projection(m_nearbyECalProjectVec, clust.getPosition());
	  const float eProj   = PFTools::calculate_energy_at_point(project.value(), PFTools::consts.massPiCharged);
	  const float eToSub  = m_cfg.hcalFracSub * eProj;

          // if energy to subtract less than cluster, subtract and add to input set
          if (eToSub < clust.getEnergy()) {
            m_hcalClustSet.insert( PFTools::make_subtracted_cluster(clust, eToSub) );
            m_log -> trace("Adding subtracted hcal cluster to list for merging; energy = {} GeV, subtracted energy = {} GeV", clust.getEnergy(), clust.getEnergy() - eToSub);
          }
        }
      }  // end clust loop
      m_log -> debug("Finished hcal subtraction: {} clusters remain to merge", m_hcalClustSet.size());

    }   // end while (m_projectSet.size() >= 1))

    // announce completion of step 1
    m_log -> trace("Step 1 complete: subtracted tracks from calorimeter clusters");

    // ------------------------------------------------------------------------
    // step 2
    // ------------------------------------------------------------------------
    //   - TODO much of this can be cleaned up by moving the repeated
    //     code to helper functions which operate on a vector/map
    //     of clusters
    m_log -> trace("Step 2: combine leftover clusters into neutral particles");

    // ------------------------------------------------------------------------
    // step 2(a)
    // ------------------------------------------------------------------------
    m_log -> trace("Step 2(a): combine ecal + hcal clusters, no. of ecal clusters left = {}, no. of hcal clusters left = {}", m_ecalClustSet.size(), m_hcalClustSet.size());

    // combine ecal & hcal clusters
    while (m_ecalClustSet.size() >= 1) {

       // prepare vectors for clustering
      m_ecalClustsToMergeVec.clear();
      m_hcalClustsToMergeVec.clear();

      // --------------------------------------------------------------------
      // step 2(a)(i)
      // --------------------------------------------------------------------
      std::optional<edm4eic::Cluster> ecalSeed;
      m_log -> trace("Step 2(a)(i): identify seed ecal cluster");

      // 1st loop over ecal clusters
      float eSeed = std::numeric_limits<float>::min();
      for (const auto ecalClust : m_ecalClustSet) {

        // update seed if needed
        if (ecalClust.getEnergy() > eSeed) {
          eSeed     = ecalClust.getEnergy();
          ecalSeed = ecalClust;
          m_log -> trace("Updated ecal seed: seed energy = {}", eSeed);
        }
      }  // end 1st ecal cluster loop

      // add seed to merged cluster and decement counters
      if (ecalSeed.has_value()) {
	m_ecalClustsToMergeVec.push_back(ecalSeed.value());
	m_ecalClustSet.erase(ecalSeed.value());
        m_log -> debug("Found seed ecal cluster with energy {}: {} ecal clusters remaining", ecalSeed.value().getEnergy(), m_ecalClustSet.size());
      } else {
        m_log -> debug("Was not able to find seed ecal cluster out of {} clusters reamining", m_ecalClustSet.size());
      }

      // add any remaining ecal clusters if needed
      if (m_ecalClustSet.size() >= 1) {

        // ------------------------------------------------------------------
        // step 2(a)(ii)
        // ------------------------------------------------------------------
        m_log -> trace("Step 2(a)(ii): add nearby ecal clusters to merged cluster");

        // 2nd loop over ecal clusters
        for (auto ecalClust : m_ecalClustSet) {

          // if in ecalSumRadius, add to combined cluste
	  const auto  centroid = PFTools::calculate_energy_weighted_centroid(m_ecalClustsToMergeVec);
          const float dist     = PFTools::calculate_dist_in_eta_phi(centroid, ecalClust.getPosition());
          if (dist < m_cfg.ecalSumRadius) {
            m_ecalClustsToMergeVec.push_back(ecalClust);
	    m_ecalClustSet.erase(ecalClust);
          }
        }  // end 2nd ecal loop
        m_log -> trace("Combined {} nearby ecal clusters for a total of energy = {} GeV: {} ecal clusters remaining", m_ecalClustsToMergeVec.size(), PFTools::calculate_sum_of_energies(m_ecalClustsToMergeVec), m_ecalClustSet.size());
      }  // end if (m_ecalClustSet.size() >= 1))

      // add any remaining hcal clusters if needed
      //   - FIXME the hcal energies need to be calibrated to make sure they're
      //     on the same footing as the ecal!
      if (m_hcalClustSet.size() >= 1) {

        // ------------------------------------------------------------------
        // step 2(a)(iii)
        // ------------------------------------------------------------------
        m_log -> trace("Step 2(a)(iii): add nearby hcal clusters to merged cluster");

        // loop over hcal clusters
        for (auto hcalClust : m_hcalClustSet) {

          // if in hcalSumRadius, add to combined cluster
          const auto  centroid = PFTools::calculate_energy_weighted_centroid(std::ranges::view::concat(m_ecalClustsToMergeVec, m_hcalClustsToMergeVec));
      	  const float dist     = PFTools::calculate_dist_in_eta_phi(std::ranges::view::concat(m_ecalClustsToMergeVec, m_hcalClustsToMergeVec), hcalClust.getPosition());
          if (dist < m_cfg.hcalSumRadius) {
            m_hcalClustsToMergeVec.push_back(hcalClust);
	    m_hcalClustSet.erase(hcalClust);
	  }
        }  // end hcal loop
        m_log -> trace("Combined {} nearby hcal clusters for a total of energy = {} GeV: {} hcal clusters remaining", m_hcalClustsToMergeVec.size(), PFTools::calculate_sum_of_energies(m_hcalClustsToMergeVec), m_hcalClustSet.size());
      }  // end if (m_hcalClustSet.size() >= 1)

      // --------------------------------------------------------------------
      // step 2(a)(iv)
      // --------------------------------------------------------------------
      m_log -> trace("Step 2(a)(iv): save combined cluster as a neutral particle.");

      add_clusters_to_output(std::ranges::view::concat(m_ecalClustsToMergeVec, m_hcalClustsToMergeVec));
      m_log -> debug("Saved merged cluster to output");
    }  // end while (nECalLeft >= 1)

    // ------------------------------------------------------------------------
    // step 2(b)
    // ------------------------------------------------------------------------
    m_log -> trace("Step 2(b): combine leftover hcal clusters; no. of hcal clusters left = {}", m_hcalClustSet.size());

    // combine leftover hcal clusters
    while (m_hcalClustSet.size() >= 1) {

      // prepare vectors for merging
      m_hcalClustsToMergeVec.clear();

      // --------------------------------------------------------------------
      // step 2(b)(i)
      // --------------------------------------------------------------------
      m_log -> trace("Step 2(b)(i): identify seed hcal cluster");
      std::optional<edm4eic::Cluster> hcalSeed;

      // 1st loop over hcal clusters
      float eSeed = std::numeric_limits<float>::min();
      for (const auto hcalClust : m_hcalClustSet) {

         // update seed if needed
        if (hcalClust.getEnergy() > eSeed) {
          eSeed    = hcalClust.getEnergy();
          hcalSeed = hcalClust;
          m_log -> trace("Updated hcal seed: seed energy = {}", eSeed);
        }
      }  // end 1st hcal cluster loop

      // add seed to merged cluster and remove from list
      if (hcalSeed.has_value()) {
        m_hcalClustsToMergeVec.push_back(hcalSeed.value());
	m_hcalClustSet.erase(hcalSeed.value());
        m_log -> debug("Found seed hcal cluster with energy {}: {} hcal clusters remaining", hcalSeed.value().getEnergy(), m_hcalClustSet.size());
      } else {
        m_log -> debug("Was not able to find seed hcal cluster out of {} clusters reamining", m_hcalClustSet.size());
      }

      if (m_hcalClustSet.size() >= 1) {

        // ------------------------------------------------------------------
        // step 2(b)(ii)
        // ------------------------------------------------------------------
        m_log -> trace("Step 2(b)(ii): add nearby hcal clusters to merged cluster");

        // 2nd loop over hcal clusters
        for (auto hcalClust : m_hcalClustSet) {

          // if in hcalSumRadius, add to combined cluster
	  const auto  centroid = PFTools::calculate_energy_weighted_centroid(m_hcalClustsToMergeVec);
          const float dist     = PFTools::calculate_dist_in_eta_phi(centroid, hcalClust.getPosition());
          if (dist < m_cfg.hcalSumRadius) {
	    m_hcalClustsToMergeVec.push_back(hcalClust);
	    m_hcalClustSet.erase(hcalClust);
          }
        }  // end 2nd hcal loop
        m_log -> trace("Combined {} nearby hcal clusters for a total of energy = {} GeV: {} hcal clusters remaining", m_hcalClustsToMergeVec.size(), PFTools::calculate_sum_of_energies(m_hcalClustsToMergeVec), m_hcalClustSet.size());
      }  // end if (m_hcalClustSet.size() >= 1)

      // --------------------------------------------------------------------
      // set 2(b)(iii)
      // --------------------------------------------------------------------
      m_log -> trace("Step 2(b)(iii): save combined cluster as a neutral particle.");

      add_clusters_to_output(m_hcalClustsToMergeVec);
      m_log -> debug("Saved merged cluster to output");
    }  // end while (m_hcalClustSet.size() >= 1)

    // announce end of PFAlpha
    m_log -> trace("PFAlpha complete!");

  }  // end 'do_pf_alpha(CaloInput)'



  // --------------------------------------------------------------------------
  //! Initialize Set of Clusters
  // --------------------------------------------------------------------------
  /*! Helper function to initialize a set of calorimeter clusters. Clusters are
   *  sorted according to decreasing energy.  If a minimum energy is provided
   *  (`minEnergy`), only clusters above the minimum will be added.
   */
  void ParticleFlow::initialize_cluster_set(const edm4eic::ClusterCollection* clustCollect, CaloSet& clustSet, std::optional<float> minEnergy) {

    // make sure set is clear
    clustSet.clear();

    // add clusters to set
    for (const auto cluster : (*clustCollect)) {

      // if energy is less than minimum, skip if needed
      if (minEnergy.has_value() && (cluster.getEnergy() < minEnergy.value())) {
        continue;
      }

      // otherwise add to set
      clustSet.insert(cluster);
    }

  }  // end 'initialize_cluster_set(edm4eic::ClusterCollection*, CaloSet&, std::optional<float>)'



  // --------------------------------------------------------------------------
  //! Initialize Set of Track Projections
  // --------------------------------------------------------------------------
  /*! Helper function to initialize a set of track projections pointing into
   *  the systems defined by the IDs listed in `sysToUse`.  If a minimum total
   *  momentum is provided (`minMomentum`), only projections with momentum
   *  above the minimum in the specified systems will be added.
   */
  void ParticleFlow::initialize_projection_set(const edm4eic::TrackSegmentCollection* projCollect, const std::vector<uint32_t> sysToUse, ProjectSet& projSet, std::optional<float> minMomentum) {

    if (sysToUse.size() == 0) {
      m_log -> error("Error! Trying to initialize projection map without specifying to what system(s)!");
      throw JException("empty list of systems to project to");
    }

    // make sure set is clear
    projSet.clear();

    // loop over projections
    for (const auto projection : (*projCollect)) {

      // loop over points in projection
      bool hasAnOkayPoint = false;
      for (const auto point : projection.getPoints()) {

        // if needed, skip point if not above threshold
        if (minMomentum.has_value() && (edm4hep::utils::magnitude(point.momentum) < minMomentum)) {
          continue;
        }

        // check if point is inside specified system
        if (PFTools::is_track_pointing_to_system(point, sysToUse)) {
          hasAnOkayPoint = true;
          break;
        }
      }  // end point loop

      // skip if no point in specified system above threshold
      if (!hasAnOkayPoint) continue;

      // otherwise, add projection to set
      projSet.insert(projection);
    }  // end projection loop

  }  // end 'intialize_track_projection_set(const edm4eic::TrackSegmentCollection*, std::vector<uint32_t>, ProjectSet&, std::optional<float>)'



  // --------------------------------------------------------------------------
  //! Add Track to PFO Collection
  // --------------------------------------------------------------------------
  /*! Helper function to add a track to output collection of
   *  particle flow objects (ReconstructedParticle's).
   *
   *  FIXME move to using tracks instead of TrackSegments when Track EDM is
   *  ready
   *
   *  FIXME fill in as much of remaining info as possible when Track EDM is
   *  ready
   */
  void ParticleFlow::add_track_to_output(const edm4eic::TrackSegment track, const uint32_t system, const uint64_t surface, std::optional<std::vector<edm4eic::Cluster>> assocClusters) {

    // grab projection at specified point
    std::optional<edm4eic::TrackPoint> projection = PFTools::find_point_at_surface(track, system, surface); 
    if (!projection.has_value()) {
      m_log -> error("Error! Trying to evaluate a track at an unavailble point! (system = {}, surface = {})", system, surface);
    }

    // create pfo in output collection
    edm4eic::MutableReconstructedParticle track_pfo = m_outPFO->create();
    track_pfo.setType( 0 );
    track_pfo.setEnergy( PFTools::calculate_energy_at_point(projection.value()) );
    track_pfo.setMomentum( projection.value().momentum );
    track_pfo.setReferencePoint( projection.value().position );
    track_pfo.setPDG( PFTools::consts.idPiP );
    track_pfo.setMass( PFTools::consts.massPiCharged );

    // link track to output pfo
    track_pfo.addToTracks(track.getTrack());

    // if present, link associated calo info
    if (assocClusters.has_value()) {
      for (auto cluster : assocClusters.value()) {
        track_pfo.addToClusters(cluster);
      }
    }

  }  // end 'add_track_to_output(edm4eic::ReconstructedParticle&)'



  // --------------------------------------------------------------------------
  //! Add Merged Cluster to PFO Collection
  // --------------------------------------------------------------------------
  /*! Helper function to combine a list of clusters into a reconstructed
   *  particle and add to output collection of particle flow objects
   *  (i.e. ReconstructedParticle's).
   *
   *  FIXME origin should be replaced with interaction vertex when
   *  calculating momentum
   *
   *  FIXME PDG and mass should be tied together. A PDG database would make that
   *  straightforward to do...
   */
  void ParticleFlow::add_clusters_to_output(std::vector<edm4eic::Cluster> clusters, std::optional<float> mass, std::optional<int32_t> pdg) {

    // create pfo in output collection
    edm4eic::MutableReconstructedParticle clust_pfo = m_outPFO->create();
    clust_pfo.setEnergy( PFTools::calculate_sum_of_energies(clusters) );
    clust_pfo.setCharge( 0. );

    // if provided, set mass
    //   - otherwise set to pi0
    if (mass.has_value()) {
      clust_pfo.setMass( mass.value() );
      clust_pfo.setMomentum( PFTools::calculate_sum_of_momenta(clusters, {0., 0., 0.}, mass.value()) );
    } else {
      clust_pfo.setMass( PFTools::consts.massPi0 );
      clust_pfo.setMomentum( PFTools::calculate_sum_of_momenta(clusters, {0., 0., 0.}, PFTools::consts.massPi0) );
    }
 
    // likewise for pdg code
    if (pdg.has_value()) {
      clust_pfo.setPDG( pdg.value() );
    } else {
      clust_pfo.setPDG( PFTools::consts.idPi0 );
    }

    // link clusters in merged cluster to output pfo
    for (const auto cluster : clusters) {
      clust_pfo.addToClusters(cluster);
    }

  }  // end 'add_clust_to_output(MergedCluster&, std::optional<float>, std::optional<int32_t>)'

}  // end eicrecon namespace
