// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 - 2025 Friederike Bock, Wouter Deconinck

#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/Utils/JTypeInfo.h>
#include <TString.h>
#include <string>
#include <variant>
#include <vector>

#include "algorithms/calorimetry/CalorimeterHitDigiConfig.h"
#include "algorithms/calorimetry/ImagingTopoClusterConfig.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/calorimetry/CalorimeterClusterRecoCoG_factory.h"
#include "factories/calorimetry/CalorimeterClusterShape_factory.h"
#include "factories/calorimetry/CalorimeterHitDigi_factory.h"
#include "factories/calorimetry/CalorimeterHitReco_factory.h"
#include "factories/calorimetry/CalorimeterHitsMerger_factory.h"
#include "factories/calorimetry/CalorimeterIslandCluster_factory.h"
#include "factories/calorimetry/CalorimeterTruthClustering_factory.h"
#include "factories/calorimetry/HEXPLIT_factory.h"
#include "factories/calorimetry/ImagingTopoCluster_factory.h"
#include "factories/calorimetry/TrackClusterMergeSplitter_factory.h"

extern "C" {
void InitPlugin(JApplication* app) {

  using namespace eicrecon;

  InitJANAPlugin(app);

  // Make sure digi and reco use the same value
  decltype(CalorimeterHitDigiConfig::capADC) HcalEndcapPInsert_capADC           = 32768;
  decltype(CalorimeterHitDigiConfig::dyRangeADC) HcalEndcapPInsert_dyRangeADC   = 200 * dd4hep::MeV;
  decltype(CalorimeterHitDigiConfig::pedMeanADC) HcalEndcapPInsert_pedMeanADC   = 10;
  decltype(CalorimeterHitDigiConfig::pedSigmaADC) HcalEndcapPInsert_pedSigmaADC = 2;
  decltype(CalorimeterHitDigiConfig::resolutionTDC) HcalEndcapPInsert_resolutionTDC =
      10 * dd4hep::picosecond;

  app->Add(new JOmniFactoryGeneratorT<CalorimeterHitDigi_factory>(
      "HcalEndcapPInsertRawHits", {"EventHeader", "HcalEndcapPInsertHits"},
      {"HcalEndcapPInsertRawHits", "HcalEndcapPInsertRawHitAssociations"},
      {
          .eRes          = {},
          .tRes          = 0.0 * dd4hep::ns,
          .capADC        = HcalEndcapPInsert_capADC,
          .dyRangeADC    = HcalEndcapPInsert_dyRangeADC,
          .pedMeanADC    = HcalEndcapPInsert_pedMeanADC,
          .pedSigmaADC   = HcalEndcapPInsert_pedSigmaADC,
          .resolutionTDC = HcalEndcapPInsert_resolutionTDC,
          .corrMeanScale = "1.0",
          .readout       = "HcalEndcapPInsertHits",
      },
      app // TODO: Remove me once fixed
      ));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterHitReco_factory>(
      "HcalEndcapPInsertRecHits", {"HcalEndcapPInsertRawHits"}, {"HcalEndcapPInsertRecHits"},
      {
          .capADC          = HcalEndcapPInsert_capADC,
          .dyRangeADC      = HcalEndcapPInsert_dyRangeADC,
          .pedMeanADC      = HcalEndcapPInsert_pedMeanADC,
          .pedSigmaADC     = HcalEndcapPInsert_pedSigmaADC,
          .resolutionTDC   = HcalEndcapPInsert_resolutionTDC,
          .thresholdFactor = 0.,
          .thresholdValue  = 41.0, // 0.25 MeV --> 0.25 / 200 * 32768 = 41

          .sampFrac   = "1.0",
          .readout    = "HcalEndcapPInsertHits",
          .layerField = "layer",
      },
      app // TODO: Remove me once fixed
      ));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterHitsMerger_factory>(
      "HcalEndcapPInsertMergedHits", {"HcalEndcapPInsertRecHits"}, {"HcalEndcapPInsertMergedHits"},
      {.readout = "HcalEndcapPInsertHits", .fieldTransformations = {"layer:1", "slice:0"}},
      app // TODO: Remove me once fixed
      ));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterTruthClustering_factory>(
      "HcalEndcapPInsertTruthProtoClusters",
      {"HcalEndcapPInsertMergedHits", "HcalEndcapPInsertHits"},
      {"HcalEndcapPInsertTruthProtoClusters"},
      app // TODO: Remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<HEXPLIT_factory>(
      "HcalEndcapPInsertSubcellHits", {"HcalEndcapPInsertRecHits"},
      {"HcalEndcapPInsertSubcellHits"},
      {
          .MIP          = 480. * dd4hep::keV,
          .Emin_in_MIPs = 0.5,
          .tmax         = 162 * dd4hep::ns, //150 ns + (z at front face)/(speed of light)
      },
      app // TODO: Remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<ImagingTopoCluster_factory>(
      "HcalEndcapPInsertImagingProtoClusters", {"HcalEndcapPInsertSubcellHits"},
      {"HcalEndcapPInsertImagingProtoClusters"},
      {
          .neighbourLayersRange = 1,
          .sameLayerDistXY =
              {"0.5 * max(HcalEndcapPInsertCellSizeLGRight, HcalEndcapPInsertCellSizeLGLeft)",
               "0.5 * max(HcalEndcapPInsertCellSizeLGRight, HcalEndcapPInsertCellSizeLGLeft) * "
               "sin(pi / 3)"},
          .diffLayerDistXY =
              {"0.25 * max(HcalEndcapPInsertCellSizeLGRight, HcalEndcapPInsertCellSizeLGLeft)",
               "0.25 * max(HcalEndcapPInsertCellSizeLGRight, HcalEndcapPInsertCellSizeLGLeft) * "
               "sin(pi / 3)"},

          .sameLayerMode        = eicrecon::ImagingTopoClusterConfig::ELayerMode::xy,
          .sectorDist           = 10.0 * dd4hep::cm,
          .minClusterHitEdep    = 5.0 * dd4hep::keV,
          .minClusterCenterEdep = 3.0 * dd4hep::MeV,
          .minClusterEdep       = 11.0 * dd4hep::MeV,
          .minClusterNhits      = 100,
      },
      app // TODO: Remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
      "HcalEndcapPInsertTruthClustersWithoutShapes",
      {
          "HcalEndcapPInsertTruthProtoClusters", // edm4eic::ProtoClusterCollection
          "HcalEndcapPInsertRawHitAssociations" // edm4eic::MCRecoCalorimeterHitAssociationCollection
      },
      {"HcalEndcapPInsertTruthClustersWithoutShapes",             // edm4eic::Cluster
       "HcalEndcapPInsertTruthClusterAssociationsWithoutShapes"}, // edm4eic::MCRecoClusterParticleAssociation
      {.energyWeight = "log", .sampFrac = 0.0257, .logWeightBase = 3.6, .enableEtaBounds = true},
      app // TODO: Remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>(
      "HcalEndcapPInsertTruthClusters",
      {"HcalEndcapPInsertTruthClustersWithoutShapes",
       "HcalEndcapPInsertTruthClusterAssociationsWithoutShapes"},
      {"HcalEndcapPInsertTruthClusters", "HcalEndcapPInsertTruthClusterAssociations"},
      {.longitudinalShowerInfoAvailable = true, .energyWeight = "log", .logWeightBase = 3.6}, app));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
      "HcalEndcapPInsertClustersWithoutShapes",
      {
          "HcalEndcapPInsertImagingProtoClusters", // edm4eic::ProtoClusterCollection
          "HcalEndcapPInsertRawHitAssociations" // edm4eic::MCRecoCalorimeterHitAssociationCollection
      },
      {"HcalEndcapPInsertClustersWithoutShapes",             // edm4eic::Cluster
       "HcalEndcapPInsertClusterAssociationsWithoutShapes"}, // edm4eic::MCRecoClusterParticleAssociation
      {
          .energyWeight    = "log",
          .sampFrac        = 0.0257,
          .logWeightBase   = 6.2,
          .enableEtaBounds = false,
      },
      app // TODO: Remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>(
      "HcalEndcapPInsertClusters",
      {"HcalEndcapPInsertClustersWithoutShapes",
       "HcalEndcapPInsertClusterAssociationsWithoutShapes"},
      {"HcalEndcapPInsertClusters", "HcalEndcapPInsertClusterAssociations"},
      {.longitudinalShowerInfoAvailable = true,
       .energyWeight                    = "log",
       .sampFrac                        = 0.0257,
       .logWeightBase                   = 6.2},
      app));

  // Make sure digi and reco use the same value
  decltype(CalorimeterHitDigiConfig::capADC) LFHCAL_capADC               = 65536;
  decltype(CalorimeterHitDigiConfig::dyRangeADC) LFHCAL_dyRangeADC       = 1 * dd4hep::GeV;
  decltype(CalorimeterHitDigiConfig::pedMeanADC) LFHCAL_pedMeanADC       = 50;
  decltype(CalorimeterHitDigiConfig::pedSigmaADC) LFHCAL_pedSigmaADC     = 10;
  decltype(CalorimeterHitDigiConfig::resolutionTDC) LFHCAL_resolutionTDC = 10 * dd4hep::picosecond;

  app->Add(new JOmniFactoryGeneratorT<CalorimeterHitDigi_factory>(
      "LFHCALRawHits", {"LFHCALHits"}, {"LFHCALRawHits", "LFHCALRawHitAssociations"},
      "LFHCALRawHits", {"EventHeader", "LFHCALHits"}, {"LFHCALRawHits", "LFHCALRawHitAssociations"},
      {
          .eRes          = {},
          .tRes          = 0.0 * dd4hep::ns,
          .capADC        = LFHCAL_capADC,
          .capTime       = 100,
          .dyRangeADC    = LFHCAL_dyRangeADC,
          .pedMeanADC    = LFHCAL_pedMeanADC,
          .pedSigmaADC   = LFHCAL_pedSigmaADC,
          .resolutionTDC = LFHCAL_resolutionTDC,
          .corrMeanScale = "1.0",
          .readout       = "LFHCALHits",
          .fields        = {"layerz"},
      },
      app // TODO: Remove me once fixed
      ));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterHitReco_factory>(
      "LFHCALRecHits", {"LFHCALRawHits"}, {"LFHCALRecHits"},
      {
          .capADC          = LFHCAL_capADC,
          .dyRangeADC      = LFHCAL_dyRangeADC,
          .pedMeanADC      = LFHCAL_pedMeanADC,
          .pedSigmaADC     = LFHCAL_pedSigmaADC,
          .resolutionTDC   = LFHCAL_resolutionTDC,
          .thresholdFactor = 0.0,
          .thresholdValue  = 20, // 0.3 MeV deposition --> adc = 50 + 0.3 / 1000 * 65536 == 70
          .sampFrac        = "(rlayerz == 0) ? 0.019 : 0.037", // 0.019 only in the 0-th tile
          .readout         = "LFHCALHits",
          .layerField      = "rlayerz",
      },
      app // TODO: Remove me once fixed
      ));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterTruthClustering_factory>(
      "LFHCALTruthProtoClusters", {"LFHCALRecHits", "LFHCALHits"}, {"LFHCALTruthProtoClusters"},
      app // TODO: Remove me once fixed
      ));

  // Magic constants:
  //  54 - number of modules in a row/column
  //  2  - number of towers in a module
  // sign for towerx and towery are *negative* to maintain linearity with global X and Y
  std::string cellIdx_1 = "(54*2-moduleIDx_1*2-towerx_1)";
  std::string cellIdx_2 = "(54*2-moduleIDx_2*2-towerx_2)";
  std::string cellIdy_1 = "(54*2-moduleIDy_1*2-towery_1)";
  std::string cellIdy_2 = "(54*2-moduleIDy_2*2-towery_2)";
  std::string cellIdz_1 = "rlayerz_1";
  std::string cellIdz_2 = "rlayerz_2";
  std::string deltaX    = Form("abs(%s-%s)", cellIdx_2.data(), cellIdx_1.data());
  std::string deltaY    = Form("abs(%s-%s)", cellIdy_2.data(), cellIdy_1.data());
  std::string deltaZ    = Form("abs(%s-%s)", cellIdz_2.data(), cellIdz_1.data());
  std::string neighbor  = Form("(%s+%s+%s==1)", deltaX.data(), deltaY.data(), deltaZ.data());
  std::string corner2D =
      Form("((%s==0&&%s==1&&%s==1)||(%s==1&&%s==0&&%s==1)||(%s==1&&%s==1&&%s==0))", deltaZ.data(),
           deltaX.data(), deltaY.data(), deltaZ.data(), deltaX.data(), deltaY.data(), deltaZ.data(),
           deltaX.data(), deltaY.data());

  app->Add(new JOmniFactoryGeneratorT<CalorimeterIslandCluster_factory>(
      "LFHCALIslandProtoClusters", {"LFHCALRecHits"}, {"LFHCALIslandProtoClusters"},
      {
          .adjacencyMatrix = Form("%s||%s", neighbor.data(), corner2D.data()),
          .peakNeighbourhoodMatrix{},
          .readout    = "LFHCALHits",
          .sectorDist = 0 * dd4hep::cm,
          .localDistXY{},
          .localDistXZ{},
          .localDistYZ{},
          .globalDistRPhi{},
          .globalDistEtaPhi{},
          .dimScaledLocalDistXY{},
          .splitCluster                  = false,
          .minClusterHitEdep             = 1 * dd4hep::MeV,
          .minClusterCenterEdep          = 100.0 * dd4hep::MeV,
          .transverseEnergyProfileMetric = "globalDistEtaPhi",
          .transverseEnergyProfileScale  = 1.,
          .transverseEnergyProfileScaleUnits{},
      },
      app // TODO: Remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
      "LFHCALTruthClustersWithoutShapes",
      {
          "LFHCALTruthProtoClusters", // edm4eic::ProtoClusterCollection
          "LFHCALRawHitAssociations"  // edm4eic::MCRecoCalorimeterHitAssociationCollection
      },
      {"LFHCALTruthClustersWithoutShapes",             // edm4eic::Cluster
       "LFHCALTruthClusterAssociationsWithoutShapes"}, // edm4eic::MCRecoClusterParticleAssociation
      {.energyWeight = "log", .sampFrac = 1.0, .logWeightBase = 4.5, .enableEtaBounds = false},
      app // TODO: Remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>(
      "LFHCALTruthClusters",
      {"LFHCALTruthClustersWithoutShapes", "LFHCALTruthClusterAssociationsWithoutShapes"},
      {"LFHCALTruthClusters", "LFHCALTruthClusterAssociations"},
      {.longitudinalShowerInfoAvailable = true, .energyWeight = "log", .logWeightBase = 4.5}, app));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
      "LFHCALClustersWithoutShapes",
      {
          "LFHCALIslandProtoClusters", // edm4eic::ProtoClusterCollection
          "LFHCALRawHitAssociations"   // edm4eic::MCRecoCalorimeterHitAssociationCollection
      },
      {"LFHCALClustersWithoutShapes",             // edm4eic::Cluster
       "LFHCALClusterAssociationsWithoutShapes"}, // edm4eic::MCRecoClusterParticleAssociation
      {
          .energyWeight    = "log",
          .sampFrac        = 1.0,
          .logWeightBase   = 4.5,
          .enableEtaBounds = false,
      },
      app // TODO: Remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>(
      "LFHCALClusters", {"LFHCALClustersWithoutShapes", "LFHCALClusterAssociationsWithoutShapes"},
      {"LFHCALClusters", "LFHCALClusterAssociations"},
      {.longitudinalShowerInfoAvailable = true, .energyWeight = "log", .logWeightBase = 4.5}, app));

  app->Add(new JOmniFactoryGeneratorT<TrackClusterMergeSplitter_factory>(
      "LFHCALSplitMergeProtoClusters",
      {"LFHCALTrackClusterMatches", "LFHCALClusters", "CalorimeterTrackProjections"},
      {"LFHCALSplitMergeProtoClusters",
#if EDM4EIC_VERSION_MAJOR >= 8 && EDM4EIC_VERSION_MINOR >= 4
       "LFHCALTrackSplitMergeProtoClusterMatches"},
#endif
      {.minSigCut                    = -2.0,
       .avgEP                        = 0.50,
       .sigEP                        = 0.25,
       .drAdd                        = 0.30,
       .surfaceToUse                 = 1,
       .transverseEnergyProfileScale = 1.0},
      app // TODO: remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
      "LFHCALSplitMergeClustersWithoutShapes",
      {"LFHCALSplitMergeProtoClusters", "LFHCALRawHitAssociations"},
      {"LFHCALSplitMergeClustersWithoutShapes", "LFHCALSplitMergeClusterAssociationsWithoutShapes"},
      {
          .energyWeight    = "log",
          .sampFrac        = 1.0,
          .logWeightBase   = 4.5,
          .enableEtaBounds = false,
      },
      app // TODO: Remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>(
      "LFHCALSplitMergeClusters",
      {"LFHCALSplitMergeClustersWithoutShapes", "LFHCALSplitMergeClusterAssociationsWithoutShapes"},
      {"LFHCALSplitMergeClusters", "LFHCALSplitMergeClusterAssociations"},
      {.longitudinalShowerInfoAvailable = true, .energyWeight = "log", .logWeightBase = 4.5}, app));
}
}
