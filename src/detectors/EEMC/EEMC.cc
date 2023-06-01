// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>
#include <JANA/JFactoryGenerator.h>


#include "RawCalorimeterHit_factory_EcalEndcapNRawHits.h"
#include "CalorimeterHit_factory_EcalEndcapNRecHits.h"
#include "ProtoCluster_factory_EcalEndcapNTruthProtoClusters.h"
#include "ProtoCluster_factory_EcalEndcapNIslandProtoClusters.h"
#include "Cluster_factory_EcalEndcapNTruthClusters.h"
#include "Cluster_factory_EcalEndcapNClusters.h"

extern "C" {
    void InitPlugin(JApplication *app) {
        InitJANAPlugin(app);
        app->Add(new JFactoryGeneratorT<RawCalorimeterHit_factory_EcalEndcapNRawHits>());
        app->Add(new JFactoryGeneratorT<CalorimeterHit_factory_EcalEndcapNRecHits>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_EcalEndcapNTruthProtoClusters>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_EcalEndcapNIslandProtoClusters>());
        app->Add(new JFactoryGeneratorT<Cluster_factory_EcalEndcapNTruthClusters>());
        app->Add(new JFactoryGeneratorT<Cluster_factory_EcalEndcapNClusters>());
        app->Add(new JFactoryGeneratorT<Association_factory_EcalEndcapNTruthClusterAssociations>());
        app->Add(new JFactoryGeneratorT<Association_factory_EcalEndcapNClusterAssociations>());
    }
}
