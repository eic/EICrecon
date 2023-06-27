// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <extensions/jana/JChainMultifactoryGeneratorT.h>

#include <factories/calorimetry/CalorimeterClusterRecoCoG_factoryT.h>

#include "RawCalorimeterHit_factory_EcalLumiSpecRawHits.h"
#include "CalorimeterHit_factory_EcalLumiSpecRecHits.h"
#include "ProtoCluster_factory_EcalLumiSpecTruthProtoClusters.h"
#include "ProtoCluster_factory_EcalLumiSpecIslandProtoClusters.h"

namespace eicrecon {
    class Cluster_factory_EcalLumiSpecTruthClusters: public CalorimeterClusterRecoCoG_factoryT<Cluster_factory_EcalLumiSpecTruthClusters> {
    public:
        template <typename... Args>
        Cluster_factory_EcalLumiSpecTruthClusters(Args&&... args)
        : CalorimeterClusterRecoCoG_factoryT<Cluster_factory_EcalLumiSpecTruthClusters>(std::forward<Args>(args)...) { }
    };

    class Cluster_factory_EcalLumiSpecClusters: public CalorimeterClusterRecoCoG_factoryT<Cluster_factory_EcalLumiSpecClusters> {
    public:
        template <typename... Args>
        Cluster_factory_EcalLumiSpecClusters(Args&&... args)
        : CalorimeterClusterRecoCoG_factoryT<Cluster_factory_EcalLumiSpecClusters>(std::forward<Args>(args)...) { }
    };
}

extern "C" {
    void InitPlugin(JApplication *app) {

        using namespace eicrecon;

        InitJANAPlugin(app);

        app->Add(new JFactoryGeneratorT<RawCalorimeterHit_factory_EcalLumiSpecRawHits>());
        app->Add(new JFactoryGeneratorT<CalorimeterHit_factory_EcalLumiSpecRecHits>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_EcalLumiSpecTruthProtoClusters>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_EcalLumiSpecIslandProtoClusters>());

        app->Add(
          new JChainMultifactoryGeneratorT<Cluster_factory_EcalLumiSpecClusters>(
             "EcalLumiSpecClusters",
            {"EcalLumiSpecIslandProtoClusters",  // edm4eic::ProtoClusterCollection
             "LumiSpecCALHits"},                 // edm4hep::SimCalorimeterHitCollection
            {"EcalLumiSpecClusters",             // edm4eic::Cluster
             "EcalLumiSpecClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
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
          new JChainMultifactoryGeneratorT<Cluster_factory_EcalLumiSpecTruthClusters>(
             "EcalLumiSpecTruthClusters",
            {"EcalLumiSpecTruthProtoClusters",        // edm4eic::ProtoClusterCollection
             "LumiSpecCALHits"},                      // edm4hep::SimCalorimeterHitCollection
            {"EcalLumiSpecTruthClusters",             // edm4eic::Cluster
             "EcalLumiSpecTruthClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
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
    }
}
