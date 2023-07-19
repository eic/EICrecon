// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <extensions/jana/JChainFactoryGeneratorT.h>
#include <extensions/jana/JChainMultifactoryGeneratorT.h>

#include <factories/calorimetry/CalorimeterClusterRecoCoG_factoryT.h>

#include "RawCalorimeterHit_factory_ZDCEcalRawHits.h"
#include "CalorimeterHit_factory_ZDCEcalRecHits.h"
#include "ProtoCluster_factory_ZDCEcalTruthProtoClusters.h"
#include "ProtoCluster_factory_ZDCEcalIslandProtoClusters.h"


namespace eicrecon {
    using Cluster_factory_ZDCEcalTruthClusters = CalorimeterClusterRecoCoG_factoryT<>;
    using Cluster_factory_ZDCEcalClusters = CalorimeterClusterRecoCoG_factoryT<>;
}

extern "C" {
    void InitPlugin(JApplication *app) {

        using namespace eicrecon;

        InitJANAPlugin(app);

        app->Add(new JChainFactoryGeneratorT<RawCalorimeterHit_factory_ZDCEcalRawHits>(
	  {"ZDCEcalHits"}, "ZDCEcalRawHits"
	));
        app->Add(new JChainFactoryGeneratorT<CalorimeterHit_factory_ZDCEcalRecHits>(
	  {"ZDCEcalRawHits"}, "ZDCEcalRecHits"
        ));
        app->Add(new JChainFactoryGeneratorT<ProtoCluster_factory_ZDCEcalTruthProtoClusters>(
	  {"ZDCEcalRecHits", "ZDCEcalHits"}, "ZDCEcalTruthProtoClusters"
        ));
        app->Add(new JChainFactoryGeneratorT<ProtoCluster_factory_ZDCEcalIslandProtoClusters>(
	  {"ZDCEcalRecHits"}, "ZDCEcalIslandProtoClusters"
        ));

        app->Add(
          new JChainMultifactoryGeneratorT<Cluster_factory_ZDCEcalTruthClusters>(
             "ZDCEcalTruthClusters",
            {"ZDCEcalTruthProtoClusters",        // edm4eic::ProtoClusterCollection
             "ZDCEcalHits"},                     // edm4hep::SimCalorimeterHitCollection
            {"ZDCEcalTruthClusters",             // edm4eic::Cluster
             "ZDCEcalTruthClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
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
          new JChainMultifactoryGeneratorT<Cluster_factory_ZDCEcalClusters>(
             "ZDCEcalClusters",
            {"ZDCEcalIslandProtoClusters",  // edm4eic::ProtoClusterCollection
             "ZDCEcalHits"},                // edm4hep::SimCalorimeterHitCollection
            {"ZDCEcalClusters",             // edm4eic::Cluster
             "ZDCEcalClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
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
    }
}
