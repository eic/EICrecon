// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include "extensions/jana/JChainMultifactoryGeneratorT.h"

#include "factories/calorimetry/CalorimeterClusterRecoCoG_factoryT.h"
#include "factories/calorimetry/CalorimeterHitDigi_factoryT.h"
#include "factories/calorimetry/CalorimeterHitReco_factoryT.h"
#include "factories/calorimetry/CalorimeterTruthClustering_factoryT.h"
#include "factories/calorimetry/CalorimeterIslandCluster_factoryT.h"

extern "C" {
    void InitPlugin(JApplication *app) {

        using namespace eicrecon;

        InitJANAPlugin(app);

        app->Add(new JChainMultifactoryGeneratorT<CalorimeterHitDigi_factoryT>(
          "EcalEndcapPRawHits", {"EcalEndcapPHits"}, {"EcalEndcapPRawHits"},
          {
            .eRes = {0.00340 * sqrt(dd4hep::GeV), 0.0009, 0.0 * dd4hep::GeV}, // (0.340% / sqrt(E)) \oplus 0.09%
            .tRes = 0.0,
	    .threshold = 15.0 * dd4hep::MeV,	     
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
        app->Add(new JChainMultifactoryGeneratorT<CalorimeterHitReco_factoryT>(
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
        app->Add(new JChainMultifactoryGeneratorT<CalorimeterTruthClustering_factoryT>(
          "EcalEndcapPTruthProtoClusters", {"EcalEndcapPRecHits", "EcalEndcapPHits"}, {"EcalEndcapPTruthProtoClusters"},
          app   // TODO: Remove me once fixed
        ));
        app->Add(new JChainMultifactoryGeneratorT<CalorimeterIslandCluster_factoryT>(
          "EcalEndcapPIslandProtoClusters", {"EcalEndcapPRecHits"}, {"EcalEndcapPIslandProtoClusters"},
          {
            .sectorDist = 5.0 * dd4hep::cm,
            .localDistXY = {10.0 * dd4hep::cm, 10.0 * dd4hep::cm},
            .dimScaledLocalDistXY = {1.5, 1.5},
            .splitCluster = true,
            .minClusterHitEdep = 0.0 * dd4hep::MeV,
            .minClusterCenterEdep = 10.0 * dd4hep::MeV,
            .transverseEnergyProfileMetric = "globalDistEtaPhi",
            .transverseEnergyProfileScale = 0.04,
          },
          app   // TODO: Remove me once fixed
        ));

        app->Add(
          new JChainMultifactoryGeneratorT<CalorimeterClusterRecoCoG_factoryT>(
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
          new JChainMultifactoryGeneratorT<CalorimeterClusterRecoCoG_factoryT>(
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

        app->Add(new JChainMultifactoryGeneratorT<CalorimeterHitDigi_factoryT>(
          "EcalEndcapPInsertRawHits", {"EcalEndcapPInsertHits"}, {"EcalEndcapPInsertRawHits"},
          {
            .eRes = {0.00340 * sqrt(dd4hep::GeV), 0.0009, 0.0 * dd4hep::GeV}, // (0.340% / sqrt(E)) \oplus 0.09%
            .tRes = 0.0 * dd4hep::ns,
	     // .threshold = 250.0 * dd4hep::keV,
	    .threshold = 15.0 * dd4hep::MeV,
            .capADC = 16384,
            .dyRangeADC = 3 * dd4hep::GeV,
            .pedMeanADC = 100,
            .pedSigmaADC = 0.7,
            .resolutionTDC = 10 * dd4hep::picosecond,
            .corrMeanScale = 0.03,
          },
          app   // TODO: Remove me once fixed
        ));
        app->Add(new JChainMultifactoryGeneratorT<CalorimeterHitReco_factoryT>(
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
        app->Add(new JChainMultifactoryGeneratorT<CalorimeterTruthClustering_factoryT>(
          "EcalEndcapPInsertTruthProtoClusters", {"EcalEndcapPInsertRecHits", "EcalEndcapPInsertHits"}, {"EcalEndcapPInsertTruthProtoClusters"},
          app   // TODO: Remove me once fixed
        ));
        app->Add(new JChainMultifactoryGeneratorT<CalorimeterIslandCluster_factoryT>(
          "EcalEndcapPInsertIslandProtoClusters", {"EcalEndcapPInsertRecHits"}, {"EcalEndcapPInsertIslandProtoClusters"},
          {
            .sectorDist = 5.0 * dd4hep::cm,
            .localDistXY = {10.0 * dd4hep::cm, 10.0 * dd4hep::cm},
            .dimScaledLocalDistXY = {1.5,1.5},
            .splitCluster = false,
            .minClusterHitEdep = 0.0 * dd4hep::MeV,
            .minClusterCenterEdep = 10.0 * dd4hep::MeV,
            .transverseEnergyProfileMetric = "globalDistEtaPhi",
            .transverseEnergyProfileScale = 1.,
          },
          app   // TODO: Remove me once fixed
        ));

        app->Add(
          new JChainMultifactoryGeneratorT<CalorimeterClusterRecoCoG_factoryT>(
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
          new JChainMultifactoryGeneratorT<CalorimeterClusterRecoCoG_factoryT>(
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
