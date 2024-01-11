// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

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
          "EcalEndcapPRawHits", {"EcalEndcapPHits"}, {"EcalEndcapPRawHits"},
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
            .corrMeanScale = 0.03,
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
            .sampFrac  =0.03,
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
          new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
             "EcalEndcapPTruthClusters",
            {"EcalEndcapPTruthProtoClusters",        // edm4eic::ProtoClusterCollection
             "EcalEndcapPHits"},                     // edm4hep::SimCalorimeterHitCollection
            {"EcalEndcapPTruthClusters",             // edm4eic::Cluster
             "EcalEndcapPTruthClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
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
          new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
             "EcalEndcapPClusters",
            {"EcalEndcapPIslandProtoClusters",  // edm4eic::ProtoClusterCollection
             "EcalEndcapPHits"},                // edm4hep::SimCalorimeterHitCollection
            {"EcalEndcapPClusters",             // edm4eic::Cluster
             "EcalEndcapPClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
            {
              .energyWeight = "log",
              .sampFrac = 1.0,
              .logWeightBase = 3.6,
              .enableEtaBounds = false,
            },
            app   // TODO: Remove me once fixed
          )
        );

        // Insert is identical to regular Ecal
        app->Add(new JOmniFactoryGeneratorT<CalorimeterHitDigi_factory>(
          "EcalEndcapPInsertRawHits", {"EcalEndcapPInsertHits"}, {"EcalEndcapPInsertRawHits"},
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
            .corrMeanScale = 0.03,
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
            .sampFrac  =0.03,
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
          new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
             "EcalEndcapPInsertTruthClusters",
            {"EcalEndcapPInsertTruthProtoClusters",        // edm4eic::ProtoClusterCollection
             "EcalEndcapPInsertHits"},                     // edm4hep::SimCalorimeterHitCollection
            {"EcalEndcapPInsertTruthClusters",             // edm4eic::Cluster
             "EcalEndcapPInsertTruthClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
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
          new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
             "EcalEndcapPInsertClusters",
            {"EcalEndcapPInsertIslandProtoClusters",  // edm4eic::ProtoClusterCollection
             "EcalEndcapPInsertHits"},                // edm4hep::SimCalorimeterHitCollection
            {"EcalEndcapPInsertClusters",             // edm4eic::Cluster
             "EcalEndcapPInsertClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
            {
              .energyWeight = "log",
              .sampFrac = 1.0,
              .logWeightBase = 3.6,
              .enableEtaBounds = false,
            },
            app   // TODO: Remove me once fixed
          )
        );

    }
}
