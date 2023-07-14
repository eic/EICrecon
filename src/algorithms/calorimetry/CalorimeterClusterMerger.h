
// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Sylvester Joosten

#pragma once

#include <random>
#include <spdlog/spdlog.h>
#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <Evaluator/DD4hepUnits.h>
#include <edm4eic/Cluster.h>
#include <edm4eic/MutableCluster.h>
#include <edm4eic/MCRecoClusterParticleAssociation.h>
#include <edm4eic/MutableMCRecoClusterParticleAssociation.h>






class CalorimeterClusterMerger {

protected:
    // Insert any member variables here
    std::shared_ptr<spdlog::logger> m_log;

public:
    CalorimeterClusterMerger() = default;
    void AlgorithmInit(std::shared_ptr<spdlog::logger>& logger);
    void AlgorithmChangeRun() ;
    void AlgorithmProcess() ;

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

      m_log->debug("Cluster {} has MC ID {} and energy", cluster->id(), mcID, cluster->getEnergy());

      if (mcID < 0) {
        m_log->warn("No valid MC truth link found, skipping cluster...");
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
