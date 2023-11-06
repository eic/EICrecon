// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplication.h>
#include <math.h>
#include <string>

#include "algorithms/calorimetry/CalorimeterHitDigiConfig.h"
#include "extensions/jana/JChainMultifactoryGeneratorT.h"
#include "factories/calorimetry/CalorimeterClusterRecoCoG_factoryT.h"
#include "factories/calorimetry/CalorimeterHitDigi_factoryT.h"
#include "factories/calorimetry/CalorimeterHitReco_factoryT.h"
#include "factories/calorimetry/CalorimeterIslandCluster_factoryT.h"
#include "factories/calorimetry/CalorimeterTruthClustering_factoryT.h"

extern "C" {
    void InitPlugin(JApplication *app) {

        using namespace eicrecon;

        InitJANAPlugin(app);

        // Make sure digi and reco use the same value
        decltype(CalorimeterHitDigiConfig::capADC)        EcalEndcapN_capADC = 16384; //65536,  16bit ADC
        decltype(CalorimeterHitDigiConfig::dyRangeADC)    EcalEndcapN_dyRangeADC = 20.0 * dd4hep::GeV;
        decltype(CalorimeterHitDigiConfig::pedMeanADC)    EcalEndcapN_pedMeanADC = 20;
        decltype(CalorimeterHitDigiConfig::pedSigmaADC)   EcalEndcapN_pedSigmaADC = 1;
        decltype(CalorimeterHitDigiConfig::resolutionTDC) EcalEndcapN_resolutionTDC = 10 * dd4hep::picosecond;
        app->Add(new JChainMultifactoryGeneratorT<CalorimeterHitDigi_factoryT>(
          "EcalEndcapNRawHits", {"EcalEndcapNHits"}, {"EcalEndcapNRawHits"},
          {
            .eRes = {0.0 * sqrt(dd4hep::GeV), 0.02, 0.0 * dd4hep::GeV},
            .tRes = 0.0 * dd4hep::ns,
            .threshold =  5.0 * dd4hep::MeV,
            .capADC = EcalEndcapN_capADC,
            .dyRangeADC = EcalEndcapN_dyRangeADC,
            .pedMeanADC = EcalEndcapN_pedMeanADC,
            .pedSigmaADC = EcalEndcapN_pedSigmaADC,
            .resolutionTDC = EcalEndcapN_resolutionTDC,
            .corrMeanScale = 1.0,
          },
          app   // TODO: Remove me once fixed
        ));
        app->Add(new JChainMultifactoryGeneratorT<CalorimeterHitReco_factoryT>(
          "EcalEndcapNRecHits", {"EcalEndcapNRawHits"}, {"EcalEndcapNRecHits"},
          {
            .capADC = EcalEndcapN_capADC,
            .dyRangeADC = EcalEndcapN_dyRangeADC,
            .pedMeanADC = EcalEndcapN_pedMeanADC,
            .pedSigmaADC = EcalEndcapN_pedSigmaADC,
            .resolutionTDC = EcalEndcapN_resolutionTDC,
            .thresholdFactor = 0.0,
            .thresholdValue = 0.0,
            .sampFrac = 0.998,
            .readout = "EcalEndcapNHits",
            .sectorField = "sector",
          },
          app   // TODO: Remove me once fixed
        ));
        app->Add(new JChainMultifactoryGeneratorT<CalorimeterTruthClustering_factoryT>(
          "EcalEndcapNTruthProtoClusters", {"EcalEndcapNRecHits", "EcalEndcapNHits"}, {"EcalEndcapNTruthProtoClusters"},
          app   // TODO: Remove me once fixed
        ));
        app->Add(new JChainMultifactoryGeneratorT<CalorimeterIslandCluster_factoryT>(
          "EcalEndcapNIslandProtoClusters", {"EcalEndcapNRecHits"}, {"EcalEndcapNIslandProtoClusters"},
          {
            .adjacencyMatrix = "(abs(row_1 - row_2) + abs(column_1 - column_2)) == 1",
            .readout = "EcalEndcapNHits",
            .sectorDist = 5.0 * dd4hep::cm,
            .splitCluster = true,
            .minClusterHitEdep = 1.0 * dd4hep::MeV,
            .minClusterCenterEdep = 30.0 * dd4hep::MeV,
            .transverseEnergyProfileMetric = "globalDistEtaPhi",
            .transverseEnergyProfileScale = 0.08,
          },
          app   // TODO: Remove me once fixed
        ));

        app->Add(
          new JChainMultifactoryGeneratorT<CalorimeterClusterRecoCoG_factoryT>(
             "EcalEndcapNTruthClusters",
            {"EcalEndcapNTruthProtoClusters",        // edm4eic::ProtoClusterCollection
             "EcalEndcapNHits"},                     // edm4hep::SimCalorimeterHitCollection
            {"EcalEndcapNTruthClusters",             // edm4eic::Cluster
             "EcalEndcapNTruthClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
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

        app->Add(
          new JChainMultifactoryGeneratorT<CalorimeterClusterRecoCoG_factoryT>(
             "EcalEndcapNClusters",
            {"EcalEndcapNIslandProtoClusters",  // edm4eic::ProtoClusterCollection
             "EcalEndcapNHits"},                // edm4hep::SimCalorimeterHitCollection
            {"EcalEndcapNClusters",             // edm4eic::Cluster
             "EcalEndcapNClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
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
