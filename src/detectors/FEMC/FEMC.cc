// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include "extensions/jana/JChainFactoryGeneratorT.h"
#include "extensions/jana/JChainMultifactoryGeneratorT.h"

#include "factories/calorimetry/CalorimeterClusterRecoCoG_factoryT.h"

#include "RawCalorimeterHit_factory_EcalEndcapPRawHits.h"
#include "CalorimeterHit_factory_EcalEndcapPRecHits.h"
#include "ProtoCluster_factory_EcalEndcapPTruthProtoClusters.h"
#include "ProtoCluster_factory_EcalEndcapPIslandProtoClusters.h"

#include "RawCalorimeterHit_factory_EcalEndcapPInsertRawHits.h"
#include "CalorimeterHit_factory_EcalEndcapPInsertRecHits.h"
#include "ProtoCluster_factory_EcalEndcapPInsertTruthProtoClusters.h"
#include "ProtoCluster_factory_EcalEndcapPInsertIslandProtoClusters.h"

namespace eicrecon {
    using Cluster_factory_EcalEndcapPTruthClusters = CalorimeterClusterRecoCoG_factoryT<>;
    using Cluster_factory_EcalEndcapPClusters = CalorimeterClusterRecoCoG_factoryT<>;
    using Cluster_factory_EcalEndcapPInsertTruthClusters = CalorimeterClusterRecoCoG_factoryT<>;
    using Cluster_factory_EcalEndcapPInsertClusters = CalorimeterClusterRecoCoG_factoryT<>;
}

extern "C" {
    void InitPlugin(JApplication *app) {

        using namespace eicrecon;

        InitJANAPlugin(app);

        app->Add(new JChainFactoryGeneratorT<RawCalorimeterHit_factory_EcalEndcapPRawHits>(
          {"EcalEndcapPHits"}, "EcalEndcapPRawHits"
        ));
        app->Add(new JChainFactoryGeneratorT<CalorimeterHit_factory_EcalEndcapPRecHits>(
          {"EcalEndcapPRawHits"}, "EcalEndcapPRecHits"
        ));
        app->Add(new JChainFactoryGeneratorT<ProtoCluster_factory_EcalEndcapPTruthProtoClusters>(
          {"EcalEndcapPRecHits", "EcalEndcapPHits"}, "EcalEndcapPTruthProtoClusters"
        ));
        app->Add(new JChainFactoryGeneratorT<ProtoCluster_factory_EcalEndcapPIslandProtoClusters>(
          {"EcalEndcapPRecHits"}, "EcalEndcapPIslandProtoClusters"
        ));

        app->Add(
          new JChainMultifactoryGeneratorT<Cluster_factory_EcalEndcapPTruthClusters>(
             "EcalEndcapPTruthClusters",
            {"EcalEndcapPTruthProtoClusters",        // edm4eic::ProtoClusterCollection
             "EcalEndcapPHits"},                     // edm4hep::SimCalorimeterHitCollection
            {"EcalEndcapPTruthClusters",             // edm4eic::Cluster
             "EcalEndcapPTruthClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
            {
              .energyWeight = "log",
              .moduleDimZName = "",
              .sampFrac = 1.0,
              .logWeightBase = 6.2,
              .depthCorrection = 0.0,
              .enableEtaBounds = true
            },
            app   // TODO: Remove me once fixed
          )
        );

        app->Add(
          new JChainMultifactoryGeneratorT<Cluster_factory_EcalEndcapPClusters>(
             "EcalEndcapPClusters",
            {"EcalEndcapPIslandProtoClusters",  // edm4eic::ProtoClusterCollection
             "EcalEndcapPHits"},                // edm4hep::SimCalorimeterHitCollection
            {"EcalEndcapPClusters",             // edm4eic::Cluster
             "EcalEndcapPClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
            {
              .energyWeight = "log",
              .moduleDimZName = "",
              .sampFrac = 1.0,
              .logWeightBase = 3.6,
              .depthCorrection = 0.0,
              .enableEtaBounds = false,
            },
            app   // TODO: Remove me once fixed
          )
        );

        app->Add(new JChainFactoryGeneratorT<RawCalorimeterHit_factory_EcalEndcapPInsertRawHits>(
          {"EcalEndcapPInsertHits"}, "EcalEndcapPInsertRawHits"
        ));
        app->Add(new JChainFactoryGeneratorT<CalorimeterHit_factory_EcalEndcapPInsertRecHits>(
          {"EcalEndcapPInsertRawHits"}, "EcalEndcapPInsertRecHits"
        ));
        app->Add(new JChainFactoryGeneratorT<ProtoCluster_factory_EcalEndcapPInsertTruthProtoClusters>(
          {"EcalEndcapPInsertRecHits", "EcalEndcapPInsertHits"}, "EcalEndcapPInsertTruthProtoClusters"
        ));
        app->Add(new JChainFactoryGeneratorT<ProtoCluster_factory_EcalEndcapPInsertIslandProtoClusters>(
          {"EcalEndcapPInsertRecHits"}, "EcalEndcapPInsertIslandProtoClusters"
        ));

        app->Add(
          new JChainMultifactoryGeneratorT<Cluster_factory_EcalEndcapPInsertTruthClusters>(
             "EcalEndcapPInsertTruthClusters",
            {"EcalEndcapPInsertTruthProtoClusters",        // edm4eic::ProtoClusterCollection
             "EcalEndcapPInsertHits"},                     // edm4hep::SimCalorimeterHitCollection
            {"EcalEndcapPInsertTruthClusters",             // edm4eic::Cluster
             "EcalEndcapPInsertTruthClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
            {
              .energyWeight = "log",
              .moduleDimZName = "",
              .sampFrac = 1.0,
              .logWeightBase = 6.2,
              .depthCorrection = 0.0,
              .enableEtaBounds = true
            },
            app   // TODO: Remove me once fixed
          )
        );

        app->Add(
          new JChainMultifactoryGeneratorT<Cluster_factory_EcalEndcapPInsertClusters>(
             "EcalEndcapPInsertClusters",
            {"EcalEndcapPInsertIslandProtoClusters",  // edm4eic::ProtoClusterCollection
             "EcalEndcapPInsertHits"},                // edm4hep::SimCalorimeterHitCollection
            {"EcalEndcapPInsertClusters",             // edm4eic::Cluster
             "EcalEndcapPInsertClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
            {
              .energyWeight = "log",
              .moduleDimZName = "",
              .sampFrac = 1.0,
              .logWeightBase = 3.6,
              .depthCorrection = 0.0,
              .enableEtaBounds = false,
            },
            app   // TODO: Remove me once fixed
          )
        );

    }
}
