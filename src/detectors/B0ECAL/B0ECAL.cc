// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>
#include <JANA/JFactoryGenerator.h>


#include "RawCalorimeterHit_factory_B0ECalRawHits.h"
#include "CalorimeterHit_factory_B0ECalRecHits.h"
#include "ProtoCluster_factory_B0ECalTruthProtoClusters.h"
#include "ProtoCluster_factory_B0ECalIslandProtoClusters.h"
#include "Cluster_factory_B0ECalClusters.h"
#include "TruthCluster_factory_B0ECalTruthProtoClusters.h"


extern "C" {
    void InitPlugin(JApplication *app) {
        InitJANAPlugin(app);
        app->Add(new JFactoryGeneratorT<RawCalorimeterHit_factory_B0ECalRawHits>());
        app->Add(new JFactoryGeneratorT<CalorimeterHit_factory_B0ECalRecHits>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_B0ECalTruthProtoClusters>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_B0ECalIslandProtoClusters>());
        app->Add(new JFactoryGeneratorT<Cluster_factory_B0ECalClusters>());
        app->Add(new JFactoryGeneratorT<TruthCluster_factory_B0ECalTruthProtoClusters>());
    }
}
