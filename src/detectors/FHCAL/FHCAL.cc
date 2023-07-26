// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include "extensions/jana/JChainFactoryGeneratorT.h"
#include "extensions/jana/JChainMultifactoryGeneratorT.h"

#include "factories/calorimetry/CalorimeterClusterRecoCoG_factoryT.h"

#include "RawCalorimeterHit_factory_HcalEndcapPRawHits.h"
#include "CalorimeterHit_factory_HcalEndcapPRecHits.h"
#include "CalorimeterHit_factory_HcalEndcapPMergedHits.h"
#include "ProtoCluster_factory_HcalEndcapPTruthProtoClusters.h"
#include "ProtoCluster_factory_HcalEndcapPIslandProtoClusters.h"

#include "RawCalorimeterHit_factory_HcalEndcapPInsertRawHits.h"
#include "CalorimeterHit_factory_HcalEndcapPInsertRecHits.h"
#include "CalorimeterHit_factory_HcalEndcapPInsertMergedHits.h"
#include "ProtoCluster_factory_HcalEndcapPInsertTruthProtoClusters.h"
#include "ProtoCluster_factory_HcalEndcapPInsertIslandProtoClusters.h"

#include "RawCalorimeterHit_factory_LFHCALRawHits.h"
#include "CalorimeterHit_factory_LFHCALRecHits.h"
#include "ProtoCluster_factory_LFHCALTruthProtoClusters.h"
#include "ProtoCluster_factory_LFHCALIslandProtoClusters.h"

namespace eicrecon {
    using Cluster_factory_HcalEndcapPTruthClusters =  CalorimeterClusterRecoCoG_factoryT<>;
    using Cluster_factory_HcalEndcapPClusters = CalorimeterClusterRecoCoG_factoryT<>;
    using Cluster_factory_HcalEndcapPInsertTruthClusters =  CalorimeterClusterRecoCoG_factoryT<>;
    using Cluster_factory_HcalEndcapPInsertClusters =  CalorimeterClusterRecoCoG_factoryT<>;
    using Cluster_factory_LFHCALTruthClusters = CalorimeterClusterRecoCoG_factoryT<>;
    using Cluster_factory_LFHCALClusters = CalorimeterClusterRecoCoG_factoryT<>;
}

