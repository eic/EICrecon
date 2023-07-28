// Copyright 2023, Friederike Bock
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include "extensions/jana/JChainFactoryGeneratorT.h"
#include "extensions/jana/JChainMultifactoryGeneratorT.h"

#include "factories/calorimetry/CalorimeterClusterRecoCoG_factoryT.h"
#include "factories/calorimetry/CalorimeterHitDigi_factoryT.h"
#include "factories/calorimetry/CalorimeterHitReco_factoryT.h"
#include "factories/calorimetry/CalorimeterHitsMerger_factoryT.h"
#include "factories/calorimetry/CalorimeterTruthClustering_factoryT.h"

#include "ProtoCluster_factory_HcalEndcapNIslandProtoClusters.h"


namespace eicrecon {
  using RawCalorimeterHit_factory_HcalEndcapNRawHits = CalorimeterHitDigi_factoryT<>;
  using CalorimeterHit_factory_HcalEndcapNRecHits = CalorimeterHitReco_factoryT<>;
  using CalorimeterHit_factory_HcalEndcapNMergedHits = CalorimeterHitsMerger_factoryT<>;
  using ProtoCluster_factory_HcalEndcapNTruthProtoClusters = CalorimeterTruthClustering_factoryT<>;
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
        app->Add(new JChainMultifactoryGeneratorT<CalorimeterHit_factory_HcalEndcapNRecHits>(
          "HcalEndcapNRecHits", {"HcalEndcapNRawHits"}, {"HcalEndcapNRecHits"},
          {
            .capADC = 1024,
            .dyRangeADC = 3.6 * dd4hep::MeV,
            .pedMeanADC = 20,
            .pedSigmaADC = 0.3,
            .resolutionTDC = 10 * dd4hep::picosecond,
            .thresholdFactor = 4.0,
            .thresholdValue = 1.0,
            .sampFrac = 0.998,
            .readout = "HcalEndcapNHits",
          },
          app   // TODO: Remove me once fixed
        ));
        app->Add(new JChainMultifactoryGeneratorT<CalorimeterHit_factory_HcalEndcapNMergedHits>(
          "HcalEndcapNMergedHits", {"HcalEndcapNRecHits"}, {"HcalEndcapNMergedHits"},
          {
            .readout = "HcalEndcapNHits",
            .fields = {"layer", "slice"},
            .refs = {1, 0},
          },
          app   // TODO: Remove me once fixed
        ));
        app->Add(new JChainMultifactoryGeneratorT<ProtoCluster_factory_HcalEndcapNTruthProtoClusters>(
          "HcalEndcapNTruthProtoClusters", {"HcalEndcapNRecHits", "HcalEndcapNHits"}, {"HcalEndcapNTruthProtoClusters"},
          app   // TODO: Remove me once fixed
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
