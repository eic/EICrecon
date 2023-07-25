// Copyright 2023, Friederike Bock
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include "extensions/jana/JChainFactoryGeneratorT.h"
#include "extensions/jana/JChainMultifactoryGeneratorT.h"

#include "factories/calorimetry/CalorimeterClusterRecoCoG_factoryT.h"
#include "factories/calorimetry/CalorimeterHitDigi_factoryT.h"

#include "CalorimeterHit_factory_HcalEndcapNRecHits.h"
#include "CalorimeterHit_factory_HcalEndcapNMergedHits.h"
#include "ProtoCluster_factory_HcalEndcapNTruthProtoClusters.h"
#include "ProtoCluster_factory_HcalEndcapNIslandProtoClusters.h"


namespace eicrecon {
  using RawCalorimeterHit_factory_HcalEndcapNRawHits = CalorimeterHitDigi_factoryT<>;
  using Cluster_factory_HcalEndcapNTruthClusters = CalorimeterClusterRecoCoG_factoryT<>;
  using Cluster_factory_HcalEndcapNClusters = CalorimeterClusterRecoCoG_factoryT<>;
}

extern "C" {
    void InitPlugin(JApplication *app) {

        using namespace eicrecon;

        InitJANAPlugin(app);

        app->Add(new JChainMultifactoryGeneratorT<RawCalorimeterHit_factory_HcalEndcapNRawHits>(
          "HcalEndcapNRawHits", {"HcalEndcapNHits"}, {"HcalEndcapNRawHits"},
          {
            .tRes = 0.0 * dd4hep::ns,
            .capADC = 1024,
            .dyRangeADC = 3.6 * dd4hep::MeV,
            .pedMeanADC = 20,
            .pedSigmaADC = 0.3,
            .resolutionTDC = 10 * dd4hep::picosecond,
            .corrMeanScale = 1.0,
          },
          app   // TODO: Remove me once fixed
        ));
        app->Add(new JChainFactoryGeneratorT<CalorimeterHit_factory_HcalEndcapNRecHits>(
          {"HcalEndcapNRawHits"}, "HcalEndcapNRecHits"
        ));
        app->Add(new JChainFactoryGeneratorT<CalorimeterHit_factory_HcalEndcapNMergedHits>(
          {"HcalEndcapNRecHits"}, "HcalEndcapNMergedHits"
        ));
        app->Add(new JChainFactoryGeneratorT<ProtoCluster_factory_HcalEndcapNTruthProtoClusters>(
          {"HcalEndcapNRecHits", "HcalEndcapNHits"}, "HcalEndcapNTruthProtoClusters"
        ));
        app->Add(new JChainFactoryGeneratorT<ProtoCluster_factory_HcalEndcapNIslandProtoClusters>(
          {"HcalEndcapNRecHits"}, "HcalEndcapNIslandProtoClusters"
        ));
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
