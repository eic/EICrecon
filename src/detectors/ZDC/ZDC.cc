// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2021 - 2025, Chao Peng, Sylvester Joosten, Whitney Armstrong, David Lawrence, Friederike Bock, Wouter Deconinck, Nathan Brei, Sebouh Paul, Dmitry Kalinkin, Barak Schmookler

#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplicationFwd.h>
#include <edm4eic/EDM4eicVersion.h>
#include <JANA/Utils/JTypeInfo.h>
#include <string>
#include <variant>
#include <vector>

#include "algorithms/calorimetry/ImagingTopoClusterConfig.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/calorimetry/CalorimeterClusterRecoCoG_factory.h"
#include "factories/calorimetry/CalorimeterClusterShape_factory.h"
#include "factories/calorimetry/CalorimeterHitDigi_factory.h"
#include "factories/calorimetry/CalorimeterHitReco_factory.h"
#include "factories/calorimetry/CalorimeterIslandCluster_factory.h"
#include "factories/calorimetry/CalorimeterTruthClustering_factory.h"
#include "factories/calorimetry/HEXPLIT_factory.h"
#include "factories/calorimetry/ImagingTopoCluster_factory.h"

extern "C" {
void InitPlugin(JApplication* app) {

  using namespace eicrecon;

  InitJANAPlugin(app);

  // LYSO part of the ZDC
  app->Add(new JOmniFactoryGeneratorT<CalorimeterHitDigi_factory>(
      "EcalFarForwardZDCRawHits", {"EventHeader", "EcalFarForwardZDCHits"},
      {"EcalFarForwardZDCRawHits",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
       "EcalFarForwardZDCRawHitLinks",
#endif
       "EcalFarForwardZDCRawHitAssociations"},
      {
          .eRes{},
          .tRes          = 0.0 * dd4hep::ns,
          .capADC        = 32768,
          .dyRangeADC    = 2000 * dd4hep::MeV,
          .pedMeanADC    = 400,
          .pedSigmaADC   = 3.2,
          .resolutionTDC = 10 * dd4hep::picosecond,
          .corrMeanScale = "1.0",
          .readout       = "EcalFarForwardZDCHits",
      },
      app // TODO: Remove me once fixed
      ));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterHitReco_factory>(
      "EcalFarForwardZDCRecHits", {"EcalFarForwardZDCRawHits"}, {"EcalFarForwardZDCRecHits"},
      {
          .capADC          = 32768,
          .dyRangeADC      = 2000. * dd4hep::MeV,
          .pedMeanADC      = 400,
          .pedSigmaADC     = 3.2,
          .resolutionTDC   = 10 * dd4hep::picosecond,
          .thresholdFactor = 4.0,
          .thresholdValue  = 0.0,
          .sampFrac        = "1.0",
          .readout         = "EcalFarForwardZDCHits",
      },
      app // TODO: Remove me once fixed
      ));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterTruthClustering_factory>(
      "EcalFarForwardZDCTruthProtoClusters", {"EcalFarForwardZDCRecHits", "EcalFarForwardZDCRawHitAssociations"},
      {"EcalFarForwardZDCTruthProtoClusters"},
      app // TODO: Remove me once fixed
      ));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterIslandCluster_factory>(
      "EcalFarForwardZDCIslandProtoClusters", {"EcalFarForwardZDCRecHits"},
      {"EcalFarForwardZDCIslandProtoClusters"},
      {
          .adjacencyMatrix{},
          .peakNeighbourhoodMatrix{},
          .readout{},
          .sectorDist  = 5.0 * dd4hep::cm,
          .localDistXY = {50 * dd4hep::cm, 50 * dd4hep::cm},
          .localDistXZ{},
          .localDistYZ{},
          .globalDistRPhi{},
          .globalDistEtaPhi{},
          .dimScaledLocalDistXY{},
          .splitCluster                  = true,
          .minClusterHitEdep             = 0.1 * dd4hep::MeV,
          .minClusterCenterEdep          = 3.0 * dd4hep::MeV,
          .transverseEnergyProfileMetric = "globalDistEtaPhi",
          .transverseEnergyProfileScale  = 1.,
          .transverseEnergyProfileScaleUnits{},
      },
      app // TODO: Remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
      "EcalFarForwardZDCTruthClustersWithoutShapes",
      {
          "EcalFarForwardZDCTruthProtoClusters", // edm4eic::ProtoClusterCollection
          "EcalFarForwardZDCRawHitAssociations"  // edm4eic::MCRecoClusterHitAssociationCollection
      },
      {"EcalFarForwardZDCTruthClustersWithoutShapes",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
       "EcalFarForwardZDCTruthClusterLinksWithoutShapes",
#endif
       "EcalFarForwardZDCTruthClusterAssociationsWithoutShapes"}, // edm4eic::MCRecoClusterParticleAssociation
      {.energyWeight = "log", .sampFrac = 1.0, .logWeightBase = 3.6, .enableEtaBounds = false},
      app // TODO: Remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>(
      "EcalFarForwardZDCTruthClusters",
      {"EcalFarForwardZDCTruthClustersWithoutShapes",
       "EcalFarForwardZDCTruthClusterAssociationsWithoutShapes"},
      {"EcalFarForwardZDCTruthClusters",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
       "EcalFarForwardZDCTruthClusterLinks",
#endif
       "EcalFarForwardZDCTruthClusterAssociations"},
      {.longitudinalShowerInfoAvailable = true, .energyWeight = "log", .logWeightBase = 3.6}, app));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
      "EcalFarForwardZDCClustersWithoutShapes",
      {
          "EcalFarForwardZDCIslandProtoClusters", // edm4eic::ProtoClusterCollection
          "EcalFarForwardZDCRawHitAssociations"   // edm4eic::MCRecoClusterHitAssociationCollection
      },
      {"EcalFarForwardZDCClustersWithoutShapes",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
       "EcalFarForwardZDCClusterLinksWithoutShapes",
#endif
       "EcalFarForwardZDCClusterAssociationsWithoutShapes"}, // edm4eic::MCRecoClusterParticleAssociation
      {
          .energyWeight    = "log",
          .sampFrac        = 1.0,
          .logWeightBase   = 6.2,
          .enableEtaBounds = false,
      },
      app // TODO: Remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>(
      "EcalFarForwardZDCClusters",
      {"EcalFarForwardZDCClustersWithoutShapes",
       "EcalFarForwardZDCClusterAssociationsWithoutShapes"},
      {"EcalFarForwardZDCClusters",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
       "EcalFarForwardZDCClusterLinks",
#endif
       "EcalFarForwardZDCClusterAssociations"},
      {.longitudinalShowerInfoAvailable = true, .energyWeight = "log", .logWeightBase = 6.2}, app));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterHitDigi_factory>(
      "HcalFarForwardZDCRawHits", {"EventHeader", "HcalFarForwardZDCHits"},
      {"HcalFarForwardZDCRawHits",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
       "HcalFarForwardZDCRawHitLinks",
#endif
       "HcalFarForwardZDCRawHitAssociations"},
      {
          .eRes{},
          .tRes          = 0.0 * dd4hep::ns,
          .capADC        = 65536,
          .dyRangeADC    = 1000. * dd4hep::MeV,
          .pedMeanADC    = 400,
          .pedSigmaADC   = 2,
          .resolutionTDC = 10 * dd4hep::picosecond,
          .corrMeanScale = "1.0",
          .readout       = "HcalFarForwardZDCHits",
      },
      app // TODO: Remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterHitReco_factory>(
      "HcalFarForwardZDCRecHits", {"HcalFarForwardZDCRawHits"}, {"HcalFarForwardZDCRecHits"},
      {
          .capADC          = 65536,
          .dyRangeADC      = 1000. * dd4hep::MeV,
          .pedMeanADC      = 400,
          .pedSigmaADC     = 2,
          .resolutionTDC   = 10 * dd4hep::picosecond,
          .thresholdFactor = 3.0,
          .thresholdValue  = 0.0,
          .sampFrac        = "1.0",
          .readout         = "HcalFarForwardZDCHits",
          .layerField      = "layer",
          .sectorField     = "system",
      },
      app // TODO: Remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<HEXPLIT_factory>("HcalFarForwardZDCSubcellHits",
                                                       {"HcalFarForwardZDCRecHits"},
                                                       {"HcalFarForwardZDCSubcellHits"},
                                                       {
                                                           .MIP           = 472. * dd4hep::keV,
                                                           .Emin_in_MIPs  = 0.5,
                                                           .delta_in_MIPs = 0.01,
                                                           .tmax          = 269 * dd4hep::ns,
                                                       },
                                                       app // TODO: Remove me once fixed
                                                       ));

  app->Add(new JOmniFactoryGeneratorT<ImagingTopoCluster_factory>(
      "HcalFarForwardZDCImagingProtoClusters", {"HcalFarForwardZDCSubcellHits"},
      {"HcalFarForwardZDCImagingProtoClusters"},
      {
          .neighbourLayersRange = 1,
          .sameLayerDistXY      = {"0.5 * HcalFarForwardZDC_SiPMonTile_HexSideLength",
                                   "0.5 * HcalFarForwardZDC_SiPMonTile_HexSideLength * sin(pi / 3)"},
          .diffLayerDistXY      = {"0.5 * HcalFarForwardZDC_SiPMonTile_HexSideLength",
                                   "0.5 * HcalFarForwardZDC_SiPMonTile_HexSideLength * sin(pi / 3)"},
          .sameLayerMode        = eicrecon::ImagingTopoClusterConfig::ELayerMode::xy,
          .sectorDist           = 10.0 * dd4hep::cm,
          .minClusterHitEdep    = 50.0 * dd4hep::keV,
          .minClusterCenterEdep = 3.0 * dd4hep::MeV,
          .minClusterEdep       = 11.0 * dd4hep::MeV,
          .minClusterNhits      = 30,
      },
      app // TODO: Remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterIslandCluster_factory>(
      "HcalFarForwardZDCIslandProtoClusters", {"HcalFarForwardZDCSubcellHits"},
      {"HcalFarForwardZDCIslandProtoClusters"},
      {.adjacencyMatrix{},
       .peakNeighbourhoodMatrix{},
       .readout{},
       .sectorDist  = 1.5 * dd4hep::cm,
       .localDistXY = {"0.9 * HcalFarForwardZDC_SiPMonTile_HexSideLength",
                       "0.76 * HcalFarForwardZDC_SiPMonTile_HexSideLength * sin(pi / 3)"},
       .localDistXZ{},
       .localDistYZ{},
       .globalDistRPhi{},
       .globalDistEtaPhi{},
       .dimScaledLocalDistXY{},
       .splitCluster         = false,
       .minClusterHitEdep    = 100.0 * dd4hep::keV,
       .minClusterCenterEdep = 1.0 * dd4hep::MeV,
       .transverseEnergyProfileMetric{}, // = "globalDistEtaPhi",
       .transverseEnergyProfileScale{},  // = 1.,
       .transverseEnergyProfileScaleUnits{}},
      app));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
      "HcalFarForwardZDCClustersWithoutShapes",
      {
          "HcalFarForwardZDCImagingProtoClusters", // edm4eic::ProtoClusterCollection
          "HcalFarForwardZDCRawHitAssociations" // edm4eic::MCRecoCalorimeterHitAssociationCollection
      },
      {"HcalFarForwardZDCClustersWithoutShapes",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
       "HcalFarForwardZDCClusterLinksWithoutShapes",
#endif
       "HcalFarForwardZDCClusterAssociationsWithoutShapes"}, // edm4eic::MCRecoClusterParticleAssociation
      {.energyWeight        = "log",
       .sampFrac            = 0.0203,
       .logWeightBaseCoeffs = {5.8, 0.65, 0.31},
       .logWeightBase_Eref  = 50 * dd4hep::GeV},
      app // TODO: Remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>(
      "HcalFarForwardZDCClusters",
      {"HcalFarForwardZDCClustersWithoutShapes",
       "HcalFarForwardZDCClusterAssociationsWithoutShapes"},
      {"HcalFarForwardZDCClusters",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
       "HcalFarForwardZDCClusterLinks",
#endif
       "HcalFarForwardZDCClusterAssociations"},
      {.longitudinalShowerInfoAvailable = true,
       .energyWeight                    = "log",
       .sampFrac                        = 0.0203,
       .logWeightBaseCoeffs             = {5.8, 0.65, 0.31},
       .logWeightBase_Eref              = 50 * dd4hep::GeV},
      app));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterTruthClustering_factory>(
      "HcalFarForwardZDCTruthProtoClusters", {"HcalFarForwardZDCRecHits", "HcalFarForwardZDCRawHitAssociations"},
      {"HcalFarForwardZDCTruthProtoClusters"},
      app // TODO: Remove me once fixed
      ));

  //Clusters with the baseline algorithm (no HEXPLIT)
  app->Add(new JOmniFactoryGeneratorT<CalorimeterIslandCluster_factory>(
      "HcalFarForwardZDCIslandProtoClustersBaseline", {"HcalFarForwardZDCRecHits"},
      {"HcalFarForwardZDCIslandProtoClustersBaseline"},
      {.adjacencyMatrix{},
       .peakNeighbourhoodMatrix{},
       .readout{},
       .sectorDist  = 5.0 * dd4hep::cm,
       .localDistXY = {50 * dd4hep::cm, 50 * dd4hep::cm},
       .localDistXZ{},
       .localDistYZ{},
       .globalDistRPhi{},
       .globalDistEtaPhi{},
       .dimScaledLocalDistXY{},
       .splitCluster                  = true,
       .minClusterHitEdep             = 0.1 * dd4hep::MeV,
       .minClusterCenterEdep          = 3.0 * dd4hep::MeV,
       .transverseEnergyProfileMetric = "globalDistEtaPhi",
       .transverseEnergyProfileScale  = 1.,
       .transverseEnergyProfileScaleUnits{}},
      app // TODO: Remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
      "HcalFarForwardZDCTruthClustersWithoutShapes",
      {
          "HcalFarForwardZDCTruthProtoClusters", // edm4eic::ProtoClusterCollection
          "HcalFarForwardZDCRawHitAssociations" // edm4eic::MCRecoCalorimeterHitAssociationCollection
      },
      {"HcalFarForwardZDCTruthClustersWithoutShapes",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
       "HcalFarForwardZDCTruthClusterLinksWithoutShapes",
#endif
       "HcalFarForwardZDCTruthClusterAssociationsWithoutShapes"}, // edm4eic::MCRecoClusterParticleAssociation
      {.energyWeight = "log", .sampFrac = 1.0, .logWeightBase = 3.6, .enableEtaBounds = false},
      app // TODO: Remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>(
      "HcalFarForwardZDCTruthClusters",
      {"HcalFarForwardZDCTruthClustersWithoutShapes",
       "HcalFarForwardZDCTruthClusterAssociationsWithoutShapes"},
      {"HcalFarForwardZDCTruthClusters",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
       "HcalFarForwardZDCTruthClusterLinks",
#endif
       "HcalFarForwardZDCTruthClusterAssociations"},
      {.longitudinalShowerInfoAvailable = true, .energyWeight = "log", .logWeightBase = 3.6}, app));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
      "HcalFarForwardZDCClustersBaselineWithoutShapes",
      {
          "HcalFarForwardZDCIslandProtoClustersBaseline", // edm4eic::ProtoClusterCollection
          "HcalFarForwardZDCRawHitAssociations" // edm4eic::MCRecoCalorimeterHitAssociationCollection
      },
      {"HcalFarForwardZDCClustersBaselineWithoutShapes", // edm4eic::Cluster
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
       "HcalFarForwardZDCClusterLinksBaselineWithoutShapes",
#endif
       "HcalFarForwardZDCClusterAssociationsBaselineWithoutShapes"}, // edm4eic::MCRecoClusterParticleAssociation
      {
          .energyWeight    = "log",
          .sampFrac        = 0.0203,
          .logWeightBase   = 6.2,
          .enableEtaBounds = false,
      },
      app // TODO: Remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>(
      "HcalFarForwardZDCClustersBaseline",
      {"HcalFarForwardZDCClustersBaselineWithoutShapes",
       "HcalFarForwardZDCClusterAssociationsBaselineWithoutShapes"},
      {"HcalFarForwardZDCClustersBaseline",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
       "HcalFarForwardZDCClusterLinksBaseline",
#endif
       "HcalFarForwardZDCClusterAssociationsBaseline"},
      {.longitudinalShowerInfoAvailable = true,
       .energyWeight                    = "log",
       .sampFrac                        = 0.0203,
       .logWeightBase                   = 6.2},
      app));
}
}
