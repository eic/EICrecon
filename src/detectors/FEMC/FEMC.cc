// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2021 - 2024, Chao Peng, Sylvester Joosten, Whitney Armstrong, David Lawrence, Friederike Bock, Wouter Deconinck, Kolja Kauder, Sebouh Paul

#include <edm4eic/EDM4eicVersion.h>
#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplication.h>
#include <math.h>
#include <string>

#include "algorithms/calorimetry/CalorimeterHitDigiConfig.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/calorimetry/CalorimeterClusterRecoCoG_factory.h"
#include "factories/calorimetry/CalorimeterHitDigi_factory.h"
#include "factories/calorimetry/CalorimeterHitReco_factory.h"
#include "factories/calorimetry/CalorimeterIslandCluster_factory.h"
#include "factories/calorimetry/CalorimeterTruthClustering_factory.h"
#include "factories/calorimetry/CalorimeterClusterShape_factory.h"
#include "factories/calorimetry/TrackClusterMergeSplitter_factory.h"

extern "C" {
    void InitPlugin(JApplication *app) {

        using namespace eicrecon;

        InitJANAPlugin(app);
        // Make sure digi and reco use the same value
        decltype(CalorimeterHitDigiConfig::capADC)        EcalEndcapP_capADC = 16384; //16384, assuming 14 bits. For approximate HGCROC resolution use 65536
        decltype(CalorimeterHitDigiConfig::dyRangeADC)    EcalEndcapP_dyRangeADC = 3 * dd4hep::GeV;
        decltype(CalorimeterHitDigiConfig::pedMeanADC)    EcalEndcapP_pedMeanADC = 200;
        decltype(CalorimeterHitDigiConfig::pedSigmaADC)   EcalEndcapP_pedSigmaADC = 2.4576;
        decltype(CalorimeterHitDigiConfig::resolutionTDC) EcalEndcapP_resolutionTDC = 10 * dd4hep::picosecond;
        app->Add(new JOmniFactoryGeneratorT<CalorimeterHitDigi_factory>(
          "EcalEndcapPRawHits",
          {"EcalEndcapPHits"},
#if EDM4EIC_VERSION_MAJOR >= 7
          {"EcalEndcapPRawHits", "EcalEndcapPRawHitAssociations"},
#else
          {"EcalEndcapPRawHits"},
#endif
          {
            .eRes = {0.11333 * sqrt(dd4hep::GeV), 0.03, 0.0 * dd4hep::GeV}, // (11.333% / sqrt(E)) \oplus 3%
            .tRes = 0.0,
            .threshold = 0.0,
             // .threshold = 15 * dd4hep::MeV for a single tower, applied on ADC level
            .capADC = EcalEndcapP_capADC,
            .capTime =  100, // given in ns, 4 samples in HGCROC
            .dyRangeADC = EcalEndcapP_dyRangeADC,
            .pedMeanADC = EcalEndcapP_pedMeanADC,
            .pedSigmaADC = EcalEndcapP_pedSigmaADC,
            .resolutionTDC = EcalEndcapP_resolutionTDC,
            .corrMeanScale = "0.03",
            .readout = "EcalEndcapPHits",
          },
          app   // TODO: Remove me once fixed
        ));
        app->Add(new JOmniFactoryGeneratorT<CalorimeterHitReco_factory>(
          "EcalEndcapPRecHits", {"EcalEndcapPRawHits"}, {"EcalEndcapPRecHits"},
          {
            .capADC = EcalEndcapP_capADC,
            .dyRangeADC = EcalEndcapP_dyRangeADC,
            .pedMeanADC = EcalEndcapP_pedMeanADC,
            .pedSigmaADC = EcalEndcapP_pedSigmaADC,
            .resolutionTDC = EcalEndcapP_resolutionTDC,
            .thresholdFactor = 0.0,
            .thresholdValue = 2, // The ADC of a 15 MeV particle is adc = 200 + 15 * 0.03 * ( 1.0 + 0) / 3000 * 16384 = 200 + 2.4576
            .sampFrac  = "0.03",
            .readout = "EcalEndcapPHits",
          },
          app   // TODO: Remove me once fixed
        ));
        app->Add(new JOmniFactoryGeneratorT<CalorimeterTruthClustering_factory>(
          "EcalEndcapPTruthProtoClusters", {"EcalEndcapPRecHits", "EcalEndcapPHits"}, {"EcalEndcapPTruthProtoClusters"},
          app   // TODO: Remove me once fixed
        ));
        app->Add(new JOmniFactoryGeneratorT<CalorimeterIslandCluster_factory>(
          "EcalEndcapPIslandProtoClusters", {"EcalEndcapPRecHits"}, {"EcalEndcapPIslandProtoClusters"},
          {
            .sectorDist = 5.0 * dd4hep::cm,
            .dimScaledLocalDistXY = {1.5,1.5},
            .splitCluster = false,
            .minClusterHitEdep = 0.0 * dd4hep::MeV,
            .minClusterCenterEdep = 60.0 * dd4hep::MeV,
            .transverseEnergyProfileMetric = "dimScaledLocalDistXY",
            .transverseEnergyProfileScale = 1.,
          },
          app   // TODO: Remove me once fixed
        ));

        app->Add(
          new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
             "EcalEndcapPTruthClustersWithoutShapes",
            {"EcalEndcapPTruthProtoClusters", // edm4eic::ProtoClusterCollection
#if EDM4EIC_VERSION_MAJOR >= 7
             "EcalEndcapPRawHitAssociations"}, // edm4eic::MCRecoCalorimeterHitAssociationCollection
#else
             "EcalEndcapPHits"}, // edm4hep::SimCalorimeterHitCollection
#endif
            {"EcalEndcapPTruthClustersWithoutShapes", // edm4eic::Cluster
             "EcalEndcapPTruthClusterAssociationsWithoutShapes"}, // edm4eic::MCRecoClusterParticleAssociation
            {
              .energyWeight = "log",
              .sampFrac = 1.0,
              .logWeightBase = 6.2,
              .enableEtaBounds = true
            },
            app   // TODO: Remove me once fixed
          )
        );

        app->Add(
          new JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>(
            "EcalEndcapPTruthClusters",
            {"EcalEndcapPTruthClustersWithoutShapes",
             "EcalEndcapPTruthClusterAssociationsWithoutShapes"},
            {"EcalEndcapPTruthClusters",
             "EcalEndcapPTruthClusterAssociations"},
            {
              .energyWeight = "log",
              .logWeightBase = 6.2
            },
            app
          )
        );

        app->Add(
          new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
             "EcalEndcapPClustersWithoutShapes",
            {"EcalEndcapPIslandProtoClusters", // edm4eic::ProtoClusterCollection
#if EDM4EIC_VERSION_MAJOR >= 7
             "EcalEndcapPRawHitAssociations"}, // edm4eic::MCRecoCalorimeterHitAssociationCollection
#else
             "EcalEndcapPHits"}, // edm4hep::SimCalorimeterHitCollection
