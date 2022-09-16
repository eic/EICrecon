// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>
#include <JANA/JFactoryGenerator.h>


#include "RawCalorimeterHit_factory_EcalBarrelNRawHits.h"
#include "CalorimeterHit_factory_EcalBarrelNRecHits.h"
#include "ProtoCluster_factory_EcalBarrelNTruthProtoClusters.h"
#include "ProtoCluster_factory_EcalBarrelNIslandProtoClusters.h"
#include "Cluster_factory_EcalBarrelNClusters.h"
#include "Cluster_factory_EcalBarrelNMergedClusters.h"

extern "C" {
    void InitPlugin(JApplication *app) {
        InitJANAPlugin(app);
        app->Add(new JFactoryGeneratorT<RawCalorimeterHit_factory_EcalBarrelNRawHits>());
        app->Add(new JFactoryGeneratorT<CalorimeterHit_factory_EcalBarrelNRecHits>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_EcalBarrelNTruthProtoClusters>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_EcalBarrelNIslandProtoClusters>());
        app->Add(new JFactoryGeneratorT<Cluster_factory_EcalBarrelNClusters>());
        app->Add(new JFactoryGeneratorT<Cluster_factory_EcalBarrelNMergedClusters>());
    }
}
    
