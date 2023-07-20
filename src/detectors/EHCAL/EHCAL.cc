// Copyright 2023, Friederike Bock
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <extensions/jana/JChainMultifactoryGeneratorT.h>

#include <factories/calorimetry/CalorimeterClusterRecoCoG_factoryT.h>

#include "RawCalorimeterHit_factory_HcalEndcapNRawHits.h"
#include "CalorimeterHit_factory_HcalEndcapNRecHits.h"
#include "CalorimeterHit_factory_HcalEndcapNMergedHits.h"
#include "ProtoCluster_factory_HcalEndcapNTruthProtoClusters.h"
#include "ProtoCluster_factory_HcalEndcapNIslandProtoClusters.h"


namespace eicrecon {
    using Cluster_factory_HcalEndcapNTruthClusters = CalorimeterClusterRecoCoG_factoryT<>;
    using Cluster_factory_HcalEndcapNClusters = CalorimeterClusterRecoCoG_factoryT<>;
}

extern "C" {
    void InitPlugin(JApplication *app) {

        using namespace eicrecon;

        InitJANAPlugin(app);

        app->Add(new JFactoryGeneratorT<RawCalorimeterHit_factory_HcalEndcapNRawHits>());
        app->Add(new JFactoryGeneratorT<CalorimeterHit_factory_HcalEndcapNRecHits>());
        app->Add(new JFactoryGeneratorT<CalorimeterHit_factory_HcalEndcapNMergedHits>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_HcalEndcapNTruthProtoClusters>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_HcalEndcapNIslandProtoClusters>());
        app->Add(
          new JChainMultifactoryGeneratorT<Cluster_factory_HcalEndcapNTruthClusters>(
             "HcalEndcapNTruthClusters",
            {"HcalEndcapNTruthProtoClusters",        // edm4eic::ProtoClusterCollection
             "HcalEndcapNHits"},                     // edm4hep::SimCalorimeterHitCollection
            {"HcalEndcapNTruthClusters",             // edm4eic::Cluster
             "HcalEndcapNTruthClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
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
          new JChainMultifactoryGeneratorT<Cluster_factory_HcalEndcapNClusters>(
             "HcalEndcapNClusters",
            {"HcalEndcapNIslandProtoClusters",  // edm4eic::ProtoClusterCollection
             "HcalEndcapNHits"},                // edm4hep::SimCalorimeterHitCollection
            {"HcalEndcapNClusters",             // edm4eic::Cluster
             "HcalEndcapNClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
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
