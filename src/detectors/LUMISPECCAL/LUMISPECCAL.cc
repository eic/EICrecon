// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>
#include <JANA/JFactoryGenerator.h>


#include "RawCalorimeterHit_factory_LumiSpecCalRawHits.h"
#include "CalorimeterHit_factory_LumiSpecCalRecHits.h"
#include "ProtoCluster_factory_LumiSpecCalTruthProtoClusters.h"
#include "ProtoCluster_factory_LumiSpecCalIslandProtoClusters.h"
#include "Cluster_factory_LumiSpecCalTruthClusters.h"
#include "Cluster_factory_LumiSpecCalClusters.h"
#include "Cluster_factory_LumiSpecCalMergedClusters.h"



extern "C" {
    void InitPlugin(JApplication *app) {
        InitJANAPlugin(app);
        app->Add(new JFactoryGeneratorT<RawCalorimeterHit_factory_LumiSpecCalRawHits>());
        app->Add(new JFactoryGeneratorT<CalorimeterHit_factory_LumiSpecCalRecHits>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_LumiSpecCalTruthProtoClusters>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_LumiSpecCalIslandProtoClusters>());
        app->Add(new JFactoryGeneratorT<Cluster_factory_LumiSpecCalTruthClusters>());
        app->Add(new JFactoryGeneratorT<Cluster_factory_LumiSpecCalClusters>());
        app->Add(new JFactoryGeneratorT<Cluster_factory_LumiSpecCalMergedClusters>());
        app->Add(new JFactoryGeneratorT<Association_factory_LumiSpecCalTruthClustersAssociations>());
        app->Add(new JFactoryGeneratorT<Association_factory_LumiSpecCalClustersAssociations>());
        app->Add(new JFactoryGeneratorT<Association_factory_LumiSpecCalMergedClustersAssociations>());
    }
}
    
