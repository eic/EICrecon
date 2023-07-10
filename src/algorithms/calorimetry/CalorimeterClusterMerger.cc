// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Sylvester Joosten

#include "CalorimeterClusterMerger.h"

#include <JANA/JEvent.h>
#include <Evaluator/DD4hepUnits.h>
#include <fmt/format.h>
#include <edm4hep/MCParticle.h>
#include <edm4eic/vector_utils.h>

using namespace dd4hep;

//------------------------
// AlgorithmInit
//------------------------
void CalorimeterClusterMerger::AlgorithmInit(std::shared_ptr<spdlog::logger>& logger) {
    m_log=logger;
}

//------------------------
// AlgorithmChangeRun
//------------------------
void CalorimeterClusterMerger::AlgorithmChangeRun() {
}

//------------------------
// AlgorithmProcess
//------------------------
std::pair<std::unique_ptr<edm4eic::ClusterCollection>, std::unique_ptr<edm4eic::MCRecoClusterParticleAssociationCollection>> CalorimeterClusterMerger::AlgorithmProcess(const edm4eic::ClusterCollection &split, const edm4eic::MCRecoClusterParticleAssociationCollection &assoc) {
    auto merged = std::make_unique<edm4eic::ClusterCollection>();
    auto assoc2 = std::make_unique<edm4eic::MCRecoClusterParticleAssociationCollection>();

    if (!split.size()) {
      m_log->debug("Nothing to do for this event, returning...");
      return std::make_tuple(std::move(merged), std::move(assoc2));
    }
    m_log->debug( "Step 0/1: Getting indexed list of clusters..." );
    // get an indexed map of all vectors of clusters, indexed by mcID
    auto clusterMap = indexedClusterLists(split, assoc);
    // loop over all position clusters and match with energy clusters
    m_log->debug( "Step 1/1: Merging clusters where needed" );
    for (const auto& [mcID, clusters] : clusterMap) {
      m_log->debug( " --> Processing {} clusters for mcID {}", clusters.size(), mcID );
      auto ca = assoc2->create();
      if (clusters.size() == 1) {
        const auto& clus = clusters[0];
        m_log->debug( "   --> Only a single cluster, energy: {} for this particle, copying", clus.getEnergy());
        edm4eic::MutableCluster new_clus = clus.clone();
        merged->push_back(new_clus);

        ca.setRecID(new_clus.getObjectID().index);
        ca.setSimID(mcID);
        ca.setWeight(1.0);
        ca.setRec(new_clus);
        //ca.setSim(//FIXME);
      } else {
        edm4eic::MutableCluster new_clus = merged->create();
        // calculate aggregate info
        float energy      = 0;
        float energyError = 0;
        float time        = 0;
        int nhits = 0;
        auto position = new_clus.getPosition();
        for (auto clus : clusters) {
          m_log->debug( "   --> Adding cluster with energy: {}" , clus.getEnergy() );
          energy += clus.getEnergy();
          energyError += clus.getEnergyError() * clus.getEnergyError();
          time += clus.getTime() * clus.getEnergy();
          nhits += clus.getNhits();
          position = position + energy * clus.getPosition();
          //new_clus.addToClusters(*clus);  // FIXME: global issue with podio reference
          for (auto& hit : clus.getHits()) {
            new_clus.addToHits(hit);
          }
        }
        new_clus.setEnergy(energy);
        new_clus.setEnergyError(sqrt(energyError));
        new_clus.setTime(time / energy);
        new_clus.setNhits(nhits);
        new_clus.setPosition(position / energy);
        m_log->debug( "   --> Merged cluster with energy: {}",new_clus.getEnergy() );
        ca.setSimID(mcID);
        ca.setWeight(1.0);
        ca.setRec(edm4eic::Cluster(new_clus));
      }
    }

    // That's all!

    return std::make_tuple(std::move(merged), std::move(assoc2));
}
