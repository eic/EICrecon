// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include "extensions/jana/JChainFactoryGeneratorT.h"
#include "extensions/jana/JChainMultifactoryGeneratorT.h"

#include "factories/calorimetry/CalorimeterClusterRecoCoG_factoryT.h"

#include "RawCalorimeterHit_factory_EcalEndcapNRawHits.h"
#include "CalorimeterHit_factory_EcalEndcapNRecHits.h"
#include "ProtoCluster_factory_EcalEndcapNTruthProtoClusters.h"
#include "ProtoCluster_factory_EcalEndcapNIslandProtoClusters.h"

namespace eicrecon {
    using Cluster_factory_EcalEndcapNTruthClusters = CalorimeterClusterRecoCoG_factoryT<>;
    using Cluster_factory_EcalEndcapNClusters = CalorimeterClusterRecoCoG_factoryT<>;
}

extern "C" {
    void InitPlugin(JApplication *app) {

        using namespace eicrecon;

        InitJANAPlugin(app);

        app->Add(new JChainFactoryGeneratorT<RawCalorimeterHit_factory_EcalEndcapNRawHits>(
	    {"EcalEndcapNHits"}, "EcalEndcapNRawHits"
	));
        app->Add(new JChainFactoryGeneratorT<CalorimeterHit_factory_EcalEndcapNRecHits>(
	    {"EcalEndcapNRawHits"}, "EcalEndcapNRecHits"
	));
        app->Add(new JChainFactoryGeneratorT<ProtoCluster_factory_EcalEndcapNTruthProtoClusters>(
	    {"EcalEndcapNRecHits", "EcalEndcapNHits"}, "EcalEndcapNTruthProtoClusters"
        ));
        app->Add(new JChainFactoryGeneratorT<ProtoCluster_factory_EcalEndcapNIslandProtoClusters>(
	    {"EcalEndcapNRecHits"}, "EcalEndcapNIslandProtoClusters"
	));

        app->Add(
          new JChainMultifactoryGeneratorT<Cluster_factory_EcalEndcapNTruthClusters>(
             "EcalEndcapNTruthClusters",
            {"EcalEndcapNTruthProtoClusters",        // edm4eic::ProtoClusterCollection
             "EcalEndcapNHits"},                     // edm4hep::SimCalorimeterHitCollection
            {"EcalEndcapNTruthClusters",             // edm4eic::Cluster
             "EcalEndcapNTruthClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
            {
              .energyWeight = "log",
              .moduleDimZName = "",
              .sampFrac = 1.0,
              .logWeightBase = 4.6,
              .depthCorrection = 0.0,
              .enableEtaBounds = false
            },
            app   // TODO: Remove me once fixed
          )
        );

        app->Add(
          new JChainMultifactoryGeneratorT<Cluster_factory_EcalEndcapNClusters>(
             "EcalEndcapNClusters",
            {"EcalEndcapNIslandProtoClusters",  // edm4eic::ProtoClusterCollection
             "EcalEndcapNHits"},                // edm4hep::SimCalorimeterHitCollection
            {"EcalEndcapNClusters",             // edm4eic::Cluster
             "EcalEndcapNClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
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
