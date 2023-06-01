// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>
#include <JANA/JFactoryGenerator.h>


#include "RawCalorimeterHit_factory_ZDCEcalRawHits.h"
#include "CalorimeterHit_factory_ZDCEcalRecHits.h"
#include "ProtoCluster_factory_ZDCEcalTruthProtoClusters.h"
#include "ProtoCluster_factory_ZDCEcalIslandProtoClusters.h"
#include "Cluster_factory_ZDCEcalClusters.h"
#include "Cluster_factory_ZDCEcalTruthClusters.h"

extern "C" {
    void InitPlugin(JApplication *app) {
        InitJANAPlugin(app);
        app->Add(new JFactoryGeneratorT<RawCalorimeterHit_factory_ZDCEcalRawHits>());
        app->Add(new JFactoryGeneratorT<CalorimeterHit_factory_ZDCEcalRecHits>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_ZDCEcalTruthProtoClusters>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_ZDCEcalIslandProtoClusters>());
        app->Add(new JFactoryGeneratorT<Cluster_factory_ZDCEcalClusters>());
        app->Add(new JFactoryGeneratorT<Cluster_factory_ZDCEcalTruthClusters>());
    }
}
