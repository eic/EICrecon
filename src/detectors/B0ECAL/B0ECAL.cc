// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <extensions/jana/JChainMultifactoryGeneratorT.h>

#include <factories/calorimetry/CalorimeterClusterRecoCoG_factoryT.h>

#include "RawCalorimeterHit_factory_B0ECalRawHits.h"
#include "CalorimeterHit_factory_B0ECalRecHits.h"
#include "ProtoCluster_factory_B0ECalTruthProtoClusters.h"
#include "ProtoCluster_factory_B0ECalIslandProtoClusters.h"


namespace eicrecon {
    class Cluster_factory_B0ECalTruthClusters: public CalorimeterClusterRecoCoG_factoryT<Cluster_factory_B0ECalTruthClusters> {
    public:
        template <typename... Args>
        Cluster_factory_B0ECalTruthClusters(Args&&... args)
        : CalorimeterClusterRecoCoG_factoryT<Cluster_factory_B0ECalTruthClusters>(std::forward<Args>(args)...) { }
    };

    class Cluster_factory_B0ECalClusters: public CalorimeterClusterRecoCoG_factoryT<Cluster_factory_B0ECalClusters> {
    public:
        template <typename... Args>
        Cluster_factory_B0ECalClusters(Args&&... args)
        : CalorimeterClusterRecoCoG_factoryT<Cluster_factory_B0ECalClusters>(std::forward<Args>(args)...) { }
    };
}


extern "C" {
    void InitPlugin(JApplication *app) {

        using namespace eicrecon;

        InitJANAPlugin(app);

        app->Add(new JFactoryGeneratorT<RawCalorimeterHit_factory_B0ECalRawHits>());
        app->Add(new JFactoryGeneratorT<CalorimeterHit_factory_B0ECalRecHits>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_B0ECalTruthProtoClusters>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_B0ECalIslandProtoClusters>());

        app->Add(
          new JChainMultifactoryGeneratorT<Cluster_factory_B0ECalClusters>(
             "B0ECalClusters",
            {"B0ECalIslandProtoClusters",  // edm4eic::ProtoClusterCollection
             "B0ECalHits"},                // edm4hep::SimCalorimeterHitCollection
            {"B0ECalClusters",             // edm4eic::Cluster
             "B0ECalClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
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
          new JChainMultifactoryGeneratorT<Cluster_factory_B0ECalTruthClusters>(
             "B0ECalTruthClusters",
            {"B0ECalTruthProtoClusters",        // edm4eic::ProtoClusterCollection
             "B0ECalHits"},                     // edm4hep::SimCalorimeterHitCollection
            {"B0ECalTruthClusters",             // edm4eic::Cluster
             "B0ECalTruthClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
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
