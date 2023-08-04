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
          "HcalBarrelRawHits", {"HcalBarrelHits"}, {"HcalBarrelRawHits"},
          {
            .eRes = {},
            .tRes = 0.0 * dd4hep::ns,
            .capADC = 65536,
            .capTime = 100, // given in ns, 4 samples in HGCROC
            .dyRangeADC = 1.0 * dd4hep::GeV,
            .pedMeanADC = 10,
            .pedSigmaADC = 2.0,
            .resolutionTDC = 1.0 * dd4hep::picosecond,
            .corrMeanScale = 1.0,
            .readout = "HcalBarrelHits",
          },
          app   // TODO: Remove me once fixed
        ));
        app->Add(new JChainMultifactoryGeneratorT<CalorimeterHitReco_factoryT>(
          "HcalBarrelRecHits", {"HcalBarrelRawHits"}, {"HcalBarrelRecHits"},
          {
            .capADC = 65536,
            .dyRangeADC = 1.0 * dd4hep::GeV,
            .pedMeanADC = 10,
            .pedSigmaADC = 2.0,
            .resolutionTDC = 1.0 * dd4hep::picosecond,
            .thresholdFactor = 5.0,
            .thresholdValue = 1.0,
            .sampFrac = 0.033, // average, from sPHENIX simulations
            .readout = "HcalBarrelHits",
            .layerField = "tower",
            .sectorField = "sector",
          },
          app   // TODO: Remove me once fixed
        ));
        app->Add(new JChainMultifactoryGeneratorT<CalorimeterTruthClustering_factoryT>(
          "HcalBarrelTruthProtoClusters", {"HcalBarrelRecHits", "HcalBarrelHits"}, {"HcalBarrelTruthProtoClusters"},
          app   // TODO: Remove me once fixed
        ));
        app->Add(new JChainMultifactoryGeneratorT<CalorimeterIslandCluster_factoryT>(
          "HcalBarrelIslandProtoClusters", {"HcalBarrelRecHits"}, {"HcalBarrelIslandProtoClusters"},
          {
            .adjacencyMatrix =
              "("
              "  abs(fmod(tower_1, 24) - fmod(tower_2, 24))"
              "  + min("
              "      abs((sector_1 - sector_2) * (2 * 5) + (floor(tower_1 / 24) - floor(tower_2 / 24)) * 5 + fmod(tile_1, 5) - fmod(tile_2, 5)),"
              "      (32 * 2 * 5) - abs((sector_1 - sector_2) * (2 * 5) + (floor(tower_1 / 24) - floor(tower_2 / 24)) * 5 + fmod(tile_1, 5) - fmod(tile_2, 5))"
              "    )"
              ") == 1",
            .readout = "HcalBarrelHits",
            .sectorDist = 5.0 * dd4hep::cm,
            .localDistXY = {15*dd4hep::mm, 15*dd4hep::mm},
            .dimScaledLocalDistXY = {50.0*dd4hep::mm, 50.0*dd4hep::mm},
            .splitCluster = false,
            .minClusterHitEdep = 3.0 * dd4hep::MeV,
            .minClusterCenterEdep = 30.0 * dd4hep::MeV,
            .transverseEnergyProfileMetric = "globalDistEtaPhi",
            .transverseEnergyProfileScale = 1.,
          },
          app   // TODO: Remove me once fixed
        ));

        app->Add(
          new JChainMultifactoryGeneratorT<CalorimeterClusterRecoCoG_factoryT>(
             "HcalBarrelClusters",
            {"HcalBarrelIslandProtoClusters",  // edm4eic::ProtoClusterCollection
             "HcalBarrelHits"},                // edm4hep::SimCalorimeterHitCollection
            {"HcalBarrelClusters",             // edm4eic::Cluster
             "HcalBarrelClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
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
          new JChainMultifactoryGeneratorT<CalorimeterClusterRecoCoG_factoryT>(
             "HcalBarrelTruthClusters",
            {"HcalBarrelTruthProtoClusters",        // edm4eic::ProtoClusterCollection
             "HcalBarrelHits"},                     // edm4hep::SimCalorimeterHitCollection
            {"HcalBarrelTruthClusters",             // edm4eic::Cluster
             "HcalBarrelTruthClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
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

    }
}
