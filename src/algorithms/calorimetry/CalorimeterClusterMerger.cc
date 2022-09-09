

#include "CalorimeterClusterMerger.h"

#include <JANA/JEvent.h>
#include <Evaluator/DD4hepUnits.h>
#include <fmt/format.h>
#include <edm4hep/MCParticle.h>
#include <edm4eic/vector_utils.h>

using namespace dd4hep;

//this algorithm converted from https://eicweb.phy.anl.gov/EIC/juggler/-/blob/master/JugReco/src/components/CalorimeterHitReco.cpp


//------------------------
// AlgorithmInit
//------------------------
void CalorimeterClusterMerger::AlgorithmInit() {

    return;
}

//------------------------
// AlgorithmChangeRun
//------------------------
void CalorimeterClusterMerger::AlgorithmChangeRun() {
}

//------------------------
// AlgorithmProcess
//------------------------
void CalorimeterClusterMerger::AlgorithmProcess() {
    // input
    const auto& split = m_inputClusters;
    const auto& assoc = m_inputAssociations;
    // output
    auto& merged = m_outputClusters;
    auto& assoc2 = m_outputAssociations;

    if (!split.size()) {
      if (false) {
        LOG_INFO(default_cout_logger) << "Nothing to do for this event, returning..." << LOG_END;
      }
      return;
    }

    if (false) {
      LOG_INFO(default_cout_logger) << "Step 0/1: Getting indexed list of clusters..." << LOG_END;
    }
    // get an indexed map of all vectors of clusters, indexed by mcID
    auto clusterMap = indexedClusterLists(split, assoc);

    // loop over all position clusters and match with energy clusters
    if (false) {
      LOG_INFO(default_cout_logger) << "Step 1/1: Merging clusters where needed" << LOG_END;
    }
    for (const auto& [mcID, clusters] : clusterMap) {
      if (false) {
        LOG_INFO(default_cout_logger) << " --> Processing " << clusters.size() << " clusters for mcID " << mcID << LOG_END;
      }
      if (clusters.size() == 1) {
        const auto& clus = clusters[0];
        if (false) {
          LOG_INFO(default_cout_logger) << "   --> Only a single cluster, energy: " << clus->getEnergy()
                  << " for this particle, copying" << LOG_END;
        }
        auto nclus= clus->clone();
        edm4eic::Cluster* new_clus = new edm4eic::Cluster(nclus);
        merged.push_back(new_clus);
        
        auto ca = new edm4eic::MutableMCRecoClusterParticleAssociation();
        ca->setRecID(new_clus->getObjectID().index);
        ca->setSimID(mcID);
        ca->setWeight(1.0);
        ca->setRec(*new_clus);
        edm4eic::MCRecoClusterParticleAssociation* toadd = new edm4eic::MCRecoClusterParticleAssociation(*ca);
        assoc2.push_back(toadd);
        //ca.setSim(//FIXME);
      } else {
        edm4eic::MutableCluster* new_clus = new edm4eic::MutableCluster();// merged.create();
        // calculate aggregate info
        float energy      = 0;
        float energyError = 0;
        float time        = 0;
        int nhits = 0;
        auto position = new_clus->getPosition();
        for (auto clus : clusters) {
          if (false) {
            LOG_INFO(default_cout_logger) << "   --> Adding cluster with energy: " << clus->getEnergy() << LOG_END;
          }
          energy += clus->getEnergy();
          energyError += clus->getEnergyError() * clus->getEnergyError();
          time += clus->getTime() * clus->getEnergy();
          nhits += clus->getNhits();
          position = position + energy * clus->getPosition();
          new_clus->addToClusters(*clus);
          for (auto& hit : clus->getHits()) {
            new_clus->addToHits(hit);
          }
        }
        new_clus->setEnergy(energy);
        new_clus->setEnergyError(sqrt(energyError));
        new_clus->setTime(time / energy);
        new_clus->setNhits(nhits);
        new_clus->setPosition(position / energy);
        if (false) {
          LOG_INFO(default_cout_logger) << "   --> Merged cluster with energy: " << new_clus->getEnergy() << LOG_END;
        }
        auto ca = new edm4eic::MutableMCRecoClusterParticleAssociation();
        ca->setSimID(mcID);
        ca->setWeight(1.0);
        ca->setRec(edm4eic::Cluster(*new_clus));
        edm4eic::MCRecoClusterParticleAssociation* toadd = new edm4eic::MCRecoClusterParticleAssociation(*ca);
        assoc2.push_back(toadd);
      }
    }

    // That's all!
    
}
