// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include "extensions/jana/JChainFactoryGeneratorT.h"
#include "extensions/jana/JChainMultifactoryGeneratorT.h"

#include "factories/calorimetry/CalorimeterClusterRecoCoG_factoryT.h"

#include "RawCalorimeterHit_factory_HcalBarrelRawHits.h"
#include "CalorimeterHit_factory_HcalBarrelRecHits.h"
#include "ProtoCluster_factory_HcalBarrelTruthProtoClusters.h"
#include "ProtoCluster_factory_HcalBarrelIslandProtoClusters.h"


namespace eicrecon {
    using Cluster_factory_HcalBarrelTruthClusters = CalorimeterClusterRecoCoG_factoryT<>;
    using Cluster_factory_HcalBarrelClusters = CalorimeterClusterRecoCoG_factoryT<>;
}

extern "C" {
    void InitPlugin(JApplication *app) {

        using namespace eicrecon;

        InitJANAPlugin(app);

        app->Add(new JChainFactoryGeneratorT<RawCalorimeterHit_factory_HcalBarrelRawHits>(
	    {"HcalBarrelHits"}, "HcalBarrelRawHits"
	));
        app->Add(new JChainFactoryGeneratorT<CalorimeterHit_factory_HcalBarrelRecHits>(
	    {"HcalBarrelRawHits"}, "HcalBarrelRecHits"
	));
        app->Add(new JChainFactoryGeneratorT<ProtoCluster_factory_HcalBarrelTruthProtoClusters>(
	    {"HcalBarrelRecHits", "HcalBarrelHits"}, "HcalBarrelTruthProtoClusters"
        ));
        app->Add(new JChainFactoryGeneratorT<ProtoCluster_factory_HcalBarrelIslandProtoClusters>(
	    {"HcalBarrelRecHits"}, "HcalBarrelIslandProtoClusters"
	));

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
