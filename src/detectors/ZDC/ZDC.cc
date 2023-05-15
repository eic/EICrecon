// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <extensions/jana/JChainMultifactoryGeneratorT.h>

#include <factories/calorimetry/CalorimeterClusterRecoCoG_factoryT.h>

#include "RawCalorimeterHit_factory_ZDCEcalRawHits.h"
#include "CalorimeterHit_factory_ZDCEcalRecHits.h"
#include "ProtoCluster_factory_ZDCEcalTruthProtoClusters.h"
#include "ProtoCluster_factory_ZDCEcalIslandProtoClusters.h"


namespace eicrecon {
    class Cluster_factory_ZDCEcalTruthClusters: public CalorimeterClusterRecoCoG_factoryT<Cluster_factory_ZDCEcalTruthClusters> {
    public:
        template <typename... Args>
        Cluster_factory_ZDCEcalTruthClusters(Args&&... args)
        : CalorimeterClusterRecoCoG_factoryT<Cluster_factory_ZDCEcalTruthClusters>(std::forward<Args>(args)...) { }
    };

    class Cluster_factory_ZDCEcalClusters: public CalorimeterClusterRecoCoG_factoryT<Cluster_factory_ZDCEcalClusters> {
    public:
        template <typename... Args>
        Cluster_factory_ZDCEcalClusters(Args&&... args)
        : CalorimeterClusterRecoCoG_factoryT<Cluster_factory_ZDCEcalClusters>(std::forward<Args>(args)...) { }
    };
}

extern "C" {
    void InitPlugin(JApplication *app) {

        using namespace eicrecon;

        InitJANAPlugin(app);

        app->Add(new JFactoryGeneratorT<RawCalorimeterHit_factory_ZDCEcalRawHits>());
        app->Add(new JFactoryGeneratorT<CalorimeterHit_factory_ZDCEcalRecHits>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_ZDCEcalTruthProtoClusters>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_ZDCEcalIslandProtoClusters>());

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
              .enableEtaBounds = true
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
