
// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Sylvester Joosten

#ifndef _CalorimeterClusterMerger_h_
#define _CalorimeterClusterMerger_h_

#include <random>
#include <spdlog/spdlog.h>
#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <Evaluator/DD4hepUnits.h>
#include <edm4eic/Cluster.h>
#include <edm4eic/MutableCluster.h>
#include <edm4eic/MCRecoClusterParticleAssociation.h>
#include <edm4eic/MutableMCRecoClusterParticleAssociation.h>



using namespace dd4hep;



class CalorimeterClusterMerger {

protected:
    // Insert any member variables here
    std::shared_ptr<spdlog::logger> m_log;

public:
    CalorimeterClusterMerger() = default;
    ~CalorimeterClusterMerger(){} // better to use smart pointer?
    virtual void AlgorithmInit(std::shared_ptr<spdlog::logger>& logger);
    virtual void AlgorithmChangeRun() ;
    virtual void AlgorithmProcess() ;

    //-------- Configuration Parameters ------------

    //inputs
    std::vector<const edm4eic::Cluster*> m_inputClusters;//{"InputClusters", Gaudi::DataHandle::Reader, this};
    std::vector<const edm4eic::MCRecoClusterParticleAssociation*> m_inputAssociations;//{"InputAssociations", Gaudi::DataHandle::Reader, this};
  
    // Outputs
    std::vector<edm4eic::Cluster*> m_outputClusters;//{"OutputClusters", Gaudi::DataHandle::Writer, this};
    std::vector<edm4eic::MCRecoClusterParticleAssociation*> m_outputAssociations;//{"OutputAssociations", Gaudi::DataHandle::Writer, this};


private:
// get a map of MCParticle index--> std::vector<Cluster> for clusters that belong together
  std::map<int, std::vector<const edm4eic::Cluster*>> indexedClusterLists(
      std::vector<const edm4eic::Cluster*> clusters,
      std::vector<const edm4eic::MCRecoClusterParticleAssociation*> associations
  ) const {

    std::map<int, std::vector<const edm4eic::Cluster*>> matched = {};

    // loop over clusters
    for (auto cluster : clusters) {

      int mcID = -1;

      // find associated particle
      for (const auto& assoc : associations) {
          auto id = (uint32_t)((uint64_t)cluster&0xFFFFFFFF); // FIXME: This is a hack. See code near bottom of CalorimeterClusterRecoCoG::AlgorithmProcess()
        if( assoc->getRecID() == id){
          mcID = assoc->getSimID();
          break;
        }
      }

      //TODO:spdlog verbosity
      if ( m_log->level() <= spdlog::level::debug) {
        m_log->debug("--> Cluster {} has MC ID {} and energy", cluster->id(), mcID, cluster->getEnergy());
        //LOG_INFO(default_cout_logger) << " --> Found cluster with mcID " << mcID << " and energy " << cluster->getEnergy() << LOG_END;
      }

      if (mcID < 0) {
        if (m_log->level() <= spdlog::level::debug) {
          m_log->debug("   --> WARNING: no valid MC truth link found, skipping cluster...");
          //LOG_INFO(default_cout_logger) << "   --> WARNING: no valid MC truth link found, skipping cluster..." << LOG_END;
        }
        continue;
      }

      if (!matched.count(mcID)) {
        matched[mcID] = {};
      }
      matched[mcID].push_back(cluster);
    }
    return matched;
  }

};

#endif //_CalorimeterClusterMerger_h_