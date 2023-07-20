// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <extensions/jana/JChainMultifactoryGeneratorT.h>

#include <factories/calorimetry/CalorimeterClusterRecoCoG_factoryT.h>

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

        app->Add(new JFactoryGeneratorT<RawCalorimeterHit_factory_EcalEndcapNRawHits>());
        app->Add(new JFactoryGeneratorT<CalorimeterHit_factory_EcalEndcapNRecHits>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_EcalEndcapNTruthProtoClusters>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_EcalEndcapNIslandProtoClusters>());

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
