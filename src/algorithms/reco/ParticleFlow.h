// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Derek Anderson

#ifndef EICRECON_PARTICLEFLOW_H
#define EICRECON_PARTICLEFLOW_H

#include <map>
#include <string>
#include <utility>
#include <algorithm>
#include <spdlog/spdlog.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
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

      // ----------------------------------------------------------------------
      //! Projection Bundle
      // ----------------------------------------------------------------------
      /*! An intermediate struct to make calculations easier. Holds track
       *  projections grouped together and the running sum of energy.
       */
      struct ProjectionBundle {
        float energy = 0.;
        std::vector<edm4eic::TrackPoint> projections;
      };

      // ----------------------------------------------------------------------
      //! Merged Cluster
      // ----------------------------------------------------------------------
      /*! An intermediate struct to make calculations easier. Holds info on
       *  calo clusters merged so far.
       */
      struct MergedCluster {
        bool done = false;
        int32_t pdg = 0;
        float mass = 0.;
        float charge = 0.;
        float energy = 0.;
        edm4hep::Vector3f momentum = {0., 0., 0.};
        edm4hep::Vector3f weighted_position = {0., 0., 0.};
        std::vector<edm4eic::Cluster> clusters;
      };

      // aliases for input types
      //   - FIXME switch over to tracks when Track EDM is ready
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
      using VecClust      = std::vector<MergedCluster>;

      // algorithm initialization
      void init(std::shared_ptr<spdlog::logger> logger);

      // primary algorithm call
      std::unique_ptr<edm4eic::ReconstructedParticleCollection> process(
        TrkInput inTrks,
        VecCaloInput vecInCalos,
        VecCaloIDs   vecCaloIDs
      );

      // overloaded += for combining MergedCluster objects
      friend MergedCluster& operator+=(MergedCluster& lhs, const MergedCluster& rhs) {
        lhs.energy += rhs.energy;
        lhs.momentum = lhs.momentum + rhs.momentum;
        lhs.weighted_position = ((lhs.energy * lhs.weighted_position) + (rhs.energy * rhs.weighted_position)) / (lhs.energy + rhs.energy);
        for (const auto rhsClust : rhs.clusters) {
          lhs.clusters.push_back(rhsClust);
        }
        return lhs;
      }  // end +=(MergedCluster&, MergedCluster&)

    private:

      // particle flow algorithms
      void do_pf_alpha(const uint16_t iCaloPair, const CaloInput inCalos, const CaloIDs idCalos);

      // helper methods
      void initialize_cluster_map(const edm4eic::ClusterCollection* clusters, ClustMap& map);
      void initialize_track_map(const edm4eic::ReconstructedParticleCollection* tracks, TrackMap& map);
      void initialize_projection_map(const edm4eic::TrackSegmentCollection* projections, const std::vector<uint32_t> sysToUse, ProjectMap& map);
      void save_unused_tracks_to_output(const TrackMap& map);
      void add_track_to_output(const edm4eic::ReconstructedParticle& track);
      void add_clust_to_output(const MergedCluster& merged);
      void add_unused_clusters_to_vector(ClustMap& map, VecClust& vec);
      float calculate_energy_at_point(const edm4eic::TrackPoint& point, const float mass);
      float calculate_dist_in_eta_phi(const edm4hep::Vector3f& pntA, const edm4hep::Vector3f& pntB);
      float get_energy_of_nearest_projection(const ProjectionBundle& bundle, const edm4hep::Vector3f& position, const float mass);
      MergedCluster make_merged_cluster(const bool done, const int32_t pdg, const float mass, const float chrg, const float ene, const edm4hep::Vector3f mom, const edm4hep::Vector3f pos, const std::vector<edm4eic::Cluster> clusters);
      PointAndFound find_point_at_surface(const edm4eic::TrackSegment projection, const uint32_t system, const uint64_t surface);
      edm4hep::Vector3f calculate_momentum(const MergedCluster& clust, const edm4hep::Vector3f vertex);

      // logging service
      std::shared_ptr<spdlog::logger> m_log;

      // input collections and lists
      TrkInput m_inTrks;
      VecCaloInput m_vecInCalos;
      VecCaloIDs m_vecCaloIDs;

      // output collection
      std::unique_ptr<edm4eic::ReconstructedParticleCollection> m_outPFO;

      // vectors & maps for indexing objects, etc.
      VecClust m_ecalClustVec;
      VecClust m_hcalClustVec;
      ClustMap m_ecalClustMap;
      ClustMap m_hcalClustMap;
      TrackMap m_trkMap;
      ProjectMap m_projMap;

      // class-wide constants
      const struct Constants {
        size_t   nCaloPairs;
        uint64_t innerSurface;
        int32_t  idPi0;
        float    massPiCharged;
        float    massPi0;
      } m_const = {3, 1, 111, 0.140, 0.135};

      // ----------------------------------------------------------------------
      //! Algorithm Options
      // ----------------------------------------------------------------------
      /*! This tabulates the possible algorithms to be run.
       */
      enum FlowAlgo {Alpha};

  };  // end ParticleFlow definition

}  // end eicrecon namespace

#endif
