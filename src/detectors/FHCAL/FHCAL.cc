// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <extensions/jana/JChainMultifactoryGeneratorT.h>

#include <factories/calorimetry/CalorimeterClusterRecoCoG_factoryT.h>

#include "RawCalorimeterHit_factory_HcalEndcapPRawHits.h"
#include "CalorimeterHit_factory_HcalEndcapPRecHits.h"
#include "CalorimeterHit_factory_HcalEndcapPMergedHits.h"
#include "ProtoCluster_factory_HcalEndcapPTruthProtoClusters.h"
#include "ProtoCluster_factory_HcalEndcapPIslandProtoClusters.h"

#include "RawCalorimeterHit_factory_HcalEndcapPInsertRawHits.h"
#include "CalorimeterHit_factory_HcalEndcapPInsertRecHits.h"
#include "CalorimeterHit_factory_HcalEndcapPInsertMergedHits.h"
#include "ProtoCluster_factory_HcalEndcapPInsertTruthProtoClusters.h"
#include "ProtoCluster_factory_HcalEndcapPInsertIslandProtoClusters.h"

#include "RawCalorimeterHit_factory_LFHCALRawHits.h"
#include "CalorimeterHit_factory_LFHCALRecHits.h"
#include "ProtoCluster_factory_LFHCALTruthProtoClusters.h"
#include "ProtoCluster_factory_LFHCALIslandProtoClusters.h"

namespace eicrecon {
    class Cluster_factory_HcalEndcapPTruthClusters: public CalorimeterClusterRecoCoG_factoryT<Cluster_factory_HcalEndcapPTruthClusters> {
    public:
        template <typename... Args>
        Cluster_factory_HcalEndcapPTruthClusters(Args&&... args)
        : CalorimeterClusterRecoCoG_factoryT<Cluster_factory_HcalEndcapPTruthClusters>(std::forward<Args>(args)...) { }
    };

    class Cluster_factory_HcalEndcapPClusters: public CalorimeterClusterRecoCoG_factoryT<Cluster_factory_HcalEndcapPClusters> {
    public:
        template <typename... Args>
        Cluster_factory_HcalEndcapPClusters(Args&&... args)
        : CalorimeterClusterRecoCoG_factoryT<Cluster_factory_HcalEndcapPClusters>(std::forward<Args>(args)...) { }
    };

    class Cluster_factory_HcalEndcapPInsertTruthClusters: public CalorimeterClusterRecoCoG_factoryT<Cluster_factory_HcalEndcapPInsertTruthClusters> {
    public:
        template <typename... Args>
        Cluster_factory_HcalEndcapPInsertTruthClusters(Args&&... args)
        : CalorimeterClusterRecoCoG_factoryT<Cluster_factory_HcalEndcapPInsertTruthClusters>(std::forward<Args>(args)...) { }
    };

    class Cluster_factory_HcalEndcapPInsertClusters: public CalorimeterClusterRecoCoG_factoryT<Cluster_factory_HcalEndcapPInsertClusters> {
    public:
        template <typename... Args>
        Cluster_factory_HcalEndcapPInsertClusters(Args&&... args)
        : CalorimeterClusterRecoCoG_factoryT<Cluster_factory_HcalEndcapPInsertClusters>(std::forward<Args>(args)...) { }
    };

    class Cluster_factory_LFHCALTruthClusters: public CalorimeterClusterRecoCoG_factoryT<Cluster_factory_LFHCALTruthClusters> {
    public:
        template <typename... Args>
        Cluster_factory_LFHCALTruthClusters(Args&&... args)
        : CalorimeterClusterRecoCoG_factoryT<Cluster_factory_LFHCALTruthClusters>(std::forward<Args>(args)...) { }
    };

    class Cluster_factory_LFHCALClusters: public CalorimeterClusterRecoCoG_factoryT<Cluster_factory_LFHCALClusters> {
    public:
        template <typename... Args>
        Cluster_factory_LFHCALClusters(Args&&... args)
        : CalorimeterClusterRecoCoG_factoryT<Cluster_factory_LFHCALClusters>(std::forward<Args>(args)...) { }
    };

}

