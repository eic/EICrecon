// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Derek Anderson

#ifndef EICRECON_PARTICLEFLOW_H
#define EICRECON_PARTICLEFLOW_H

#include <map>
#include <utility>
#include <algorithm>
#include <spdlog/spdlog.h>
// event data model definitions
#include <edm4eic/Cluster.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4eic/MCRecoClusterParticleAssociationCollection.h>
#include <edm4hep/MCParticleCollection.h>
// for algorithm configuration
#include "algorithms/interfaces/WithPodConfig.h"
#include "ParticleFlowConfig.h"

namespace eicrecon{

  //! Particle Flow
  /*! This algorithm takes a collection of tracks (and their projections)
   *  and a pair of calorimeter collections, from an ECal and an HCal,
   *  and returns a collection of reconstructed particles.
   *
   *  As there will likely need to be multiple PF algorithms at any given
   *  time, the particular choice of algorithm is set by a user-configurable
   *  flag. The `process()` call will then run whichever algorithm is
   *  specified.
   *
   *  This flexibility will hopefully allow for easier testing/benchmarking.
   */ 
  class ParticleFlow : public WithPodConfig<ParticleFlowConfig> {

    public:

      // aliases for brevity
      using CalCollect  = edm4eic::ClusterCollection*;
      using CalMcAssoc  = edm4eic::MCRecoClusterParticleAssociationCollection*;
      using ParInput    = std::tuple<edm4hep::MCParticleCollection*, edm4eic::ReconstructedParticleCollection*, edm4eic::MCRecoParticleAssociationCollection*>;
      using CaloInput   = std::pair<edm4eic::ClusterCollection*, edm4eic::ClusterCollection*>;
      using CaloAssocIn = std::pair<edm4eic::MCRecoClusterParticleAssociationCollection*, edm4eic::MCRecoClusterParticleAssociationCollection*>;

      // algorithm initialization
      void init(std::shared_ptr<spdlog::logger> logger);

      // primary algorithm call
      // - FIXME the mc/reco particle input will need to be replaced
      //   with tracks once ready
      // - FIXME the reco particle/cluster association input will
      //   need to be replaced with track projections once ready
      std::unique_ptr<edm4eic::ReconstructedParticleCollection> process(
        const ParInput    inPars,
        const CaloInput   inCalos,
        const CaloAssocIn inRecoCaloAssoc
      );

    private:

      std::shared_ptr<spdlog::logger> m_log;

      // input collections
      ParInput    m_inPars;
      CaloInput   m_inCalos;
      CaloAssocIn m_inRecoCaloAssoc;

      // particle flow algorithms
      std::unique_ptr<edm4eic::ReconstructedParticleCollection> do_pf_alpha();

      // helper methods
      std::map<edm4eic::Cluster, int> create_cluster_map(
        const ParInput   inParticles,
        const CalCollect clusters,
        const CalMcAssoc clustMcAssoc
      );

      // algorithm options
      enum FlowAlgo  {Alpha};
      enum MergeAlgo {Sum};

  };  // end ParticleFlow definition

}  // end eicrecon namespace

#endif
