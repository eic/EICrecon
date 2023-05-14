// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <extensions/jana/JChainMultifactoryGeneratorT.h>

#include <factories/calorimetry/CalorimeterClusterRecoCoG_factoryT.h>

#include "RawCalorimeterHit_factory_EcalEndcapPRawHits.h"
#include "CalorimeterHit_factory_EcalEndcapPRecHits.h"
#include "ProtoCluster_factory_EcalEndcapPTruthProtoClusters.h"
#include "ProtoCluster_factory_EcalEndcapPIslandProtoClusters.h"

#include "RawCalorimeterHit_factory_EcalEndcapPInsertRawHits.h"
#include "CalorimeterHit_factory_EcalEndcapPInsertRecHits.h"
#include "ProtoCluster_factory_EcalEndcapPInsertTruthProtoClusters.h"
#include "ProtoCluster_factory_EcalEndcapPInsertIslandProtoClusters.h"

namespace eicrecon {
    class Cluster_factory_EcalEndcapPTruthClusters: public CalorimeterClusterRecoCoG_factoryT<Cluster_factory_EcalEndcapPTruthClusters> {
    public:
        template <typename... Args>
        Cluster_factory_EcalEndcapPTruthClusters(Args&&... args)
        : CalorimeterClusterRecoCoG_factoryT<Cluster_factory_EcalEndcapPTruthClusters>(std::forward<Args>(args)...) { }
    };

    class Cluster_factory_EcalEndcapPClusters: public CalorimeterClusterRecoCoG_factoryT<Cluster_factory_EcalEndcapPClusters> {
    public:
        template <typename... Args>
        Cluster_factory_EcalEndcapPClusters(Args&&... args)
        : CalorimeterClusterRecoCoG_factoryT<Cluster_factory_EcalEndcapPClusters>(std::forward<Args>(args)...) { }
    };

    class Cluster_factory_EcalEndcapPInsertTruthClusters: public CalorimeterClusterRecoCoG_factoryT<Cluster_factory_EcalEndcapPInsertTruthClusters> {
    public:
        template <typename... Args>
        Cluster_factory_EcalEndcapPInsertTruthClusters(Args&&... args)
        : CalorimeterClusterRecoCoG_factoryT<Cluster_factory_EcalEndcapPInsertTruthClusters>(std::forward<Args>(args)...) { }
    };

    class Cluster_factory_EcalEndcapPInsertClusters: public CalorimeterClusterRecoCoG_factoryT<Cluster_factory_EcalEndcapPInsertClusters> {
    public:
        template <typename... Args>
        Cluster_factory_EcalEndcapPInsertClusters(Args&&... args)
        : CalorimeterClusterRecoCoG_factoryT<Cluster_factory_EcalEndcapPInsertClusters>(std::forward<Args>(args)...) { }
    };
}

extern "C" {
    void InitPlugin(JApplication *app) {

        using namespace eicrecon;

        InitJANAPlugin(app);

        app->Add(new JFactoryGeneratorT<RawCalorimeterHit_factory_EcalEndcapPRawHits>());
        app->Add(new JFactoryGeneratorT<CalorimeterHit_factory_EcalEndcapPRecHits>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_EcalEndcapPTruthProtoClusters>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_EcalEndcapPIslandProtoClusters>());

        app->Add(
          new JChainMultifactoryGeneratorT<Cluster_factory_EcalEndcapPTruthClusters>(
             "EcalEndcapPTruthClustersWithAssociations",
            {"EcalEndcapPTruthProtoClusters",        // edm4eic::ProtoClusterCollection
             "EcalEndcapPRawHits"},                  // edm4hep::SimCalorimeterHitCollection
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
             "EcalEndcapPClustersWithAssociations",
            {"EcalEndcapPIslandProtoClusters",  // edm4eic::ProtoClusterCollection
             "EcalEndcapPRawHits"},             // edm4hep::SimCalorimeterHitCollection
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

        app->Add(new JFactoryGeneratorT<RawCalorimeterHit_factory_EcalEndcapPInsertRawHits>());
        app->Add(new JFactoryGeneratorT<CalorimeterHit_factory_EcalEndcapPInsertRecHits>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_EcalEndcapPInsertTruthProtoClusters>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_EcalEndcapPInsertIslandProtoClusters>());

        app->Add(
          new JChainMultifactoryGeneratorT<Cluster_factory_EcalEndcapPInsertTruthClusters>(
             "EcalEndcapPInsertTruthClustersWithAssociations",
            {"EcalEndcapPInsertTruthProtoClusters",        // edm4eic::ProtoClusterCollection
             "EcalEndcapPInsertRawHits"},                  // edm4hep::SimCalorimeterHitCollection
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
             "EcalEndcapPInsertClustersWithAssociations",
            {"EcalEndcapPInsertIslandProtoClusters",  // edm4eic::ProtoClusterCollection
             "EcalEndcapPInsertRawHits"},             // edm4hep::SimCalorimeterHitCollection
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
