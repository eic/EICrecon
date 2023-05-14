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
    class Cluster_factory_HcalEndcapNTruthClusters: public CalorimeterClusterRecoCoG_factoryT<Cluster_factory_HcalEndcapNTruthClusters> {
    public:
        template <typename... Args>
        Cluster_factory_HcalEndcapNTruthClusters(Args&&... args)
        : CalorimeterClusterRecoCoG_factoryT<Cluster_factory_HcalEndcapNTruthClusters>(std::forward<Args>(args)...) { }
    };

    class Cluster_factory_HcalEndcapNClusters: public CalorimeterClusterRecoCoG_factoryT<Cluster_factory_HcalEndcapNClusters> {
    public:
        template <typename... Args>
        Cluster_factory_HcalEndcapNClusters(Args&&... args)
        : CalorimeterClusterRecoCoG_factoryT<Cluster_factory_HcalEndcapNClusters>(std::forward<Args>(args)...) { }
    };
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
             "HcalEndcapNTruthClustersWithAssociations",
            {"HcalEndcapNTruthProtoClusters",        // edm4eic::ProtoClusterCollection
             "HcalEndcapNRawHits"},                  // edm4hep::SimCalorimeterHitCollection
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
             "HcalEndcapNClustersWithAssociations",
            {"HcalEndcapNIslandProtoClusters",  // edm4eic::ProtoClusterCollection
             "HcalEndcapNRawHits"},             // edm4hep::SimCalorimeterHitCollection
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
