// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include "extensions/jana/JChainFactoryGeneratorT.h"
#include "extensions/jana/JChainMultifactoryGeneratorT.h"

#include "factories/calorimetry/CalorimeterClusterRecoCoG_factoryT.h"
#include "factories/calorimetry/CalorimeterHitDigi_factoryT.h"
#include "factories/calorimetry/CalorimeterHitReco_factoryT.h"
#include "factories/calorimetry/CalorimeterTruthClustering_factoryT.h"

#include "ProtoCluster_factory_EcalEndcapPIslandProtoClusters.h"

#include "ProtoCluster_factory_EcalEndcapPInsertIslandProtoClusters.h"

namespace eicrecon {
  using RawCalorimeterHit_factory_EcalEndcapPRawHits = CalorimeterHitDigi_factoryT<>;
  using RawCalorimeterHit_factory_EcalEndcapPInsertRawHits = CalorimeterHitDigi_factoryT<>;
  using CalorimeterHit_factory_EcalEndcapPRecHits = CalorimeterHitReco_factoryT<>;
  using CalorimeterHit_factory_EcalEndcapPInsertRecHits = CalorimeterHitReco_factoryT<>;
  using ProtoCluster_factory_EcalEndcapPTruthProtoClusters = CalorimeterTruthClustering_factoryT<>;
  using ProtoCluster_factory_EcalEndcapPInsertTruthProtoClusters = CalorimeterTruthClustering_factoryT<>;
  using Cluster_factory_EcalEndcapPTruthClusters = CalorimeterClusterRecoCoG_factoryT<>;
  using Cluster_factory_EcalEndcapPClusters = CalorimeterClusterRecoCoG_factoryT<>;
  using Cluster_factory_EcalEndcapPInsertTruthClusters = CalorimeterClusterRecoCoG_factoryT<>;
  using Cluster_factory_EcalEndcapPInsertClusters = CalorimeterClusterRecoCoG_factoryT<>;
}

