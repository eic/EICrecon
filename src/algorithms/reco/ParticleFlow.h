// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Derek Anderson

#ifndef EICRECON_PARTICLEFLOW_H
#define EICRECON_PARTICLEFLOW_H

#include <set>
#include <map>
#include <utility>
#include <algorithm>
#include <spdlog/spdlog.h>
// event data model definitions
#include <edm4eic/Cluster.h>
#include <edm4eic/TrackCollection.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <edm4eic/ClusterCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
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
      using TrkInput     = const edm4eic::TrackSegmentCollection*;
      using CaloInput    = std::pair<const edm4eic::ClusterCollection*, const edm4eic::ClusterCollection*>;
      using VecCaloInput = std::vector<CaloInput>;

      // algorithm initialization
      void init(std::shared_ptr<spdlog::logger> logger);

      // primary algorithm call
      std::unique_ptr<edm4eic::ReconstructedParticleCollection> process(
        TrkInput     inTrks,
        VecCaloInput vecInCalos
      );

    private:

      std::shared_ptr<spdlog::logger> m_log;

      // input collections
      TrkInput     m_inTrks;
      VecCaloInput m_vecInCalos;

      // output collection
      std::unique_ptr<edm4eic::ReconstructedParticleCollection> m_outPars;

      // particle flow algorithms
      void do_pf_alpha(CaloInput inCalos);

      // helper methods
      // will go here...

      // class-wide constants
      const struct constants {
        size_t nCaloPairs;
        size_t iNegative;
        size_t iCentral;
        size_t iPositive;
      } m_const = {3, 0, 1, 2};

      // algorithm options
      enum FlowAlgo  {Alpha};
      enum MergeAlgo {Sum};

  };  // end ParticleFlow definition

}  // end eicrecon namespace

#endif
