// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 - 2026 Friederike Bock, Wouter Deconinck, Aiden Wu

#include <DD4hep/Detector.h>
#include <DD4hep/Readout.h>
#include <DD4hep/Segmentations.h>
#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplication.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/Utils/JTypeInfo.h>
#include <TString.h>
#include <edm4eic/unit_system.h>
#include <gsl/pointers>
#include <memory>
#include <string>
#include <variant>
#include <vector>

#include "algorithms/calorimetry/CalorimeterHitDigiConfig.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/calorimetry/CalorimeterClusterRecoCoG_factory.h"
#include "factories/calorimetry/CalorimeterClusterShape_factory.h"
#include "factories/calorimetry/CalorimeterHitDigi_factory.h"
#include "factories/calorimetry/CalorimeterHitReco_factory.h"
#include "factories/calorimetry/CalorimeterMultiReadoutIslandCluster_factory.h"
#include "factories/calorimetry/CalorimeterTruthClustering_factory.h"
#include "factories/calorimetry/HEXPLIT_factory.h"
#include "factories/meta/CollectionCollector_factory.h"
#include "services/geometry/dd4hep/DD4hep_service.h"

extern "C" {
void InitPlugin(JApplication* app) {

  using namespace eicrecon;

  InitJANAPlugin(app);

  // Select the insert clustering path from the loaded geometry's readout segmentation.
  bool insertUsesPhysicalTiles = false;
  try {
    auto detector                 = app->GetService<DD4hep_service>()->detector();
    const auto insertSegmentation = detector->readout("HcalEndcapPInsertHits").segmentation();
    insertUsesPhysicalTiles       = insertSegmentation.type() == "NoSegmentation";
  } catch (...) {
    // Preserve legacy reconstruction when the insert readout is unavailable.
  }

  // Make sure digi and reco use the same value
  decltype(CalorimeterHitDigiConfig::capADC) HcalEndcapPInsert_capADC           = 32768;
  decltype(CalorimeterHitDigiConfig::dyRangeADC) HcalEndcapPInsert_dyRangeADC   = 200 * dd4hep::MeV;
  decltype(CalorimeterHitDigiConfig::pedMeanADC) HcalEndcapPInsert_pedMeanADC   = 10;
  decltype(CalorimeterHitDigiConfig::pedSigmaADC) HcalEndcapPInsert_pedSigmaADC = 2;
  decltype(CalorimeterHitDigiConfig::resolutionTDC) HcalEndcapPInsert_resolutionTDC =
      10 * dd4hep::picosecond;

  app->Add(new JOmniFactoryGeneratorT<CalorimeterHitDigi_factory>(
      "HcalEndcapPInsertRawHits", {"EventHeader", "HcalEndcapPInsertHits"},
      {"HcalEndcapPInsertRawHits", "HcalEndcapPInsertRawHitLinks",
       "HcalEndcapPInsertRawHitAssociations"},
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

          .sampFrac   = "0.0257",
          .readout    = "HcalEndcapPInsertHits",
          .layerField = "layer",
      },
      app // TODO: Remove me once fixed
      ));

  // Make sure digi and reco use the same value
  decltype(CalorimeterHitDigiConfig::capADC) LFHCAL_capADC               = 65536;
  decltype(CalorimeterHitDigiConfig::dyRangeADC) LFHCAL_dyRangeADC       = 1 * dd4hep::GeV;
  decltype(CalorimeterHitDigiConfig::pedMeanADC) LFHCAL_pedMeanADC       = 50;
  decltype(CalorimeterHitDigiConfig::pedSigmaADC) LFHCAL_pedSigmaADC     = 10;
  decltype(CalorimeterHitDigiConfig::resolutionTDC) LFHCAL_resolutionTDC = 10 * dd4hep::picosecond;

  app->Add(new JOmniFactoryGeneratorT<CalorimeterHitDigi_factory>(
      "LFHCALRawHits", {"EventHeader", "LFHCALHits"},
      {"LFHCALRawHits", "LFHCALRawHitLinks", "LFHCALRawHitAssociations"},
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

  // Also preserve the previous scheme
  std::string insertClusteringHits = "HcalEndcapPInsertRecHits";
  if (!insertUsesPhysicalTiles) {
    insertClusteringHits = "HcalEndcapPInsertSubcellHits";
    app->Add(new JOmniFactoryGeneratorT<HEXPLIT_factory>(
        "HcalEndcapPInsertSubcellHits", {"HcalEndcapPInsertRecHits"},
        {"HcalEndcapPInsertSubcellHits"},
        {
            .MIP          = 480. * dd4hep::keV / 0.0257,
            .Emin_in_MIPs = 0.5,
        },
        app));
  }

  // Combine calibrated hits and their truth links before clustering.
  app->Add(new JOmniFactoryGeneratorT<CollectionCollector_factory<edm4eic::CalorimeterHit>>(
      "FHCALRecHits", {"LFHCALRecHits", insertClusteringHits}, {"FHCALRecHits"}, app));

  app->Add(new JOmniFactoryGeneratorT<CollectionCollector_factory<edm4eic::MCRecoCalorimeterHitLink>>(
      "FHCALRawHitLinks", {"LFHCALRawHitLinks", "HcalEndcapPInsertRawHitLinks"},
      {"FHCALRawHitLinks"}, app));

  app->Add(new JOmniFactoryGeneratorT<
           CollectionCollector_factory<edm4eic::MCRecoCalorimeterHitAssociation>>(
      "FHCALRawHitAssociations", {"LFHCALRawHitAssociations", "HcalEndcapPInsertRawHitAssociations"},
      {"FHCALRawHitAssociations"}, app));

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

  CalorimeterIslandClusterConfig lfhcalIslandConfig{
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
      .minClusterHitEdep             = 1 * edm4eic::unit::MeV,
      .minClusterCenterEdep          = 100.0 * edm4eic::unit::MeV,
      .transverseEnergyProfileMetric = "globalDistEtaPhi",
      .transverseEnergyProfileScale  = 1.,
      .transverseEnergyProfileScaleUnits{},
  };

  CalorimeterIslandClusterConfig insertIslandConfig{
      .adjacencyMatrix{},
      .peakNeighbourhoodMatrix{},
      .readout    = "HcalEndcapPInsertHits",
      .sectorDist = 10.0 * dd4hep::cm,
      .localDistXY{},
      .localDistXZ{},
      .localDistYZ{},
      .globalDistRPhi{},
      .globalDistEtaPhi{},
      .dimScaledLocalDistXY = {1.5, 1.5},
      .splitCluster         = false,

      // Apply the deposited-energy cuts on sampling-corrected hit energies.
      .minClusterHitEdep    = 5.0 * edm4eic::unit::keV / 0.0257,
      .minClusterCenterEdep = 3.0 * edm4eic::unit::MeV / 0.0257,
      .transverseEnergyProfileMetric{},
      .transverseEnergyProfileScale{},
      .transverseEnergyProfileScaleUnits{},
  };

  app->Add(new JOmniFactoryGeneratorT<CalorimeterMultiReadoutIslandCluster_factory>(
      "FHCALIslandProtoClusters",
      {"FHCALRecHits", "LFHCALRecHits", insertClusteringHits},
      {"FHCALIslandProtoClusters"},
      {
          .readouts = {{.collectionIDs = {}, .island = lfhcalIslandConfig},
                       {.collectionIDs = {}, .island = insertIslandConfig}},
          .globalDist = 5.0 * edm4eic::unit::cm,
      },
      app));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterTruthClustering_factory>(
      "FHCALTruthProtoClusters", {"FHCALRecHits", "FHCALRawHitLinks"},
      {"FHCALTruthProtoClusters"}, app));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
      "FHCALTruthClustersWithoutShapes",
      {"FHCALTruthProtoClusters", "FHCALRawHitLinks", "FHCALRawHitAssociations"},
      {"FHCALTruthClustersWithoutShapes", "FHCALTruthClusterLinksWithoutShapes",
       "FHCALTruthClusterAssociationsWithoutShapes"},
      {.energyWeight = "log", .sampFrac = 1.0, .logWeightBase = 4.5, .enableEtaBounds = false},
      app));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>(
      "FHCALTruthClusters", {"FHCALTruthClustersWithoutShapes", "FHCALTruthClusterLinksWithoutShapes"},
      {"FHCALTruthClusters", "FHCALTruthClusterLinks", "FHCALTruthClusterAssociations"},
      {.longitudinalShowerInfoAvailable = true, .energyWeight = "log", .logWeightBase = 4.5}, app));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
      "FHCALClustersWithoutShapes",
      {"FHCALIslandProtoClusters", "FHCALRawHitLinks", "FHCALRawHitAssociations"},
      {"FHCALClustersWithoutShapes", "FHCALClusterLinksWithoutShapes",
       "FHCALClusterAssociationsWithoutShapes"},
      {.energyWeight = "log", .sampFrac = 1.0, .logWeightBase = 4.5, .enableEtaBounds = false},
      app));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>(
      "FHCALClusters", {"FHCALClustersWithoutShapes", "FHCALClusterLinksWithoutShapes"},
      {"FHCALClusters", "FHCALClusterLinks", "FHCALClusterAssociations"},
      {.longitudinalShowerInfoAvailable = true, .energyWeight = "log", .logWeightBase = 4.5}, app));
}
}