extern "C" {
    void InitPlugin(JApplication *app) {

        using namespace eicrecon;

        InitJANAPlugin(app);

        app->Add(new JChainMultifactoryGeneratorT<RawCalorimeterHit_factory_EcalEndcapPRawHits>(
          "EcalEndcapPRawHits", {"EcalEndcapPHits"}, {"EcalEndcapPRawHits"},
          {
            .eRes = {0.00340 * sqrt(dd4hep::GeV), 0.0009, 0.0 * dd4hep::GeV}, // (0.340% / sqrt(E)) \oplus 0.09%
            .tRes = 0.0,
            .capADC = 65536, //2^16  (approximate HGCROC resolution) old 16384
            .capTime = 100, // given in ns, 4 samples in HGCROC
            .dyRangeADC = 3 * dd4hep::GeV,
            .pedMeanADC = 100,
            .pedSigmaADC = 0.7,
            .resolutionTDC = 10 * dd4hep::picosecond,
            .corrMeanScale = 0.03,
          },
          app   // TODO: Remove me once fixed
        ));
        app->Add(new JChainMultifactoryGeneratorT<CalorimeterHit_factory_EcalEndcapPRecHits>(
          "EcalEndcapPRecHits", {"EcalEndcapPRawHits"}, {"EcalEndcapPRecHits"},
          {
            .capADC = 65536,
            .dyRangeADC = 3. * dd4hep::GeV,
            .pedMeanADC = 100,
            .pedSigmaADC = 0.7,
            .resolutionTDC = 10 * dd4hep::picosecond,
            .thresholdFactor = 5.0,
            .thresholdValue = 2.0,
            .sampFrac  =0.03,
            .readout = "EcalEndcapPHits",
          },
          app   // TODO: Remove me once fixed
        ));
        app->Add(new JChainMultifactoryGeneratorT<ProtoCluster_factory_EcalEndcapPTruthProtoClusters>(
          "EcalEndcapPTruthProtoClusters", {"EcalEndcapPRecHits", "EcalEndcapPHits"}, {"EcalEndcapPTruthProtoClusters"},
          app   // TODO: Remove me once fixed
        ));
        app->Add(new JChainFactoryGeneratorT<ProtoCluster_factory_EcalEndcapPIslandProtoClusters>(
          {"EcalEndcapPRecHits"}, "EcalEndcapPIslandProtoClusters"
        ));

        app->Add(
          new JChainMultifactoryGeneratorT<Cluster_factory_EcalEndcapPTruthClusters>(
             "EcalEndcapPTruthClusters",
            {"EcalEndcapPTruthProtoClusters",        // edm4eic::ProtoClusterCollection
             "EcalEndcapPHits"},                     // edm4hep::SimCalorimeterHitCollection
            {"EcalEndcapPTruthClusters",             // edm4eic::Cluster
             "EcalEndcapPTruthClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
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

        app->Add(
          new JChainMultifactoryGeneratorT<Cluster_factory_EcalEndcapPClusters>(
             "EcalEndcapPClusters",
            {"EcalEndcapPIslandProtoClusters",  // edm4eic::ProtoClusterCollection
             "EcalEndcapPHits"},                // edm4hep::SimCalorimeterHitCollection
            {"EcalEndcapPClusters",             // edm4eic::Cluster
             "EcalEndcapPClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
            {
              .energyWeight = "log",
              .moduleDimZName = "",
              .sampFrac = 1.0,
              .logWeightBase = 3.6,
              .depthCorrection = 0.0,
              .enableEtaBounds = false,
            },
            app   // TODO: Remove me once fixed
          )
        );

        app->Add(new JChainMultifactoryGeneratorT<RawCalorimeterHit_factory_EcalEndcapPInsertRawHits>(
          "EcalEndcapPInsertRawHits", {"EcalEndcapPInsertHits"}, {"EcalEndcapPInsertRawHits"},
          {
            .eRes = {0.00340 * sqrt(dd4hep::GeV), 0.0009, 0.0 * dd4hep::GeV}, // (0.340% / sqrt(E)) \oplus 0.09%
            .tRes = 0.0 * dd4hep::ns,
            .capADC = 16384,
            .dyRangeADC = 3 * dd4hep::GeV,
            .pedMeanADC = 100,
            .pedSigmaADC = 0.7,
            .resolutionTDC = 10 * dd4hep::picosecond,
            .corrMeanScale = 0.03,
          },
          app   // TODO: Remove me once fixed
        ));
        app->Add(new JChainMultifactoryGeneratorT<CalorimeterHit_factory_EcalEndcapPInsertRecHits>(
          "EcalEndcapPInsertRecHits", {"EcalEndcapPInsertRawHits"}, {"EcalEndcapPInsertRecHits"},
          {
            .capADC = 16384,
            .dyRangeADC = 3. * dd4hep::GeV,
            .pedMeanADC = 100,
            .pedSigmaADC = 0.7,
            .resolutionTDC = 10 * dd4hep::picosecond,
            .thresholdFactor = 5.0,
            .thresholdValue = 2.0,
            .sampFrac = 0.03,
            .readout = "EcalEndcapPInsertHits",
          },
          app   // TODO: Remove me once fixed
        ));
        app->Add(new JChainMultifactoryGeneratorT<ProtoCluster_factory_EcalEndcapPInsertTruthProtoClusters>(
          "EcalEndcapPInsertTruthProtoClusters", {"EcalEndcapPInsertRecHits", "EcalEndcapPInsertHits"}, {"EcalEndcapPInsertTruthProtoClusters"},
          app   // TODO: Remove me once fixed
        ));
        app->Add(new JChainFactoryGeneratorT<ProtoCluster_factory_EcalEndcapPInsertIslandProtoClusters>(
          {"EcalEndcapPInsertRecHits"}, "EcalEndcapPInsertIslandProtoClusters"
        ));

        app->Add(
          new JChainMultifactoryGeneratorT<Cluster_factory_EcalEndcapPInsertTruthClusters>(
             "EcalEndcapPInsertTruthClusters",
            {"EcalEndcapPInsertTruthProtoClusters",        // edm4eic::ProtoClusterCollection
             "EcalEndcapPInsertHits"},                     // edm4hep::SimCalorimeterHitCollection
            {"EcalEndcapPInsertTruthClusters",             // edm4eic::Cluster
             "EcalEndcapPInsertTruthClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
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

        app->Add(
          new JChainMultifactoryGeneratorT<Cluster_factory_EcalEndcapPInsertClusters>(
             "EcalEndcapPInsertClusters",
            {"EcalEndcapPInsertIslandProtoClusters",  // edm4eic::ProtoClusterCollection
             "EcalEndcapPInsertHits"},                // edm4hep::SimCalorimeterHitCollection
            {"EcalEndcapPInsertClusters",             // edm4eic::Cluster
             "EcalEndcapPInsertClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
            {
              .energyWeight = "log",
              .moduleDimZName = "",
              .sampFrac = 1.0,
              .logWeightBase = 3.6,
              .depthCorrection = 0.0,
              .enableEtaBounds = false,
            },
            app   // TODO: Remove me once fixed
          )
        );

    }
}