extern "C" {
    void InitPlugin(JApplication *app) {

        using namespace eicrecon;

        InitJANAPlugin(app);

        app->Add(new JChainFactoryGeneratorT<RawCalorimeterHit_factory_HcalEndcapPRawHits>(
          {"HcalEndcapPHits"}, "HcalEndcapPRawHits"
        ));
        app->Add(new JChainFactoryGeneratorT<CalorimeterHit_factory_HcalEndcapPRecHits>(
          {"HcalEndcapPRawHits"}, "HcalEndcapPRecHits"
        ));
        app->Add(new JChainFactoryGeneratorT<CalorimeterHit_factory_HcalEndcapPMergedHits>(
          {"HcalEndcapPRecHits"}, "HcalEndcapPMergedHits"
        ));
        app->Add(new JChainFactoryGeneratorT<ProtoCluster_factory_HcalEndcapPTruthProtoClusters>(
          {"HcalEndcapPRecHits", "HcalEndcapPHits"}, "HcalEndcapPTruthProtoClusters"
        ));
        app->Add(new JChainFactoryGeneratorT<ProtoCluster_factory_HcalEndcapPIslandProtoClusters>(
          {"HcalEndcapPRecHits"}, "HcalEndcapPIslandProtoClusters"
        ));

        app->Add(
          new JChainMultifactoryGeneratorT<Cluster_factory_HcalEndcapPTruthClusters>(
             "HcalEndcapPTruthClusters",
            {"HcalEndcapPTruthProtoClusters",        // edm4eic::ProtoClusterCollection
             "HcalEndcapPHits"},                     // edm4hep::SimCalorimeterHitCollection
            {"HcalEndcapPTruthClusters",             // edm4eic::Cluster
             "HcalEndcapPTruthClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
            {
              .energyWeight = "log",
              .moduleDimZName = "",
              .sampFrac = 1.0,
              .logWeightBase = 3.6,
              .depthCorrection = 0.0,
              .enableEtaBounds = false
            },
            app   // TODO: Remove me once fixed
          )
        );

        app->Add(
          new JChainMultifactoryGeneratorT<Cluster_factory_HcalEndcapPClusters>(
             "HcalEndcapPClusters",
            {"HcalEndcapPIslandProtoClusters",  // edm4eic::ProtoClusterCollection
             "HcalEndcapPHits"},                // edm4hep::SimCalorimeterHitCollection
            {"HcalEndcapPClusters",             // edm4eic::Cluster
             "HcalEndcapPClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
            {
              .energyWeight = "log",
              .moduleDimZName = "",
              .sampFrac = 0.033,
              .logWeightBase = 6.2,
              .depthCorrection = 0.0,
              .enableEtaBounds = false,
            },
            app   // TODO: Remove me once fixed
          )
        );

        app->Add(new JChainFactoryGeneratorT<RawCalorimeterHit_factory_HcalEndcapPInsertRawHits>(
          {"HcalEndcapPInsertHits"}, "HcalEndcapPInsertRawHits"
        ));
        app->Add(new JChainFactoryGeneratorT<CalorimeterHit_factory_HcalEndcapPInsertRecHits>(
          {"HcalEndcapPInsertRawHits"}, "HcalEndcapPInsertRecHits"
        ));
        app->Add(new JChainFactoryGeneratorT<CalorimeterHit_factory_HcalEndcapPInsertMergedHits>(
          {"HcalEndcapPInsertRecHits"}, "HcalEndcapPInsertMergedHits"
        ));
        app->Add(new JChainFactoryGeneratorT<ProtoCluster_factory_HcalEndcapPInsertTruthProtoClusters>(
          {"HcalEndcapPInsertMergedHits", "HcalEndcapPInsertHits"}, "HcalEndcapPInsertTruthProtoClusters"
        ));
        app->Add(new JChainFactoryGeneratorT<ProtoCluster_factory_HcalEndcapPInsertIslandProtoClusters>(
          {"HcalEndcapPInsertMergedHits"}, "HcalEndcapPInsertIslandProtoClusters"
        ));

        app->Add(
          new JChainMultifactoryGeneratorT<Cluster_factory_HcalEndcapPTruthClusters>(
             "HcalEndcapPInsertTruthClusters",
            {"HcalEndcapPInsertTruthProtoClusters",        // edm4eic::ProtoClusterCollection
             "HcalEndcapPInsertHits"},                     // edm4hep::SimCalorimeterHitCollection
            {"HcalEndcapPInsertTruthClusters",             // edm4eic::Cluster
             "HcalEndcapPInsertTruthClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
            {
              .energyWeight = "log",
              .moduleDimZName = "",
              .sampFrac = 1.0,
              .logWeightBase = 3.6,
              .depthCorrection = 0.0,
              .enableEtaBounds = true
            },
            app   // TODO: Remove me once fixed
          )
        );

        app->Add(
          new JChainMultifactoryGeneratorT<Cluster_factory_HcalEndcapPInsertClusters>(
             "HcalEndcapPInsertClusters",
            {"HcalEndcapPInsertIslandProtoClusters",  // edm4eic::ProtoClusterCollection
             "HcalEndcapPInsertHits"},                // edm4hep::SimCalorimeterHitCollection
            {"HcalEndcapPInsertClusters",             // edm4eic::Cluster
             "HcalEndcapPInsertClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
            {
              .energyWeight = "log",
              .moduleDimZName = "",
              .sampFrac = 1.0,
              .logWeightBase = 6.2,
              .depthCorrection = 0.0,
              .enableEtaBounds = false,
            },
            app   // TODO: Remove me once fixed
          )
        );

        app->Add(new JChainFactoryGeneratorT<RawCalorimeterHit_factory_LFHCALRawHits>(
          {"LFHCALHits"}, "LFHCALRawHits"
        ));
        app->Add(new JChainFactoryGeneratorT<CalorimeterHit_factory_LFHCALRecHits>(
          {"LFHCALRawHits"}, "LFHCALRecHits"
        ));
        app->Add(new JChainFactoryGeneratorT<ProtoCluster_factory_LFHCALTruthProtoClusters>(
          {"LFHCALRecHits", "LFHCALHits"}, "LFHCALTruthProtoClusters"
        ));
        app->Add(new JChainFactoryGeneratorT<ProtoCluster_factory_LFHCALIslandProtoClusters>(
          {"LFHCALRecHits"}, "LFHCALIslandProtoClusters"
        ));

        app->Add(
          new JChainMultifactoryGeneratorT<Cluster_factory_LFHCALTruthClusters>(
             "LFHCALTruthClusters",
            {"LFHCALTruthProtoClusters",        // edm4eic::ProtoClusterCollection
             "LFHCALHits"},                     // edm4hep::SimCalorimeterHitCollection
            {"LFHCALTruthClusters",             // edm4eic::Cluster
             "LFHCALTruthClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
            {
              .energyWeight = "log",
              .moduleDimZName = "",
              .sampFrac = 1.0,
              .logWeightBase = 4.5,
              .depthCorrection = 0.0,
              .enableEtaBounds = false
            },
            app   // TODO: Remove me once fixed
          )
        );

        app->Add(
          new JChainMultifactoryGeneratorT<Cluster_factory_LFHCALClusters>(
             "LFHCALClusters",
            {"LFHCALIslandProtoClusters",  // edm4eic::ProtoClusterCollection
             "LFHCALHits"},                // edm4hep::SimCalorimeterHitCollection
            {"LFHCALClusters",             // edm4eic::Cluster
             "LFHCALClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
            {
              .energyWeight = "log",
              .moduleDimZName = "",
              .sampFrac = 1.0,
              .logWeightBase = 4.5,
              .depthCorrection = 0.0,
              .enableEtaBounds = false,
            },
            app   // TODO: Remove me once fixed
          )
        );
    }
}
