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



  std::unique_ptr<edm4eic::ReconstructedParticleCollection> ParticleFlow::process(
    const ParticleFlow::ParInput    inPars,
    const ParticleFlow::CaloInput   inCalos,
    const ParticleFlow::CaloAssocIn inRecoCaloAssoc
  ) {

    // set inputs
    // FIXME change to vectors of calo inputs
    m_inPars          = inPars;
    m_inCalos         = inCalos;
    m_inRecoCaloAssoc = inRecoCaloAssoc;

    // instantiate collection to hold produced reco particles
    auto outPars = std::make_unique<edm4eic::ReconstructedParticleCollection>();

    // run selected algorithm:
    //   - if unknown option is selected, throw exception
    switch (m_cfg.flowAlgo) {

      case FlowAlgo::Alpha:
        m_log -> trace(" Running PF Alpha algorithm");
        outPars = do_pf_alpha();
        break;

      default:
        m_log -> error(" Unknown PF algorithm option ({}) selected!", m_cfg.flowAlgo);
        throw JException("invalid argument");
        break;
    }
    return outPars;

  }  // end 'process(ParticleFlow::ParInput, ParticleFlow::CaloInput, ParticleFlow::CaloAssoc)'




  //! PF Alpha Algorithm.
  /*! A rudimentary energy flow algorithm. This algorithm servers as a baseline against
   *  which we can compare subsequent iterations
   */ 
  std::unique_ptr<edm4eic::ReconstructedParticleCollection> ParticleFlow::do_pf_alpha() {

    // instantiate collection to hold produced reco particles
    auto outPars = std::make_unique<edm4eic::ReconstructedParticleCollection>();

    // construct cluster-particle maps
    std::map<edm4eic::Cluster, int> ecalMap = create_cluster_map(m_inPars, m_inCalos.first,  m_inRecoCaloAssoc.first);
    std::map<edm4eic::Cluster, int> hcalMap = create_cluster_map(m_inPars, m_inCalos.second, m_inRecoCaloAssoc.second);

    /* TODO
     *   - sum energy in calos
     *   - subtract reco particle energy from calo energy sums
     *   = remove clusters with no energy after subtraction
     *   - combine leftover clusters into neutral particles
     */

    return outPars;

  }  // end 'do_pf_alpha()'



  //! Construct cluster map.
  /*! This method loops over a collection of clusters and creates a map of the
   *  cluster onto the associated reconstructed particles' ID. This is done in
   *  2 steps:
   *    1. identify cluster's associated mc particle via `MCRecoClusterParticleAssociation`
   *    2. identify mc particle's associated reconstructed particle via
   *       `MCRecoParticleAssociation`
   * If no associated reconstructed particle is found, the reco particle ID
   * field is set to be -1.
   *
   * FIXME once available, this should be replaced by a map of clusters
   * onto track projections.
   */ 
  std::map<edm4eic::Cluster, int> ParticleFlow::create_cluster_map(
    const ParticleFlow::ParInput   inParticles,
    const ParticleFlow::CalCollect clusters,
    const ParticleFlow::CalMcAssoc clustMcAssoc
  ) {

    // instantiate cluster map
    std::map<edm4eic::Cluster, int> clustMap = {};
    m_log -> trace(" Preparing to construct cluster map");

    // loop over clusters and identify associated reconstruted particles
    for (const auto cluster : *clusters) {

      // 1. identify matching mc particle
      int  mcParID    = -1;
      bool foundMcPar = false;
      for (const auto mcAssoc : *clustMcAssoc) {
        if (mcAssoc.getRec() == cluster) {
          mcParID    = mcAssoc.getSimID();
          foundMcPar = true;
          break;
        }
      }  // end mc association loop

      if (foundMcPar) {
        m_log -> trace("   Found associated MC particle! Cluster energy = {}, MC particle ID = {}", cluster.getEnergy(), mcParID); 
      } else {
        m_log -> trace("   Did not find associated MC particle! Cluster energy = ()", cluster.getEnergy());
      }

      // 2. identify matching reco particle
      int  recParID    = -1;
      bool foundRecPar = false;
      if (foundMcPar) {
        for (const auto mcPar : *std::get<0>(m_inPars)) {

          // select mc particle associated to cluster
          if (mcPar.getObjectID().index != mcParID) {
            continue;
          }

          // identify associated reconstructed particle
          for (const auto recAssoc : *std::get<2>(m_inPars)) {
            if (recAssoc.getSim() == mcPar) {
              recParID    = recAssoc.getRecID();
              foundRecPar = true;
              break;
            }
          }  // end reco-mc association loop

          if (foundRecPar) {
            break;
          }
        }  // end mc particle loop

        if (foundRecPar) {
          m_log -> trace("   Found associated reco. particle! Reco particle ID = {}", recParID); 
        } else {
          m_log -> trace("   Did not find associated MC particle!");
        }
      }  // end if (foundMcPar)

      // add cluster-reco id to map
      clustMap[cluster] = recParID;
    }  // end cluster loop

    m_log -> trace(" Constructed cluster map");
    return clustMap;

  }  // end 'create_cluster_map()'

}  // end eicrecon namespace