#endif
            {"EcalEndcapPClustersWithoutShapes", // edm4eic::Cluster
             "EcalEndcapPClusterAssociationsWithoutShapes"}, // edm4eic::MCRecoClusterParticleAssociation
            {
              .energyWeight = "log",
              .sampFrac = 1.0,
              .logWeightBase = 3.6,
              .enableEtaBounds = false,
            },
            app   // TODO: Remove me once fixed
          )
        );

        app->Add(
          new JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>(
            "EcalEndcapPClusters",
            {"EcalEndcapPClustersWithoutShapes",
             "EcalEndcapPClusterAssociationsWithoutShapes"},
            {"EcalEndcapPClusters",
             "EcalEndcapPClusterAssociations"},
            {
              .energyWeight = "log",
              .logWeightBase = 3.6
            },
            app
          )
        );

        app->Add(
          new JOmniFactoryGeneratorT<TrackClusterMergeSplitter_factory>(
            "EcalEndcapPSplitMergeClustersWithoutShapes",
            {"EcalEndcapPClusters",
             "CalorimeterTrackProjections"},
            {"EcalEndcapPSplitMergeClustersWithoutShapes",
#if EDM4EIC_VERSION_MAJOR >= 8
             "EcalEndcapPTrackSplitMergeClusterMatches"},
#endif
            {
              .idCalo = "EcalEndcapP_ID",
              .minSigCut = -2.0,
              .avgEP = 1.0,
              .sigEP = 0.10,
              .drAdd = 0.30,
              .sampFrac = 1.0,
              .transverseEnergyProfileScale = 1.0
            },
            app   // TODO: remove me once fixed
          )
        );

        app->Add(
          new JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>(
            "EcalEndcapPSplitMergeClusters",
            {"EcalEndcapPSplitMergeClustersWithoutShapes",
             "EcalEndcapPSplitMergeClusterAssociationsWithoutShapes"},
            {"EcalEndcapPSplitMergeClusters",
             "EcalEndcapPSplitMergeClusterAssociations"},
            {
              .energyWeight = "log",
              .logWeightBase = 3.6
            },
            app
          )
        );

        // Insert is identical to regular Ecal
        app->Add(new JOmniFactoryGeneratorT<CalorimeterHitDigi_factory>(
          "EcalEndcapPInsertRawHits",
          {"EcalEndcapPInsertHits"},
#if EDM4EIC_VERSION_MAJOR >= 7
          {"EcalEndcapPInsertRawHits", "EcalEndcapPInsertRawHitAssociations"},
#else
          {"EcalEndcapPInsertRawHits"},
#endif
          {
            .eRes = {0.11333 * sqrt(dd4hep::GeV), 0.03, 0.0 * dd4hep::GeV}, // (11.333% / sqrt(E)) \oplus 3%
            .tRes = 0.0,
            .threshold = 0.0,
             // .threshold = 15 * dd4hep::MeV for a single tower, applied on ADC level
            .capADC = EcalEndcapP_capADC,
            .capTime =  100, // given in ns, 4 samples in HGCROC
            .dyRangeADC = EcalEndcapP_dyRangeADC,
            .pedMeanADC = EcalEndcapP_pedMeanADC,
            .pedSigmaADC = EcalEndcapP_pedSigmaADC,
            .resolutionTDC = EcalEndcapP_resolutionTDC,
            .corrMeanScale = "0.03",
            .readout = "EcalEndcapPInsertHits",
          },
          app   // TODO: Remove me once fixed
        ));
        app->Add(new JOmniFactoryGeneratorT<CalorimeterHitReco_factory>(
          "EcalEndcapPInsertRecHits", {"EcalEndcapPInsertRawHits"}, {"EcalEndcapPInsertRecHits"},
          {
            .capADC = EcalEndcapP_capADC,
            .dyRangeADC = EcalEndcapP_dyRangeADC,
            .pedMeanADC = EcalEndcapP_pedMeanADC,
            .pedSigmaADC = EcalEndcapP_pedSigmaADC,
            .resolutionTDC = EcalEndcapP_resolutionTDC,
            .thresholdFactor = 0.0,
            .thresholdValue = 2, // The ADC of a 15 MeV particle is adc = 200 + 15 * 0.03 * ( 1.0 + 0) / 3000 * 16384 = 200 + 2.4576
            .sampFrac = "0.03",
            .readout = "EcalEndcapPInsertHits",
          },
          app   // TODO: Remove me once fixed
        ));
        app->Add(new JOmniFactoryGeneratorT<CalorimeterTruthClustering_factory>(
          "EcalEndcapPInsertTruthProtoClusters", {"EcalEndcapPInsertRecHits", "EcalEndcapPInsertHits"}, {"EcalEndcapPInsertTruthProtoClusters"},
          app   // TODO: Remove me once fixed
        ));
        app->Add(new JOmniFactoryGeneratorT<CalorimeterIslandCluster_factory>(
          "EcalEndcapPInsertIslandProtoClusters", {"EcalEndcapPInsertRecHits"}, {"EcalEndcapPInsertIslandProtoClusters"},
          {
            .sectorDist = 5.0 * dd4hep::cm,
            .dimScaledLocalDistXY = {1.5,1.5},
            .splitCluster = false,
            .minClusterHitEdep = 0.0 * dd4hep::MeV,
            .minClusterCenterEdep = 60.0 * dd4hep::MeV,
            .transverseEnergyProfileMetric = "dimScaledLocalDistXY",
            .transverseEnergyProfileScale = 1.,
          },
          app   // TODO: Remove me once fixed
        ));

        app->Add(
          new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
             "EcalEndcapPInsertTruthClustersWithoutShapes",
            {"EcalEndcapPInsertTruthProtoClusters", // edm4eic::ProtoClusterCollection
#if EDM4EIC_VERSION_MAJOR >= 7
             "EcalEndcapPInsertRawHitAssociations"}, // edm4eic::MCRecoCalorimeterHitCollection
#else
             "EcalEndcapPInsertHits"}, // edm4hep::SimCalorimeterHitCollection
