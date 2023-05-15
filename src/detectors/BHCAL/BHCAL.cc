// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <extensions/jana/JChainMultifactoryGeneratorT.h>

#include <factories/calorimetry/CalorimeterClusterRecoCoG_factoryT.h>

#include "RawCalorimeterHit_factory_HcalBarrelRawHits.h"
#include "CalorimeterHit_factory_HcalBarrelRecHits.h"
#include "ProtoCluster_factory_HcalBarrelTruthProtoClusters.h"
#include "ProtoCluster_factory_HcalBarrelIslandProtoClusters.h"


namespace eicrecon {
    class Cluster_factory_HcalBarrelTruthClusters: public CalorimeterClusterRecoCoG_factoryT<Cluster_factory_HcalBarrelTruthClusters> {
    public:
        template <typename... Args>
        Cluster_factory_HcalBarrelTruthClusters(Args&&... args)
        : CalorimeterClusterRecoCoG_factoryT<Cluster_factory_HcalBarrelTruthClusters>(std::forward<Args>(args)...) { }
    };

    class Cluster_factory_HcalBarrelClusters: public CalorimeterClusterRecoCoG_factoryT<Cluster_factory_HcalBarrelClusters> {
    public:
        template <typename... Args>
        Cluster_factory_HcalBarrelClusters(Args&&... args)
        : CalorimeterClusterRecoCoG_factoryT<Cluster_factory_HcalBarrelClusters>(std::forward<Args>(args)...) { }
    };
}

extern "C" {
    void InitPlugin(JApplication *app) {

        using namespace eicrecon;

        InitJANAPlugin(app);

        app->Add(new JFactoryGeneratorT<RawCalorimeterHit_factory_HcalBarrelRawHits>());
        app->Add(new JFactoryGeneratorT<CalorimeterHit_factory_HcalBarrelRecHits>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_HcalBarrelTruthProtoClusters>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_HcalBarrelIslandProtoClusters>());

        app->Add(
          new JChainMultifactoryGeneratorT<Cluster_factory_HcalBarrelClusters>(
             "HcalBarrelClusters",
            {"HcalBarrelIslandProtoClusters",  // edm4eic::ProtoClusterCollection
             "HcalBarrelHits"},                // edm4hep::SimCalorimeterHitCollection
            {"HcalBarrelClusters",             // edm4eic::Cluster
             "HcalBarrelClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
            {
              .energyWeight = "log",
              .moduleDimZName = "",
              .sampFrac = 1.0,
              .logWeightBase = 6.2,
              .depthCorrection = 0.0,
              .enableEtaBounds = false
            },
            app   // TODO: Remove me once fixed
          )
        );

        app->Add(
          new JChainMultifactoryGeneratorT<Cluster_factory_HcalBarrelTruthClusters>(
             "HcalBarrelTruthClusters",
            {"HcalBarrelTruthProtoClusters",        // edm4eic::ProtoClusterCollection
             "HcalBarrelHits"},                     // edm4hep::SimCalorimeterHitCollection
            {"HcalBarrelTruthClusters",             // edm4eic::Cluster
             "HcalBarrelTruthClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
            {
              .energyWeight = "log",
              .moduleDimZName = "",
              .sampFrac = 1.0,
              .logWeightBase = 6.2,
              .depthCorrection = 0.0,
              .enableEtaBounds = false
            },
            app   // TODO: Remove me once fixed
          )
        );

    }
}
