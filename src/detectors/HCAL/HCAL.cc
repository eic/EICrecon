// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>
#include <JANA/JFactoryGenerator.h>

#include "RawCalorimeterHit_factory_HcalBarrelRawHits.h"
#include "CalorimeterHit_factory_HcalBarrelRecHits.h"
#include "CalorimeterHit_factory_HcalBarrelMergedHits.h"
#include "ProtoCluster_factory_HcalBarrelTruthProtoClusters.h"
#include "ProtoCluster_factory_HcalBarrelIslandProtoClusters.h"
#include "Cluster_factory_HcalBarrelClusters.h"
#include "Cluster_factory_HcalBarrelTruthClusters.h"

#include "RawCalorimeterHit_factory_HcalEndcapNRawHits.h"
#include "CalorimeterHit_factory_HcalEndcapNRecHits.h"
#include "CalorimeterHit_factory_HcalEndcapNMergedHits.h"
#include "ProtoCluster_factory_HcalEndcapNTruthProtoClusters.h"
#include "ProtoCluster_factory_HcalEndcapNIslandProtoClusters.h"
#include "Cluster_factory_HcalEndcapNClusters.h"
#include "Cluster_factory_HcalEndcapNTruthClusters.h"

#include "RawCalorimeterHit_factory_HcalEndcapPRawHits.h"
#include "CalorimeterHit_factory_HcalEndcapPRecHits.h"
#include "CalorimeterHit_factory_HcalEndcapPMergedHits.h"
#include "ProtoCluster_factory_HcalEndcapPTruthProtoClusters.h"
#include "ProtoCluster_factory_HcalEndcapPIslandProtoClusters.h"
#include "Cluster_factory_HcalEndcapPClusters.h"
#include "Cluster_factory_HcalEndcapPTruthClusters.h"
#include "Cluster_factory_HcalEndcapPMergedClusters.h"

#include "RawCalorimeterHit_factory_HcalEndcapPInsertRawHits.h"
#include "CalorimeterHit_factory_HcalEndcapPInsertRecHits.h"
#include "CalorimeterHit_factory_HcalEndcapPInsertMergedHits.h"
#include "ProtoCluster_factory_HcalEndcapPInsertTruthProtoClusters.h"
#include "ProtoCluster_factory_HcalEndcapPInsertIslandProtoClusters.h"
#include "Cluster_factory_HcalEndcapPInsertClusters.h"
#include "Cluster_factory_HcalEndcapPInsertTruthClusters.h"

#include "TrackPoint_factory_HcalEndcapNProjections.h"

extern "C" {
    void InitPlugin(JApplication *app) {
        InitJANAPlugin(app);

        app->Add(new JFactoryGeneratorT<RawCalorimeterHit_factory_HcalBarrelRawHits>());
        app->Add(new JFactoryGeneratorT<CalorimeterHit_factory_HcalBarrelRecHits>());
        app->Add(new JFactoryGeneratorT<CalorimeterHit_factory_HcalBarrelMergedHits>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_HcalBarrelTruthProtoClusters>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_HcalBarrelIslandProtoClusters>());
        app->Add(new JFactoryGeneratorT<Cluster_factory_HcalBarrelClusters>());
        app->Add(new JFactoryGeneratorT<Cluster_factory_HcalBarrelTruthClusters>());

        app->Add(new JFactoryGeneratorT<RawCalorimeterHit_factory_HcalEndcapNRawHits>());
        app->Add(new JFactoryGeneratorT<CalorimeterHit_factory_HcalEndcapNRecHits>());
        app->Add(new JFactoryGeneratorT<CalorimeterHit_factory_HcalEndcapNMergedHits>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_HcalEndcapNTruthProtoClusters>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_HcalEndcapNIslandProtoClusters>());
        app->Add(new JFactoryGeneratorT<Cluster_factory_HcalEndcapNClusters>());
        app->Add(new JFactoryGeneratorT<Cluster_factory_HcalEndcapNTruthClusters>());

        app->Add(new JFactoryGeneratorT<RawCalorimeterHit_factory_HcalEndcapPRawHits>());
        app->Add(new JFactoryGeneratorT<CalorimeterHit_factory_HcalEndcapPRecHits>());
        app->Add(new JFactoryGeneratorT<CalorimeterHit_factory_HcalEndcapPMergedHits>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_HcalEndcapPTruthProtoClusters>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_HcalEndcapPIslandProtoClusters>());
        app->Add(new JFactoryGeneratorT<Cluster_factory_HcalEndcapPClusters>());
        app->Add(new JFactoryGeneratorT<Cluster_factory_HcalEndcapPTruthClusters>());
        app->Add(new JFactoryGeneratorT<Cluster_factory_HcalEndcapPMergedClusters>());
        app->Add(new JFactoryGeneratorT<Association_factory_HcalEndcapPTruthClustersAssociations>());
        app->Add(new JFactoryGeneratorT<Association_factory_HcalEndcapPClustersAssociations>());
        app->Add(new JFactoryGeneratorT<Association_factory_HcalEndcapPMergedClustersAssociations>());

        app->Add(new JFactoryGeneratorT<RawCalorimeterHit_factory_HcalEndcapPInsertRawHits>());
        app->Add(new JFactoryGeneratorT<CalorimeterHit_factory_HcalEndcapPInsertRecHits>());
        app->Add(new JFactoryGeneratorT<CalorimeterHit_factory_HcalEndcapPInsertMergedHits>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_HcalEndcapPInsertTruthProtoClusters>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_HcalEndcapPInsertIslandProtoClusters>());
        app->Add(new JFactoryGeneratorT<Cluster_factory_HcalEndcapPInsertClusters>());
        app->Add(new JFactoryGeneratorT<Cluster_factory_HcalEndcapPInsertTruthClusters>());

        // Track propagation on HCAL
        app->Add(new JFactoryGeneratorT<TrackPoint_factory_HcalEndcapNProjections>());
    }
}
    
