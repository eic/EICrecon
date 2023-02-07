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
#include "Cluster_factory_EcalEndcapNMergedClusters.h"

#include "RawCalorimeterHit_factory_EcalEndcapPRawHits.h"
#include "CalorimeterHit_factory_EcalEndcapPRecHits.h"
#include "ProtoCluster_factory_EcalEndcapPTruthProtoClusters.h"
#include "ProtoCluster_factory_EcalEndcapPIslandProtoClusters.h"
#include "Cluster_factory_EcalEndcapPTruthClusters.h"
#include "Cluster_factory_EcalEndcapPClusters.h"
#include "Cluster_factory_EcalEndcapPMergedClusters.h"

#include "RawCalorimeterHit_factory_EcalEndcapPInsertRawHits.h"
#include "CalorimeterHit_factory_EcalEndcapPInsertRecHits.h"
#include "ProtoCluster_factory_EcalEndcapPInsertTruthProtoClusters.h"
#include "ProtoCluster_factory_EcalEndcapPInsertIslandProtoClusters.h"
#include "Cluster_factory_EcalEndcapPInsertTruthClusters.h"
#include "Cluster_factory_EcalEndcapPInsertClusters.h"
#include "Cluster_factory_EcalEndcapPInsertMergedClusters.h"

extern "C" {
    void InitPlugin(JApplication *app) {
        InitJANAPlugin(app);
        app->Add(new JFactoryGeneratorT<RawCalorimeterHit_factory_EcalEndcapNRawHits>());
        app->Add(new JFactoryGeneratorT<CalorimeterHit_factory_EcalEndcapNRecHits>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_EcalEndcapNTruthProtoClusters>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_EcalEndcapNIslandProtoClusters>());
        app->Add(new JFactoryGeneratorT<Cluster_factory_EcalEndcapNTruthClusters>());
        app->Add(new JFactoryGeneratorT<Cluster_factory_EcalEndcapNClusters>());
        app->Add(new JFactoryGeneratorT<Cluster_factory_EcalEndcapNMergedClusters>());
        app->Add(new JFactoryGeneratorT<Association_factory_EcalEndcapNTruthClustersAssociations>());
        app->Add(new JFactoryGeneratorT<Association_factory_EcalEndcapNClustersAssociations>());
        app->Add(new JFactoryGeneratorT<Association_factory_EcalEndcapNMergedClustersAssociations>());

        app->Add(new JFactoryGeneratorT<RawCalorimeterHit_factory_EcalEndcapPRawHits>());
        app->Add(new JFactoryGeneratorT<CalorimeterHit_factory_EcalEndcapPRecHits>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_EcalEndcapPTruthProtoClusters>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_EcalEndcapPIslandProtoClusters>());
        app->Add(new JFactoryGeneratorT<Cluster_factory_EcalEndcapPTruthClusters>());
        app->Add(new JFactoryGeneratorT<Cluster_factory_EcalEndcapPClusters>());
        app->Add(new JFactoryGeneratorT<Cluster_factory_EcalEndcapPMergedClusters>());
        app->Add(new JFactoryGeneratorT<Association_factory_EcalEndcapPTruthClustersAssociations>());
        app->Add(new JFactoryGeneratorT<Association_factory_EcalEndcapPClustersAssociations>());
        app->Add(new JFactoryGeneratorT<Association_factory_EcalEndcapPMergedClustersAssociations>());

        app->Add(new JFactoryGeneratorT<RawCalorimeterHit_factory_EcalEndcapPInsertRawHits>());
        app->Add(new JFactoryGeneratorT<CalorimeterHit_factory_EcalEndcapPInsertRecHits>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_EcalEndcapPInsertTruthProtoClusters>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_EcalEndcapPInsertIslandProtoClusters>());
        app->Add(new JFactoryGeneratorT<Cluster_factory_EcalEndcapPInsertTruthClusters>());
        app->Add(new JFactoryGeneratorT<Cluster_factory_EcalEndcapPInsertClusters>());
        app->Add(new JFactoryGeneratorT<Cluster_factory_EcalEndcapPInsertMergedClusters>());
        app->Add(new JFactoryGeneratorT<Association_factory_EcalEndcapPInsertTruthClustersAssociations>());
        app->Add(new JFactoryGeneratorT<Association_factory_EcalEndcapPInsertClustersAssociations>());
        app->Add(new JFactoryGeneratorT<Association_factory_EcalEndcapPInsertMergedClustersAssociations>());
    }
}
