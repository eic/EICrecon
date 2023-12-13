// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Derek Anderson

#ifndef EICRECON_PARTICLEFLOW_H
#define EICRECON_PARTICLEFLOW_H

#include <map>
#include <string>
#include <utility>
#include <algorithm>
#include <spdlog/spdlog.h>
// event data model definitions
#include <edm4eic/Cluster.h>
#include <edm4eic/TrackPoint.h>
#include <edm4eic/TrackCollection.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
// for algorithm configuration
#include "algorithms/interfaces/WithPodConfig.h"
#include "ParticleFlowConfig.h"

namespace eicrecon{

  // --------------------------------------------------------------------------
  //! Particle Flow
  // --------------------------------------------------------------------------
  /*! This algorithm takes a collection of tracks (and their projections)
   *  and a pair of calorimeter collections, from an ECal and an HCal,
   *  and returns a collection of "particle flow objects", reconstructed
   *  particles constructed from either tracks or calorimeter clusters.
   *
   *  As there will likely need to be multiple PF algorithms at any given
   *  time, e.g. different eta regions may need to use different algorithms,
   *  the particular choice of algorithm is set by a user-configurable
   *  flag. The `process()` call will then run whichever algorithm is
   *  specified.
   */ 
  class ParticleFlow : public WithPodConfig<ParticleFlowConfig> {

    public:

      // aliases for input types
      using TrkInput     = std::pair<const edm4eic::ReconstructedParticleCollection*, const edm4eic::TrackSegmentCollection*>;
      using CaloInput    = std::pair<const edm4eic::ClusterCollection*, const edm4eic::ClusterCollection*>;
      using CaloIDs      = std::pair<uint32_t, uint32_t>;
      using VecCaloInput = std::vector<CaloInput>;
      using VecCaloIDs   = std::vector<CaloIDs>;

      // aliases for internal types
      using ClustMap      = std::map<edm4eic::Cluster, bool>;
      using TrackMap      = std::map<edm4eic::ReconstructedParticle, bool>;
      using ProjectMap    = std::map<edm4eic::TrackSegment, bool>;
      using PointAndFound = std::pair<edm4eic::TrackPoint, bool>;

      // intermediate struct to hold info on merged calo clusters
      struct MergedCluster {
        int32_t pdg;
        float mass;
        float charge;
        float energy;
        edm4hep::Vector3f momentum;
        std::vector<edm4eic::Cluster> clusters;
      };

      // algorithm initialization
      void init(std::shared_ptr<spdlog::logger> logger);

      // primary algorithm call
      std::unique_ptr<edm4eic::ReconstructedParticleCollection> process(
        TrkInput inTrks,
        VecCaloInput vecInCalos,
        VecCaloIDs   vecCaloIDs
      );

    private:

      // particle flow algorithms
      void do_pf_alpha(const uint16_t iCaloPair, const CaloInput inCalos, const CaloIDs idCalos);

      // helper methods
      void initialize_cluster_map(const edm4eic::ClusterCollection* clusters, ClustMap& map);
      void initialize_track_map(const edm4eic::ReconstructedParticleCollection* tracks, TrackMap& map);
      void initialize_projection_map(const edm4eic::TrackSegmentCollection* projections, const std::vector<uint32_t> sysToUse, ProjectMap& map);
      void add_track_to_output(const edm4eic::ReconstructedParticle& track);
      void add_clust_to_output(const MergedCluster& merged);
      void save_unused_tracks_to_output();

      // OTHER HELPERS
      float         calculate_dist_in_eta_phi(const edm4hep::Vector3f& pntA, const edm4hep::Vector3f& pntB);
      MergedCluster merge_clusters(const MergedCluster& lhs, const MergedCluster& rhs); 
      PointAndFound find_point_at_surface(const edm4eic::TrackSegment projection, const uint32_t system, const uint64_t surface);

      // logging service
      std::shared_ptr<spdlog::logger> m_log;

      // input collections and lists
      TrkInput m_inTrks;
      VecCaloInput m_vecInCalos;
      VecCaloIDs m_vecCaloIDs;

      // output collection
      std::unique_ptr<edm4eic::ReconstructedParticleCollection> m_outPFO;

      // vectors & maps for indexing objects, etc.
      ClustMap m_ecalClustMap;
      ClustMap m_hcalClustMap;
      TrackMap m_trkMap;
      ProjectMap m_projMap;

      // class-wide constants
      const struct Constants {
        size_t   nCaloPairs;
        uint16_t iNegative;
        uint16_t iCentral;
        uint16_t iPositive;
        uint64_t innerSurface;
        int32_t  idPiPlus;
        int32_t  idPi0;
        float    massPiCharged;
        float    massPi0;
      } m_const = {3, 0, 1, 2, 1, 211, 111, 0.140, 0.135};

      // algorithm options
      enum FlowAlgo {Alpha};

  };  // end ParticleFlow definition

}  // end eicrecon namespace

#endif