extern "C" {
    void InitPlugin(JApplication *app) {

        using namespace eicrecon;

        InitJANAPlugin(app);

        app->Add(new JFactoryGeneratorT<RawCalorimeterHit_factory_HcalEndcapPRawHits>());
        app->Add(new JFactoryGeneratorT<CalorimeterHit_factory_HcalEndcapPRecHits>());
        app->Add(new JFactoryGeneratorT<CalorimeterHit_factory_HcalEndcapPMergedHits>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_HcalEndcapPTruthProtoClusters>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_HcalEndcapPIslandProtoClusters>());

        app->Add(
          new JChainMultifactoryGeneratorT<Cluster_factory_HcalEndcapPTruthClusters>(
             "HcalEndcapPTruthClusters",
            {"HcalEndcapPTruthProtoClusters",        // edm4eic::ProtoClusterCollection
             "HcalEndcapPHits"},                     // edm4hep::SimCalorimeterHitCollection
            {"HcalEndcapPTruthClusters",             // edm4eic::Cluster
             "HcalEndcapPTruthClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
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
          new JChainMultifactoryGeneratorT<Cluster_factory_HcalEndcapPClusters>(
             "HcalEndcapPClusters",
            {"HcalEndcapPIslandProtoClusters",  // edm4eic::ProtoClusterCollection
             "HcalEndcapPHits"},                // edm4hep::SimCalorimeterHitCollection
            {"HcalEndcapPClusters",             // edm4eic::Cluster
             "HcalEndcapPClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
            {
              .energyWeight = "log",
              .moduleDimZName = "",
              .sampFrac = 0.033,
              .logWeightBase = 6.2,
              .depthCorrection = 0.0,
              .enableEtaBounds = false,
            },
            app   // TODO: Remove me once fixed
          )
        );

        app->Add(new JFactoryGeneratorT<RawCalorimeterHit_factory_HcalEndcapPInsertRawHits>());
        app->Add(new JFactoryGeneratorT<CalorimeterHit_factory_HcalEndcapPInsertRecHits>());
        app->Add(new JFactoryGeneratorT<CalorimeterHit_factory_HcalEndcapPInsertMergedHits>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_HcalEndcapPInsertTruthProtoClusters>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_HcalEndcapPInsertIslandProtoClusters>());

        app->Add(
          new JChainMultifactoryGeneratorT<Cluster_factory_HcalEndcapPTruthClusters>(
             "HcalEndcapPInsertTruthClusters",
            {"HcalEndcapPInsertTruthProtoClusters",        // edm4eic::ProtoClusterCollection
             "HcalEndcapPInsertHits"},                     // edm4hep::SimCalorimeterHitCollection
            {"HcalEndcapPInsertTruthClusters",             // edm4eic::Cluster
             "HcalEndcapPInsertTruthClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
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
          new JChainMultifactoryGeneratorT<Cluster_factory_HcalEndcapPInsertClusters>(
             "HcalEndcapPInsertClusters",
            {"HcalEndcapPInsertIslandProtoClusters",  // edm4eic::ProtoClusterCollection
             "HcalEndcapPInsertHits"},                // edm4hep::SimCalorimeterHitCollection
            {"HcalEndcapPInsertClusters",             // edm4eic::Cluster
             "HcalEndcapPInsertClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
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

        app->Add(new JFactoryGeneratorT<RawCalorimeterHit_factory_LFHCALRawHits>());
        app->Add(new JFactoryGeneratorT<CalorimeterHit_factory_LFHCALRecHits>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_LFHCALTruthProtoClusters>());
        app->Add(new JFactoryGeneratorT<ProtoCluster_factory_LFHCALIslandProtoClusters>());

        app->Add(
          new JChainMultifactoryGeneratorT<Cluster_factory_LFHCALTruthClusters>(
             "LFHCALTruthClusters",
            {"LFHCALTruthProtoClusters",        // edm4eic::ProtoClusterCollection
             "LFHCALHits"},                     // edm4hep::SimCalorimeterHitCollection
            {"LFHCALTruthClusters",             // edm4eic::Cluster
             "LFHCALTruthClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
            {
              .energyWeight = "log",
              .moduleDimZName = "",
              .sampFrac = 1.0,
              .logWeightBase = 4.5,
              .depthCorrection = 0.0,
              .enableEtaBounds = false
            },
            app   // TODO: Remove me once fixed
          )
        );

        app->Add(
          new JChainMultifactoryGeneratorT<Cluster_factory_LFHCALClusters>(
             "LFHCALClusters",
            {"LFHCALIslandProtoClusters",  // edm4eic::ProtoClusterCollection
             "LFHCALHits"},                // edm4hep::SimCalorimeterHitCollection
            {"LFHCALClusters",             // edm4eic::Cluster
             "LFHCALClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
            {
              .energyWeight = "log",
              .moduleDimZName = "",
              .sampFrac = 1.0,
              .logWeightBase = 4.5,
              .depthCorrection = 0.0,
              .enableEtaBounds = false,
            },
            app   // TODO: Remove me once fixed
          )
        );
    }
}
