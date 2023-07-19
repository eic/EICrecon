// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <extensions/jana/JChainFactoryGeneratorT.h>
#include <extensions/jana/JChainMultifactoryGeneratorT.h>

#include <factories/calorimetry/CalorimeterClusterRecoCoG_factoryT.h>

#include "RawCalorimeterHit_factory_EcalLumiSpecRawHits.h"
#include "CalorimeterHit_factory_EcalLumiSpecRecHits.h"
#include "ProtoCluster_factory_EcalLumiSpecTruthProtoClusters.h"
#include "ProtoCluster_factory_EcalLumiSpecIslandProtoClusters.h"

namespace eicrecon {
    using Cluster_factory_EcalLumiSpecTruthClusters = CalorimeterClusterRecoCoG_factoryT<>;
    using Cluster_factory_EcalLumiSpecClusters = CalorimeterClusterRecoCoG_factoryT<>;
}

extern "C" {
    void InitPlugin(JApplication *app) {

        using namespace eicrecon;

        InitJANAPlugin(app);

        app->Add(new JChainFactoryGeneratorT<RawCalorimeterHit_factory_EcalLumiSpecRawHits>(
          {"LumiSpecCALHits"}, "EcalLumiSpecRawHits"
        ));
        app->Add(new JChainFactoryGeneratorT<CalorimeterHit_factory_EcalLumiSpecRecHits>(
          {"EcalLumiSpecRawHits"}, "EcalLumiSpecRecHits"
        ));
        app->Add(new JChainFactoryGeneratorT<ProtoCluster_factory_EcalLumiSpecTruthProtoClusters>(
          {"EcalLumiSpecRecHits"}, "EcalLumiSpecTruthProtoClusters"
        ));
        app->Add(new JChainFactoryGeneratorT<ProtoCluster_factory_EcalLumiSpecIslandProtoClusters>(
          {"EcalLumiSpecRecHits"}, "EcalLumiSpecIslandProtoClusters"
        ));

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
