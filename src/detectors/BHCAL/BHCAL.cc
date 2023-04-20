// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>
#include <JANA/JFactoryGenerator.h>

#include "RawCalorimeterHit_factory_HcalBarrelRawHits.h"
#include "CalorimeterHit_factory_HcalBarrelRecHits.h"
#include "ProtoCluster_factory_HcalBarrelTruthProtoClusters.h"
#include "ProtoCluster_factory_HcalBarrelIslandProtoClusters.h"
#include "Cluster_factory_HcalBarrelClusters.h"
#include "Cluster_factory_HcalBarrelTruthClusters.h"

extern "C" {
    void InitPlugin(JApplication *app) {
        InitJANAPlugin(app);

        app->Add(new JFactoryGeneratorT<RawCalorimeterHit_factory_HcalBarrelRawHits>());
        app->Add(new JFactoryGeneratorT<CalorimeterHit_factory_HcalBarrelRecHits>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_HcalBarrelTruthProtoClusters>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_HcalBarrelIslandProtoClusters>());
        app->Add(new JFactoryGeneratorT<Cluster_factory_HcalBarrelClusters>());
        app->Add(new JFactoryGeneratorT<Cluster_factory_HcalBarrelTruthClusters>());
    }
}
