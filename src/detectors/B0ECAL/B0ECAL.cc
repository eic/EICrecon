// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <extensions/jana/JChainFactoryGeneratorT.h>
#include <extensions/jana/JChainMultifactoryGeneratorT.h>

#include <factories/calorimetry/CalorimeterClusterRecoCoG_factoryT.h>

#include "RawCalorimeterHit_factory_B0ECalRawHits.h"
#include "CalorimeterHit_factory_B0ECalRecHits.h"
#include "ProtoCluster_factory_B0ECalTruthProtoClusters.h"
#include "ProtoCluster_factory_B0ECalIslandProtoClusters.h"


namespace eicrecon {
    using Cluster_factory_B0ECalTruthClusters = CalorimeterClusterRecoCoG_factoryT<>;
    using Cluster_factory_B0ECalClusters = CalorimeterClusterRecoCoG_factoryT<>;
}


extern "C" {
    void InitPlugin(JApplication *app) {

        using namespace eicrecon;

        InitJANAPlugin(app);

        app->Add(new JChainFactoryGeneratorT<RawCalorimeterHit_factory_B0ECalRawHits>(
          {"B0ECalHits"}, "B0ECalRawHits"
        ));
        app->Add(new JChainFactoryGeneratorT<CalorimeterHit_factory_B0ECalRecHits>(
          {"B0ECalRawHits"}, "B0ECalRecHits"
        ));
        app->Add(new JChainFactoryGeneratorT<ProtoCluster_factory_B0ECalTruthProtoClusters>(
          {"B0ECalRecHits", "B0ECalHits"}, "B0ECalTruthProtoClusters"
        ));
        app->Add(new JChainFactoryGeneratorT<ProtoCluster_factory_B0ECalIslandProtoClusters>(
          {"B0ECalRecHits"}, "B0ECalIslandProtoClusters"
        ));

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
