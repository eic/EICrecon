// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Derek Anderson

#ifndef EICRECON_PARTICLEFLOW_H
#define EICRECON_PARTICLEFLOW_H

#include <algorithm>
#include <spdlog/spdlog.h>
// event data model definitions
#include <edm4eic/ReconstructedParticle.h>
#include <edm4eic/ReconstructedParticleCollection.h>
// for algorithm configuration
#include "algorithms/interfaces/WithPodConfig.h"
#include "ParticleFlowConfig.h"

namespace eicrecon{

  class ParticleFlow : public WithPodConfig<ParticleFlowConfig> {

    public:

      // algorithm initialization
      void init(std::shared_ptr<spdlog::logger> logger);

      // primary algorithm call:
      //   - the particular choice of algorithm to be used is indentified
      //     by a user-configurable flag
      //   - this method will run whichever algorithm is specified
      std::unique_ptr<edm4eic::ReconstructedParticleCollection> process();

    private:

      std::shared_ptr<spdlog::logger> m_log;

      // particle flow algorithms:
      //   - the idea is that alternate algorithms will be added here as they're
      //     developed
      //   - the hope is that testing/benchmarking will be be much easier by
      //     letting the user easily select which algorithm to use
      std::unique_ptr<edm4eic::ReconstructedParticleCollection> do_pf_alpha();

      // algorithm options
      enum FlowAlgo  {Alpha};
      enum MergeAlgo {Sum};

  };  // end ParticleFlow definition

}  // end eicrecon namespace

#endif
