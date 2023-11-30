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



  /*! Primary algorithm call.  The particular algorithm to be run for
   *  a pair of calorimeters is specified by the `flowAlgo` option. 
   */ 
  std::unique_ptr<edm4eic::ReconstructedParticleCollection> ParticleFlow::process(
        TrkInput     inTrks,
        VecCaloInput vecInCalos
  ) {

    // set inputs
    m_inTrks     = inTrks;
    m_vecInCalos = vecInCalos;

    // instantiate collection to hold produced reco particles
    m_outPars = std::make_unique<edm4eic::ReconstructedParticleCollection>();

    // loop over pairs of input calos
    for (size_t iCaloPair = 0; iCaloPair < m_const.nCaloPairs; iCaloPair++) {

      // run selected algorithm:
      //   - if unknown option is selected, throw exception
      switch (m_cfg.flowAlgo[iCaloPair]) {

        case FlowAlgo::Alpha:
          m_log -> trace(" Running PF Alpha algorithm");
          do_pf_alpha(m_vecInCalos[iCaloPair]);
          break;

        default:
          m_log -> error(" Unknown PF algorithm option ({}) selected!", m_cfg.flowAlgo[iCaloPair]);
          throw JException("invalid argument");
          break;
      }
    }  // end calo pair loop
    return std::move(m_outPars);

  }  // end 'process(ParticleFlow::ParInput, ParticleFlow::CaloInput, ParticleFlow::CaloAssoc)'




  //! PF Alpha Algorithm.
  /*! A rudimentary energy flow algorithm. This algorithm servers as a baseline against
   *  which we can compare subsequent iterations
   */ 
  void ParticleFlow::do_pf_alpha(CaloInput inCalos) {

    /* TODO
     *   - sum energy in calos
     *   - subtract reco particle energy from calo energy sums
     *   = remove clusters with no energy after subtraction
     *   - combine leftover clusters into neutral particles
     */

    return;

  }  // end 'do_pf_alpha(CaloInput)'

}  // end eicrecon namespace
