// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>
#include <JANA/JFactoryGenerator.h>


#include "RawCalorimeterHit_factory_EcalBarrelRawHits.h"
#include "CalorimeterHit_factory_EcalBarrelRecHits.h"
#include "ProtoCluster_factory_EcalBarrelTruthProtoClusters.h"
#include "ProtoCluster_factory_EcalBarrelIslandProtoClusters.h"
#include "Cluster_factory_EcalBarrelClusters.h"
#include "Cluster_factory_EcalBarrelMergedClusters.h"
#include "Cluster_factory_EcalBarrelTruthClusters.h"
//#include "Cluster_factory_EcalBarrelMergedTruthClusters.h"

extern "C" {
    void InitPlugin(JApplication *app) {
        InitJANAPlugin(app);
        app->Add(new JFactoryGeneratorT<RawCalorimeterHit_factory_EcalBarrelRawHits>());
        app->Add(new JFactoryGeneratorT<CalorimeterHit_factory_EcalBarrelRecHits>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_EcalBarrelTruthProtoClusters>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_EcalBarrelIslandProtoClusters>());
        app->Add(new JFactoryGeneratorT<Cluster_factory_EcalBarrelClusters>());
        app->Add(new JFactoryGeneratorT<Cluster_factory_EcalBarrelMergedClusters>());
        app->Add(new JFactoryGeneratorT<Cluster_factory_EcalBarrelTruthClusters>());
//        app->Add(new JFactoryGeneratorT<Cluster_factory_EcalBarrelMergedTruthClusters>());
    }
}
    
