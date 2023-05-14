// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

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
    class Cluster_factory_EcalBarrelSciGlassTruthClusters: public CalorimeterClusterRecoCoG_factoryT<Cluster_factory_EcalBarrelSciGlassTruthClusters> {
    public:
        template <typename... Args>
        Cluster_factory_EcalBarrelSciGlassTruthClusters(Args&&... args)
        : CalorimeterClusterRecoCoG_factoryT<Cluster_factory_EcalBarrelSciGlassTruthClusters>(std::forward<Args>(args)...) { }
    };

    class Cluster_factory_EcalBarrelSciGlassClusters: public CalorimeterClusterRecoCoG_factoryT<Cluster_factory_EcalBarrelSciGlassClusters> {
    public:
        template <typename... Args>
        Cluster_factory_EcalBarrelSciGlassClusters(Args&&... args)
        : CalorimeterClusterRecoCoG_factoryT<Cluster_factory_EcalBarrelSciGlassClusters>(std::forward<Args>(args)...) { }
    };

    class Cluster_factory_EcalBarrelScFiClusters: public CalorimeterClusterRecoCoG_factoryT<Cluster_factory_EcalBarrelScFiClusters> {
    public:
        template <typename... Args>
        Cluster_factory_EcalBarrelScFiClusters(Args&&... args)
        : CalorimeterClusterRecoCoG_factoryT<Cluster_factory_EcalBarrelScFiClusters>(std::forward<Args>(args)...) { }
    };

}

extern "C" {
    void InitPlugin(JApplication *app) {

        using namespace eicrecon;

        InitJANAPlugin(app);

        app->Add(new JFactoryGeneratorT<RawCalorimeterHit_factory_EcalBarrelSciGlassRawHits>());
        app->Add(new JFactoryGeneratorT<CalorimeterHit_factory_EcalBarrelSciGlassRecHits>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_EcalBarrelSciGlassProtoClusters>());
        app->Add(
          new JChainMultifactoryGeneratorT<Cluster_factory_EcalBarrelSciGlassClusters>(
             "EcalBarrelSciGlassClustersWithAssociations",
            {"EcalBarrelSciGlassProtoClusters",        // edm4eic::ProtoClusterCollection
             "EcalBarrelSciGlassRawHits"},             // edm4hep::SimCalorimeterHitCollection
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


        app->Add(new JFactoryGeneratorT<RawCalorimeterHit_factory_EcalBarrelScFiRawHits>());
        app->Add(new JFactoryGeneratorT<CalorimeterHit_factory_EcalBarrelScFiRecHits>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_EcalBarrelScFiProtoClusters>());
        app->Add(
          new JChainMultifactoryGeneratorT<Cluster_factory_EcalBarrelScFiClusters>(
             "EcalBarrelScFiClustersWithAssociations",
            {"EcalBarrelScFiProtoClusters",        // edm4eic::ProtoClusterCollection
             "EcalBarrelScFiRawHits"},             // edm4hep::SimCalorimeterHitCollection
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

        app->Add(new JFactoryGeneratorT<RawCalorimeterHit_factory_EcalBarrelImagingRawHits>());
        app->Add(new JFactoryGeneratorT<CalorimeterHit_factory_EcalBarrelImagingRecHits>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_EcalBarrelImagingProtoClusters>());

        app->Add(new JFactoryGeneratorT<Cluster_factory_EcalBarrelImagingClusters>());
        app->Add(new JFactoryGeneratorT<Cluster_factory_EcalBarrelImagingMergedClusters>());

        // Inserted types (so they can be written to output podio file)
        app->Add(new JFactoryGeneratorT<JFactoryT<edm4eic::Cluster>>("EcalBarrelImagingLayers"));
        app->Add(new JFactoryGeneratorT<JFactoryT<edm4eic::MCRecoClusterParticleAssociation>>("EcalBarrelImagingClusterAssociations"));

        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_EcalBarrelSciGlassTruthProtoClusters>());
        app->Add(
          new JChainMultifactoryGeneratorT<Cluster_factory_EcalBarrelSciGlassTruthClusters>(
             "EcalBarrelSciGlassTruthClustersWithAssociations",
            {"EcalBarrelSciGlassTruthProtoClusters",        // edm4eic::ProtoClusterCollection
             "EcalBarrelSciGlassTruthRawHits"},             // edm4hep::SimCalorimeterHitCollection
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
        app->Add(new JFactoryGeneratorT<Cluster_factory_EcalBarrelSciGlassMergedTruthClusters>());
    }
}
