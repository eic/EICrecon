// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Derek Anderson

#include <stdexcept>
#include <JANA/JException.h>
// event data model related classes
#include <edm4eic/MutableReconstructedParticle.h>
// class definition
#include "ParticleFlow.h"

namespace eicrecon {

  void ParticleFlow::init(std::shared_ptr<spdlog::logger> logger) {

    m_log = logger;
    m_log -> trace(" Initialized particle flow algorithm");

  }  // end 'init(std::shared_ptr<spdlog::logger>)'



  std::unique_ptr<edm4eic::ReconstructedParticleCollection> ParticleFlow::process() {

    // instantiate collection to hold produced reco particles
    auto out_collection = std::make_unique<edm4eic::ReconstructedParticleCollection>();

    // run selected algorithm:
    //   - if unknown option is selected, throw exception
    switch (m_cfg.flowAlgo) {

      case FlowAlgo::Alpha:
        m_log -> trace(" Running PF Alpha algorithm");
        out_collection = do_pf_alpha();
        break;

      default:
        m_log -> error(" Unknown PF algorithm option ({}) selected!", m_cfg.flowAlgo);
        throw JException("invalid argument");
        break;
    }
    return out_collection;

  }  // end 'process()'



  std::unique_ptr<edm4eic::ReconstructedParticleCollection> do_pf_alpha() {

    // instantiate collection to hold produced reco particles
    auto out_collection = std::make_unique<edm4eic::ReconstructedParticleCollection>();

    /* TODO fill in alpha algorithm */

    return out_collection;

  }  // end 'do_pf_alpha()'

}  // end eicrecon namespace
