// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <extensions/jana/JChainFactoryGeneratorT.h>
#include <extensions/jana/JChainMultifactoryGeneratorT.h>

#include <factories/calorimetry/CalorimeterClusterRecoCoG_factoryT.h>

#include "RawCalorimeterHit_factory_EcalBarrelSciGlassRawHits.h"
#include "CalorimeterHit_factory_EcalBarrelSciGlassRecHits.h"
#include "ProtoCluster_factory_EcalBarrelSciGlassTruthProtoClusters.h"
#include "ProtoCluster_factory_EcalBarrelSciGlassProtoClusters.h"
#include "Cluster_factory_EcalBarrelSciGlassMergedTruthClusters.h"

#include "RawCalorimeterHit_factory_EcalBarrelScFiRawHits.h"
#include "CalorimeterHit_factory_EcalBarrelScFiRecHits.h"
#include "CalorimeterHit_factory_EcalBarrelScFiMergedHits.h"
#include "ProtoCluster_factory_EcalBarrelScFiProtoClusters.h"

#include "RawCalorimeterHit_factory_EcalBarrelImagingRawHits.h"
#include "CalorimeterHit_factory_EcalBarrelImagingRecHits.h"
#include "ProtoCluster_factory_EcalBarrelImagingProtoClusters.h"
#include "Cluster_factory_EcalBarrelImagingClusters.h"
#include "Cluster_factory_EcalBarrelImagingMergedClusters.h"


namespace eicrecon {
    using Cluster_factory_EcalBarrelSciGlassTruthClusters = CalorimeterClusterRecoCoG_factoryT<>;
    using Cluster_factory_EcalBarrelSciGlassClusters = CalorimeterClusterRecoCoG_factoryT<>;
    using Cluster_factory_EcalBarrelScFiClusters = CalorimeterClusterRecoCoG_factoryT<>;
}

extern "C" {
    void InitPlugin(JApplication *app) {

        using namespace eicrecon;

        InitJANAPlugin(app);

        app->Add(new JChainFactoryGeneratorT<RawCalorimeterHit_factory_EcalBarrelSciGlassRawHits>(
          {"EcalBarrelSciGlassHits"}, "EcalBarrelSciGlassRawHits"
        ));
        app->Add(new JChainFactoryGeneratorT<CalorimeterHit_factory_EcalBarrelSciGlassRecHits>(
          {"EcalBarrelSciGlassRawHits"}, "EcalBarrelSciGlassRecHits"
        ));
        app->Add(new JChainFactoryGeneratorT<ProtoCluster_factory_EcalBarrelSciGlassProtoClusters>(
          {"EcalBarrelSciGlassRecHits"}, "EcalBarrelSciGlassProtoClusters"
        ));
        app->Add(
          new JChainMultifactoryGeneratorT<Cluster_factory_EcalBarrelSciGlassClusters>(
             "EcalBarrelSciGlassClusters",
            {"EcalBarrelSciGlassProtoClusters",        // edm4eic::ProtoClusterCollection
             "EcalBarrelSciGlassHits"},                // edm4hep::SimCalorimeterHitCollection
            {"EcalBarrelSciGlassClusters",             // edm4eic::Cluster
             "EcalBarrelSciGlassClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
            {
              .energyWeight = "log",
              .moduleDimZName = "",
              .sampFrac = 0.92,
              .logWeightBase = 6.2,
              .depthCorrection = 0.0,
              .enableEtaBounds = true
            },
            app   // TODO: Remove me once fixed
          )
        );


        app->Add(new JChainFactoryGeneratorT<RawCalorimeterHit_factory_EcalBarrelScFiRawHits>(
          {"EcalBarrelScFiHits"}, "EcalBarrelScFiRawHits"
        ));
        app->Add(new JChainFactoryGeneratorT<CalorimeterHit_factory_EcalBarrelScFiRecHits>(
          {"EcalBarrelScFiRawHits"}, "EcalBarrelScFiRecHits"
        ));
        app->Add(new JChainFactoryGeneratorT<ProtoCluster_factory_EcalBarrelScFiProtoClusters>(
          {"EcalBarrelScFiRecHits"}, "EcalBarrelScFiProtoClusters"
        ));
        app->Add(
          new JChainMultifactoryGeneratorT<Cluster_factory_EcalBarrelScFiClusters>(
             "EcalBarrelScFiClusters",
            {"EcalBarrelScFiProtoClusters",        // edm4eic::ProtoClusterCollection
             "EcalBarrelScFiHits"},                // edm4hep::SimCalorimeterHitCollection
            {"EcalBarrelScFiClusters",             // edm4eic::Cluster
             "EcalBarrelScFiClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
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

        app->Add(new JChainFactoryGeneratorT<RawCalorimeterHit_factory_EcalBarrelImagingRawHits>(
          {"EcalBarrelImagingHits"}, "EcalBarrelImagingRawHits"
        ));
        app->Add(new JChainFactoryGeneratorT<CalorimeterHit_factory_EcalBarrelImagingRecHits>(
          {"EcalBarrelImagingRawHits"}, "EcalBarrelImagingRecHits"
        ));
        app->Add(new JChainFactoryGeneratorT<ProtoCluster_factory_EcalBarrelImagingProtoClusters>(
          {"EcalBarrelImagingRecHits"}, "EcalBarrelImagingProtoClusters"
        ));

        app->Add(new JChainFactoryGeneratorT<Cluster_factory_EcalBarrelImagingClusters>(
          {"EcalBarrelImagingProtoClusters"}, "EcalBarrelImagingClusters"
        ));
        app->Add(new JChainFactoryGeneratorT<Cluster_factory_EcalBarrelImagingMergedClusters>(
          {"EcalBarrelImagingClusters",
            "EcalBarrelScFiClusters",
            "EcalBarrelImagingClusterAssociations",
            "EcalBarrelScFiClusterAssociations"},
          "EcalBarrelImagingMergedClusters"
        ));

        // Inserted types (so they can be written to output podio file)
        app->Add(new JFactoryGeneratorT<JFactoryT<edm4eic::Cluster>>("EcalBarrelImagingLayers"));
        app->Add(new JFactoryGeneratorT<JFactoryT<edm4eic::MCRecoClusterParticleAssociation>>("EcalBarrelImagingClusterAssociations"));

        app->Add(new JChainFactoryGeneratorT<ProtoCluster_factory_EcalBarrelSciGlassTruthProtoClusters>(
	    {},
	    "EcalBarrelSciGlassTruthProtoClusters"
	));
        app->Add(
          new JChainMultifactoryGeneratorT<Cluster_factory_EcalBarrelSciGlassTruthClusters>(
             "EcalBarrelSciGlassTruthClusters",
            {"EcalBarrelSciGlassTruthProtoClusters",        // edm4eic::ProtoClusterCollection
             "EcalBarrelSciGlassHits"},                     // edm4hep::SimCalorimeterHitCollection
            {"EcalBarrelSciGlassTruthClusters",             // edm4eic::Cluster
             "EcalBarrelSciGlassTruthClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
             {
               .energyWeight = "log",
               .moduleDimZName = "",
               .sampFrac = 1.0,
               .logWeightBase = 6.2,
               .depthCorrection = 0.0,
               .enableEtaBounds = true
             },
            app   // TODO: Remove me once fixed
          )
        );
        app->Add(new JChainFactoryGeneratorT<Cluster_factory_EcalBarrelSciGlassMergedTruthClusters>(
          {},
	  "EcalBarrelSciGlassMergedTruthCluster"
	));
    }
}
