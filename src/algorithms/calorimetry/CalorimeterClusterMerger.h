

#ifndef _CalorimeterClusterMerger_h_
#define _CalorimeterClusterMerger_h_

#include <random>

#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <Evaluator/DD4hepUnits.h>
#include <edm4eic/Cluster.h>
#include <edm4eic/MutableCluster.h>
#include <edm4eic/MCRecoClusterParticleAssociation.h>
#include <edm4eic/MutableMCRecoClusterParticleAssociation.h>



using namespace dd4hep;



class CalorimeterClusterMerger {

    // Insert any member variables here

public:
    CalorimeterClusterMerger() = default;
    ~CalorimeterClusterMerger(){} // better to use smart pointer?
    virtual void AlgorithmInit() ;
    virtual void AlgorithmChangeRun() ;
    virtual void AlgorithmProcess() ;

    //-------- Configuration Parameters ------------
    // Name of input data type (collection)
    std::string              m_input_tag;
    std::string              m_inputAssociations_tag;

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
        if (assoc->getRec() == (*cluster)) {
          mcID = assoc->getSimID();
          break;
        }
      }
      //TODO:spdlog verbosity
      if (false) {
        LOG_INFO(default_cout_logger) << " --> Found cluster with mcID " << mcID << " and energy "
                  << cluster->getEnergy() << LOG_END;
      }

      if (mcID < 0) {
        if (false) {
          LOG_INFO(default_cout_logger) << "   --> WARNING: no valid MC truth link found, skipping cluster..." << LOG_END;
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