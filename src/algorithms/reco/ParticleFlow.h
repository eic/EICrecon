// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Derek Anderson

#ifndef EICRECON_PARTICLEFLOW_H
#define EICRECON_PARTICLEFLOW_H

#include <set>
#include <vector>
#include <string>
#include <utility>
#include <optional>
#include <algorithm>
#include <spdlog/spdlog.h>
#include <DD4hep/Detector.h>
// event data model definitions
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <edm4eic/Cluster.h>
#include <edm4eic/TrackPoint.h>
#include <edm4eic/TrackCollection.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
// for algorithm configuration
#include "algorithms/interfaces/WithPodConfig.h"
#include "ParticleFlowConfig.h"
#include "ParticleFlowTools.h"

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

      // aliases for brevity
      using TrkInput   = const edm4eic::TrackSegmentCollection*;
      using CaloInput  = std::pair<const edm4eic::ClusterCollection*, const edm4eic::ClusterCollection*>;
      using CaloSet    = std::set<edm4eic::Cluster, PFTools::is_clust_ene_greater_than>;
      using ProjectSet = std::set<edm4eic::TrackSegment, PFTools::is_project_mom_greater_than>;


      // algorithm initialization
      void init(
        const dd4hep::Detector* detector,
        std::shared_ptr<spdlog::logger>& logger
      );

      // primary algorithm call
      std::unique_ptr<edm4eic::ReconstructedParticleCollection> process(
        const edm4eic::TrackSegmentCollection* inputProjections,
        const edm4eic::ClusterCollection* inputECalClusters,
        const edm4eic::ClusterCollection* inputHCalClusters
      );

    private:

      // particle flow algorithms
      void do_pf_alpha(const CaloInput inCalos);

      // helper methods
      void initialize_cluster_set(const edm4eic::ClusterCollection* clustCollect, CaloSet& clustSet, std::optional<float> minEnergy = std::nullopt);
      void initialize_projection_set(const edm4eic::TrackSegmentCollection* projCollect, const std::vector<uint32_t> sysToUse, ProjectSet& projSet, std::optional<float> minMomentim = std::nullopt);
      void add_track_to_output(const edm4eic::TrackSegment track, const uint32_t system, const uint64_t surface, std::optional<std::vector<edm4eic::Cluster>> assocClusters = std::nullopt);
      void add_clusters_to_output(std::vector<edm4eic::Cluster> clusters, std::optional<float> mass = std::nullopt, std::optional<int32_t> pdg = std::nullopt);

      // detector & logging service
      const dd4hep::Detector* m_detector;
      std::shared_ptr<spdlog::logger> m_log;

      // input collections
      TrkInput  m_inTrks;
      CaloInput m_inCalos;

      // output collection
      std::unique_ptr<edm4eic::ReconstructedParticleCollection> m_outPFO;

      // sets for indexing input objct
      CaloSet    m_ecalClustSet;
      CaloSet    m_hcalClustSet;
      ProjectSet m_projectSet;

      // vectors for collecting projections to use or clusters to merge
      std::vector<edm4eic::TrackPoint> m_nearbyECalProjectVec;
      std::vector<edm4eic::TrackPoint> m_nearbyHCalProjectVec;
      std::vector<edm4eic::Cluster>    m_ecalClustsToMergeVec;
      std::vector<edm4eic::Cluster>    m_hcalClustsToMergeVec;

      // ----------------------------------------------------------------------
      //! Algorithm Options
      // ----------------------------------------------------------------------
      enum FlowAlgo {Alpha};

  };  // end ParticleFlow definition

}  // end eicrecon namespace

#endif