#endif
            {"EcalEndcapPInsertTruthClustersWithoutShapes", // edm4eic::Cluster
             "EcalEndcapPInsertTruthClusterAssociationsWithoutShapes"}, // edm4eic::MCRecoClusterParticleAssociation
            {
              .energyWeight = "log",
              .sampFrac = 1.0,
              .logWeightBase = 6.2,
              .enableEtaBounds = true
            },
            app   // TODO: Remove me once fixed
          )
        );

        app->Add(
          new JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>(
            "EcalEndcapPInsertTruthClusters",
            {"EcalEndcapPInsertTruthClustersWithoutShapes",
             "EcalEndcapPInsertTruthClusterAssociationsWithoutShapes"},
            {"EcalEndcapPInsertTruthClusters",
             "EcalEndcapPInsertTruthClusterAssociations"},
            {
              .energyWeight = "log",
              .logWeightBase = 6.2
            },
            app
          )
        );

        app->Add(
          new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
             "EcalEndcapPInsertClustersWithoutShapes",
            {"EcalEndcapPInsertIslandProtoClusters", // edm4eic::ProtoClusterCollection
#if EDM4EIC_VERSION_MAJOR >= 7
             "EcalEndcapPInsertRawHitAssociations"}, // edm4eic::MCRecoCalorimeterHitCollection
#else
             "EcalEndcapPInsertHits"}, // edm4hep::SimCalorimeterHitCollection
#endif
            {"EcalEndcapPInsertClustersWithoutShapes", // edm4eic::Cluster
             "EcalEndcapPInsertClusterAssociationsWithoutShapes"}, // edm4eic::MCRecoClusterParticleAssociation
            {
              .energyWeight = "log",
              .sampFrac = 1.0,
              .logWeightBase = 3.6,
              .enableEtaBounds = false,
            },
            app   // TODO: Remove me once fixed
          )
        );

        app->Add(
          new JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>(
            "EcalEndcapPInsertClusters",
            {"EcalEndcapPInsertClustersWithoutShapes",
             "EcalEndcapPInsertClusterAssociationsWithoutShapes"},
            {"EcalEndcapPInsertClusters",
             "EcalEndcapPInsertClusterAssociations"},
            {
              .energyWeight = "log",
              .logWeightBase = 3.6
            },
            app
          )
        );
    }
}
