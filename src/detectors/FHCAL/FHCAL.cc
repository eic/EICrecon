// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include "extensions/jana/JChainMultifactoryGeneratorT.h"

#include "factories/calorimetry/CalorimeterClusterRecoCoG_factoryT.h"
#include "factories/calorimetry/CalorimeterHitDigi_factoryT.h"
#include "factories/calorimetry/CalorimeterHitReco_factoryT.h"
#include "factories/calorimetry/CalorimeterHitsMerger_factoryT.h"
#include "factories/calorimetry/CalorimeterTruthClustering_factoryT.h"
#include "factories/calorimetry/CalorimeterIslandCluster_factoryT.h"

extern "C" {
    void InitPlugin(JApplication *app) {

        using namespace eicrecon;

        InitJANAPlugin(app);

        app->Add(new JChainMultifactoryGeneratorT<CalorimeterHitDigi_factoryT>(
           "HcalEndcapPRawHits", {"HcalEndcapPHits"}, {"HcalEndcapPRawHits"},
           {
             .eRes = {},
             .tRes = 0.001 * dd4hep::ns,
             .capADC = 65536,
             .dyRangeADC = 1 * dd4hep::GeV,
             .pedMeanADC = 20,
             .pedSigmaADC = 0.8,
             .resolutionTDC = 10 * dd4hep::picosecond,
             .corrMeanScale = 1.0,
           },
           app   // TODO: Remove me once fixed
        ));
        app->Add(new JChainMultifactoryGeneratorT<CalorimeterHitReco_factoryT>(
          "HcalEndcapPRecHits", {"HcalEndcapPRawHits"}, {"HcalEndcapPRecHits"},
          {
            .capADC = 65536,
            .dyRangeADC = 1 * dd4hep::GeV,
            .pedMeanADC = 20,
            .pedSigmaADC = 0.8,
            .resolutionTDC = 10 * dd4hep::picosecond,
            .thresholdFactor = 1.0,
            .thresholdValue = 3.0,
            .sampFrac = 0.033,
            .readout = "HcalEndcapPHits",
          },
          app   // TODO: Remove me once fixed
        ));
        app->Add(new JChainMultifactoryGeneratorT<CalorimeterHitsMerger_factoryT>(
          "HcalEndcapPMergedHits", {"HcalEndcapPRecHits"}, {"HcalEndcapPMergedHits"},
          {
            .readout = "HcalEndcapPHits",
            .fields = {"layer", "slice"},
            .refs = {1, 0},
          },
          app   // TODO: Remove me once fixed
        ));
        app->Add(new JChainMultifactoryGeneratorT<CalorimeterTruthClustering_factoryT>(
          "HcalEndcapPTruthProtoClusters", {"HcalEndcapPRecHits", "HcalEndcapPHits"}, {"HcalEndcapPTruthProtoClusters"},
          app   // TODO: Remove me once fixed
        ));
        app->Add(new JChainMultifactoryGeneratorT<CalorimeterIslandCluster_factoryT>(
          "HcalEndcapPIslandProtoClusters", {"HcalEndcapPRecHits"}, {"HcalEndcapPIslandProtoClusters"},
          {
            .sectorDist = 5.0 * dd4hep::cm,
            .localDistXY = {15.0*dd4hep::mm, 15.0*dd4hep::mm},
            .dimScaledLocalDistXY = {15.0*dd4hep::mm, 15.0*dd4hep::mm},
            .splitCluster = true,
            .minClusterHitEdep = 0.0 * dd4hep::MeV,
            .minClusterCenterEdep = 30.0 * dd4hep::MeV,
            .transverseEnergyProfileMetric = "globalDistEtaPhi",
            .transverseEnergyProfileScale = 1.,
          },
          app   // TODO: Remove me once fixed
        ));

        app->Add(
          new JChainMultifactoryGeneratorT<CalorimeterClusterRecoCoG_factoryT>(
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
          new JChainMultifactoryGeneratorT<CalorimeterClusterRecoCoG_factoryT>(
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

        app->Add(new JChainMultifactoryGeneratorT<CalorimeterHitDigi_factoryT>(
           "HcalEndcapPInsertRawHits", {"HcalEndcapPInsertHits"}, {"HcalEndcapPInsertRawHits"},
           {
             .eRes = {},
             .tRes = 0.0 * dd4hep::ns,
             .capADC = 32768,
             .dyRangeADC = 200 * dd4hep::MeV,
             .pedMeanADC = 400,
             .pedSigmaADC = 10,
             .resolutionTDC = 10 * dd4hep::picosecond,
             .corrMeanScale = 1.0,
           },
          app   // TODO: Remove me once fixed
        ));
        app->Add(new JChainMultifactoryGeneratorT<CalorimeterHitReco_factoryT>(
          "HcalEndcapPInsertRecHits", {"HcalEndcapPInsertRawHits"}, {"HcalEndcapPInsertRecHits"},
          {
            .capADC = 32768,
            .dyRangeADC = 200. * dd4hep::MeV,
            .pedMeanADC = 400,
            .pedSigmaADC = 10.,
            .resolutionTDC = 10 * dd4hep::picosecond,
            .thresholdFactor = 0.,
            .thresholdValue = -100.,
            .sampFrac = 0.0098,
            .readout = "HcalEndcapPInsertHits",
          },
          app   // TODO: Remove me once fixed
        ));
        app->Add(new JChainMultifactoryGeneratorT<CalorimeterHitsMerger_factoryT>(
          "HcalEndcapPInsertMergedHits", {"HcalEndcapPInsertRecHits"}, {"HcalEndcapPInsertMergedHits"},
          {
            .readout = "HcalEndcapPInsertHits",
            .fields = {"layer", "slice"},
            .refs = {1, 0},
          },
          app   // TODO: Remove me once fixed
        ));
        app->Add(new JChainMultifactoryGeneratorT<CalorimeterTruthClustering_factoryT>(
          "HcalEndcapPInsertTruthProtoClusters", {"HcalEndcapPInsertMergedHits", "HcalEndcapPInsertHits"}, {"HcalEndcapPInsertTruthProtoClusters"},
          app   // TODO: Remove me once fixed
        ));
        app->Add(new JChainMultifactoryGeneratorT<CalorimeterIslandCluster_factoryT>(
          "HcalEndcapPInsertIslandProtoClusters", {"HcalEndcapPInsertMergedHits"}, {"HcalEndcapPInsertIslandProtoClusters"},
          {
            .sectorDist = 5.0 * dd4hep::cm,
            .localDistXY = {15*dd4hep::mm, 15*dd4hep::mm},
            .dimScaledLocalDistXY = {15.0*dd4hep::mm, 15.0*dd4hep::mm},
            .splitCluster = true,
            .minClusterHitEdep = 0.0 * dd4hep::MeV,
            .minClusterCenterEdep = 30.0 * dd4hep::MeV,
            .transverseEnergyProfileMetric = "globalDistEtaPhi",
            .transverseEnergyProfileScale = 1.,
          },
          app   // TODO: Remove me once fixed
        ));

        app->Add(
          new JChainMultifactoryGeneratorT<CalorimeterClusterRecoCoG_factoryT>(
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
          new JChainMultifactoryGeneratorT<CalorimeterClusterRecoCoG_factoryT>(
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

        app->Add(new JChainMultifactoryGeneratorT<CalorimeterHitDigi_factoryT>(
          "LFHCALRawHits", {"LFHCALHits"}, {"LFHCALRawHits"},
          {
            .eRes = {},
            .tRes = 0.0 * dd4hep::ns,
            .capADC = 65536,
            .capTime = 100,
            .dyRangeADC = 1 * dd4hep::GeV,
            .pedMeanADC = 20,
            .pedSigmaADC = 0.8,
            .resolutionTDC = 10 * dd4hep::picosecond,
            .corrMeanScale = 1.0,
            .readout = "LFHCALHits",
            .fields = {"layerz"},
          },
          app   // TODO: Remove me once fixed
        ));
        app->Add(new JChainMultifactoryGeneratorT<CalorimeterHitReco_factoryT>(
          "LFHCALRecHits", {"LFHCALRawHits"}, {"LFHCALRecHits"},
          {
            .capADC = 65536,
            .dyRangeADC = 1 * dd4hep::GeV,
            .pedMeanADC = 20,
            .pedSigmaADC = 0.8,
            .resolutionTDC = 10 * dd4hep::picosecond,
            .thresholdFactor = 1.0,
            .thresholdValue = 3.0,
            .sampFrac = 0.033,
            .sampFracLayer = {
              0.019, //  0
              0.037, //  1
              0.037, //  2
              0.037, //  3
              0.037, //  4
              0.037, //  5
              0.037, //  6
              0.037, //  7
              0.037, //  8
              0.037, //  9
              0.037, // 10
              0.037, // 11
              0.037, // 12
              0.037, // 13
            },
            .readout = "LFHCALHits",
            .layerField = "rlayerz",
          },
          app   // TODO: Remove me once fixed
        ));
        app->Add(new JChainMultifactoryGeneratorT<CalorimeterTruthClustering_factoryT>(
          "LFHCALTruthProtoClusters", {"LFHCALRecHits", "LFHCALHits"}, {"LFHCALTruthProtoClusters"},
          app   // TODO: Remove me once fixed
        ));

        // Magic constants:
        //  54 - number of modules in a row/column
        //  2  - number of towers in a module
        std::string cellIdx_1  = "(54*2-moduleIDx_1*2+towerx_1)";
        std::string cellIdx_2  = "(54*2-moduleIDx_2*2+towerx_2)";
        std::string cellIdy_1  = "(54*2-moduleIDy_1*2+towery_1)";
        std::string cellIdy_2  = "(54*2-moduleIDy_2*2+towery_2)";
        std::string cellIdz_1  = "rlayerz_1";
        std::string cellIdz_2  = "rlayerz_2";
        std::string deltaX     = Form("abs(%s-%s)", cellIdx_2.data(), cellIdx_1.data());
        std::string deltaY     = Form("abs(%s-%s)", cellIdy_2.data(), cellIdy_1.data());
        std::string deltaZ     = Form("abs(%s-%s)", cellIdz_2.data(), cellIdz_1.data());
        std::string neighbor   = Form("(%s+%s+%s==1)", deltaX.data(), deltaY.data(), deltaZ.data());
        std::string corner2D   = Form("((%s==0&&%s==1&&%s==1)||(%s==1&&%s==0&&%s==1)||(%s==1&&%s==1&&%s==0))",
                                  deltaZ.data(), deltaX.data(), deltaY.data(),
                                  deltaZ.data(), deltaX.data(), deltaY.data(),
                                  deltaZ.data(), deltaX.data(), deltaY.data());

        app->Add(new JChainMultifactoryGeneratorT<CalorimeterIslandCluster_factoryT>(
          "LFHCALIslandProtoClusters", {"LFHCALRecHits"}, {"LFHCALIslandProtoClusters"},
          {
            .adjacencyMatrix = Form("%s||%s", neighbor.data(), corner2D.data()),
            .readout = "LFHCALHits",
            .sectorDist = 0 * dd4hep::cm,
            .splitCluster = false,
            .minClusterHitEdep = 1 * dd4hep::MeV,
            .minClusterCenterEdep = 100.0 * dd4hep::MeV,
            .transverseEnergyProfileMetric = "globalDistEtaPhi",
            .transverseEnergyProfileScale = 1.,
          },
          app   // TODO: Remove me once fixed
        ));

        app->Add(
          new JChainMultifactoryGeneratorT<CalorimeterClusterRecoCoG_factoryT>(
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
          new JChainMultifactoryGeneratorT<CalorimeterClusterRecoCoG_factoryT>(
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
