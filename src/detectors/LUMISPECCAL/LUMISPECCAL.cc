// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>
#include <JANA/JFactoryGenerator.h>


#include "RawCalorimeterHit_factory_EcalLumiSpecRawHits.h"
#include "CalorimeterHit_factory_EcalLumiSpecRecHits.h"
#include "ProtoCluster_factory_EcalLumiSpecTruthProtoClusters.h"
#include "ProtoCluster_factory_EcalLumiSpecIslandProtoClusters.h"
#include "Cluster_factory_EcalLumiSpecTruthClusters.h"
#include "Cluster_factory_EcalLumiSpecClusters.h"



extern "C" {
    void InitPlugin(JApplication *app) {
        InitJANAPlugin(app);
        app->Add(new JFactoryGeneratorT<RawCalorimeterHit_factory_EcalLumiSpecRawHits>());
        app->Add(new JFactoryGeneratorT<CalorimeterHit_factory_EcalLumiSpecRecHits>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_EcalLumiSpecTruthProtoClusters>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_EcalLumiSpecIslandProtoClusters>());
        app->Add(new JFactoryGeneratorT<Cluster_factory_EcalLumiSpecTruthClusters>());
        app->Add(new JFactoryGeneratorT<Cluster_factory_EcalLumiSpecClusters>());
        app->Add(new JFactoryGeneratorT<Association_factory_EcalLumiSpecTruthClusterAssociations>());
        app->Add(new JFactoryGeneratorT<Association_factory_EcalLumiSpecClusterAssociations>());
    }
}

