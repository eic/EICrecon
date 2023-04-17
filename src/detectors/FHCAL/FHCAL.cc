// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>
#include <JANA/JFactoryGenerator.h>

#include "RawCalorimeterHit_factory_HcalEndcapPRawHits.h"
#include "CalorimeterHit_factory_HcalEndcapPRecHits.h"
#include "CalorimeterHit_factory_HcalEndcapPMergedHits.h"
#include "ProtoCluster_factory_HcalEndcapPTruthProtoClusters.h"
#include "ProtoCluster_factory_HcalEndcapPIslandProtoClusters.h"
#include "Cluster_factory_HcalEndcapPClusters.h"
#include "Cluster_factory_HcalEndcapPTruthClusters.h"

#include "RawCalorimeterHit_factory_HcalEndcapPInsertRawHits.h"
#include "CalorimeterHit_factory_HcalEndcapPInsertRecHits.h"
#include "CalorimeterHit_factory_HcalEndcapPInsertMergedHits.h"
#include "ProtoCluster_factory_HcalEndcapPInsertTruthProtoClusters.h"
#include "ProtoCluster_factory_HcalEndcapPInsertIslandProtoClusters.h"
#include "Cluster_factory_HcalEndcapPInsertClusters.h"
#include "Cluster_factory_HcalEndcapPInsertTruthClusters.h"

#include "RawCalorimeterHit_factory_LFHCALRawHits.h"
#include "CalorimeterHit_factory_LFHCALRecHits.h"
#include "ProtoCluster_factory_LFHCALTruthProtoClusters.h"
#include "ProtoCluster_factory_LFHCALIslandProtoClusters.h"
#include "Cluster_factory_LFHCALClusters.h"
#include "Cluster_factory_LFHCALTruthClusters.h"

extern "C" {
    void InitPlugin(JApplication *app) {
        InitJANAPlugin(app);

        app->Add(new JFactoryGeneratorT<RawCalorimeterHit_factory_HcalEndcapPRawHits>());
        app->Add(new JFactoryGeneratorT<CalorimeterHit_factory_HcalEndcapPRecHits>());
        app->Add(new JFactoryGeneratorT<CalorimeterHit_factory_HcalEndcapPMergedHits>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_HcalEndcapPTruthProtoClusters>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_HcalEndcapPIslandProtoClusters>());
        app->Add(new JFactoryGeneratorT<Cluster_factory_HcalEndcapPClusters>());
        app->Add(new JFactoryGeneratorT<Cluster_factory_HcalEndcapPTruthClusters>());
        app->Add(new JFactoryGeneratorT<Association_factory_HcalEndcapPTruthClusterAssociations>());
        app->Add(new JFactoryGeneratorT<Association_factory_HcalEndcapPClusterAssociations>());

        app->Add(new JFactoryGeneratorT<RawCalorimeterHit_factory_HcalEndcapPInsertRawHits>());
        app->Add(new JFactoryGeneratorT<CalorimeterHit_factory_HcalEndcapPInsertRecHits>());
        app->Add(new JFactoryGeneratorT<CalorimeterHit_factory_HcalEndcapPInsertMergedHits>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_HcalEndcapPInsertTruthProtoClusters>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_HcalEndcapPInsertIslandProtoClusters>());
        app->Add(new JFactoryGeneratorT<Cluster_factory_HcalEndcapPInsertClusters>());
        app->Add(new JFactoryGeneratorT<Cluster_factory_HcalEndcapPInsertTruthClusters>());

        app->Add(new JFactoryGeneratorT<RawCalorimeterHit_factory_LFHCALRawHits>());
        app->Add(new JFactoryGeneratorT<CalorimeterHit_factory_LFHCALRecHits>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_LFHCALTruthProtoClusters>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_LFHCALIslandProtoClusters>());
        app->Add(new JFactoryGeneratorT<Cluster_factory_LFHCALClusters>());
        app->Add(new JFactoryGeneratorT<Cluster_factory_LFHCALTruthClusters>());

    }
}
