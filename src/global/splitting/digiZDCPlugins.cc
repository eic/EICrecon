// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2021 - 2025, Chao Peng, Sylvester Joosten, Whitney Armstrong, David Lawrence, Friederike Bock, Wouter Deconinck, Nathan Brei, Sebouh Paul, Dmitry Kalinkin, Barak Schmookler

#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplicationFwd.h>
#include <edm4eic/EDM4eicVersion.h>
#include <JANA/Utils/JTypeInfo.h>
#include <string>
#include <variant>
#include <vector>

// #include "algorithms/calorimetry/ImagingTopoClusterConfig.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/calorimetry/CalorimeterClusterRecoCoG_factory.h"
#include "factories/calorimetry/CalorimeterClusterShape_factory.h"
#include "factories/calorimetry/CalorimeterHitDigi_factory.h"
#include "factories/calorimetry/CalorimeterHitReco_factory.h"
#include "factories/calorimetry/CalorimeterIslandCluster_factory.h"
#include "factories/calorimetry/CalorimeterTruthClustering_factory.h"
#include "factories/calorimetry/HEXPLIT_factory.h"
// #include "factories/calorimetry/ImagingTopoCluster_factory.h"

// extern "C" {
void InitPlugin_digiZDC(JApplication* app) {

  using namespace eicrecon;

  InitJANAPlugin(app);

  // LYSO part of the ZDC
  app->Add(new JOmniFactoryGeneratorT<CalorimeterHitDigi_factory>(
      JOmniFactoryGeneratorT<CalorimeterHitDigi_factory>::TypedWiring{
          .m_tag                 = "EcalFarForwardZDCRawHits_TK",
          .m_default_input_tags  = {"EventHeader", "EcalFarForwardZDCHits"},
          .m_default_output_tags = {"EcalFarForwardZDCRawHits_TK",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                                    "EcalFarForwardZDCRawHitLinks_TK",
#endif
                                    "EcalFarForwardZDCRawHitAssociations_TK"},
          .m_default_cfg =
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
          .level = JEventLevel::Timeslice},
      app));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterHitReco_factory>(
      JOmniFactoryGeneratorT<CalorimeterHitReco_factory>::TypedWiring{
          .m_tag                 = "EcalFarForwardZDCRecHits_TK",
          .m_default_input_tags  = {"EcalFarForwardZDCRawHits_TK"},
          .m_default_output_tags = {"EcalFarForwardZDCRecHits_TK"},
          .m_default_cfg =
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
          .level = JEventLevel::Timeslice},
      app));
      
//   app->Add(new JOmniFactoryGeneratorT<CalorimeterTruthClustering_factory>(
//       JOmniFactoryGeneratorT<CalorimeterTruthClustering_factory>::TypedWiring{
//           .m_tag                 = "EcalFarForwardZDCTruthProtoClusters_TK",
//           .m_default_input_tags  = {"EcalFarForwardZDCRecHits_TK", "EcalFarForwardZDCHits"},
//           .m_default_output_tags = {"EcalFarForwardZDCTruthProtoClusters_TK"},
//           .m_default_cfg         = {},
//           .level                 = JEventLevel::Timeslice},
//       app));
//   app->Add(new JOmniFactoryGeneratorT<CalorimeterIslandCluster_factory>(
//       JOmniFactoryGeneratorT<CalorimeterIslandCluster_factory>::TypedWiring{
//           .m_tag                 = "EcalFarForwardZDCIslandProtoClusters_TK",
//           .m_default_input_tags  = {"EcalFarForwardZDCRecHits_TK"},
//           .m_default_output_tags = {"EcalFarForwardZDCIslandProtoClusters_TK"},
//           .m_default_cfg =
//               {
//                   .adjacencyMatrix{},
//                   .peakNeighbourhoodMatrix{},
//                   .readout{},
//                   .sectorDist  = 5.0 * dd4hep::cm,
//                   .localDistXY = {50 * dd4hep::cm, 50 * dd4hep::cm},
//                   .localDistXZ{},
//                   .localDistYZ{},
//                   .globalDistRPhi{},
//                   .globalDistEtaPhi{},
//                   .dimScaledLocalDistXY{},
//                   .splitCluster                  = true,
//                   .minClusterHitEdep             = 0.1 * dd4hep::MeV,
//                   .minClusterCenterEdep          = 3.0 * dd4hep::MeV,
//                   .transverseEnergyProfileMetric = "globalDistEtaPhi",
//                   .transverseEnergyProfileScale  = 1.,
//                   .transverseEnergyProfileScaleUnits{},
//               },
//           .level = JEventLevel::Timeslice},
//       app));

//   app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
//       JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>::TypedWiring{
//           .m_tag = "EcalFarForwardZDCTruthClustersWithoutShapes_TK",
//           .m_default_input_tags =
//               {
//                   "EcalFarForwardZDCTruthProtoClusters_TK", // edm4eic::ProtoClusterCollection
// #if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
//                   "EcalFarForwardZDCRawHitLinks_TK", // edm4eic::MCRecoCalorimeterHitLink
// #endif
//                   "EcalFarForwardZDCRawHitAssociations_TK" // edm4eic::MCRecoCalorimeterHitAssociation
//               },
//           .m_default_output_tags = {"EcalFarForwardZDCTruthClustersWithoutShapes_TK",
// #if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
//                                     "EcalFarForwardZDCTruthClusterLinksWithoutShapes_TK",
// #endif
//                                     "EcalFarForwardZDCTruthClusterAssociationsWithoutShapes_TK"},
//           .m_default_cfg =
//               {
//                   .energyWeight    = "log",
//                   .sampFrac        = 1.0,
//                   .logWeightBase   = 3.6,
//                   .enableEtaBounds = false,
//               },
//           .level = JEventLevel::Timeslice},
//       app));

//   app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>(
//       JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>::TypedWiring{
//           .m_tag                 = "EcalFarForwardZDCTruthClusters_TK",
//           .m_default_input_tags  = {"EcalFarForwardZDCTruthClustersWithoutShapes_TK",
//                                     "EcalFarForwardZDCTruthClusterAssociationsWithoutShapes_TK"},
//           .m_default_output_tags = {"EcalFarForwardZDCTruthClusters_TK",
// #if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
//                                     "EcalFarForwardZDCTruthClusterLinks_TK",
// #endif
//                                     "EcalFarForwardZDCTruthClusterAssociations_TK"},
//           .m_default_cfg = {.longitudinalShowerInfoAvailable = true,
//                             .energyWeight                    = "log",
//                             .logWeightBase                   = 3.6},
//           .level         = JEventLevel::Timeslice},
//       app));

//   app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
//       JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>::TypedWiring{
//           .m_tag = "EcalFarForwardZDCClustersWithoutShapes_TK",
//           .m_default_input_tags =
//               {
//                   "EcalFarForwardZDCIslandProtoClusters_TK", // edm4eic::ProtoClusterCollection
// #if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
//                   "EcalFarForwardZDCRawHitLinks_TK", // edm4eic::MCRecoCalorimeterHitLink
// #endif
//                   "EcalFarForwardZDCRawHitAssociations_TK" // edm4eic::MCRecoCalorimeterHitAssociation
//               },
//           .m_default_output_tags = {"EcalFarForwardZDCClustersWithoutShapes_TK",
// #if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
//                                     "EcalFarForwardZDCClusterLinksWithoutShapes_TK",
// #endif
//                                     "EcalFarForwardZDCClusterAssociationsWithoutShapes_TK"},
//           .m_default_cfg =
//               {
//                   .energyWeight    = "log",
//                   .sampFrac        = 1.0,
//                   .logWeightBase   = 6.2,
//                   .enableEtaBounds = false,
//               },
//           .level = JEventLevel::Timeslice},
//       app));

//   app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>(
//       JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>::TypedWiring{
//           .m_tag                 = "EcalFarForwardZDCClusters_TK",
//           .m_default_input_tags  = {"EcalFarForwardZDCClustersWithoutShapes_TK",
//                                     "EcalFarForwardZDCClusterAssociationsWithoutShapes_TK"},
//           .m_default_output_tags = {"EcalFarForwardZDCClusters_TK",
// #if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
//                                     "EcalFarForwardZDCClusterLinks_TK",
// #endif
//                                     "EcalFarForwardZDCClusterAssociations_TK"},
//           .m_default_cfg = {.longitudinalShowerInfoAvailable = true,
//                             .energyWeight                    = "log",
//                             .logWeightBase                   = 6.2},
//           .level         = JEventLevel::Timeslice},
//       app));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterHitDigi_factory>(
      JOmniFactoryGeneratorT<CalorimeterHitDigi_factory>::TypedWiring{
          .m_tag                 = "HcalFarForwardZDCRawHits_TK",
          .m_default_input_tags  = {"EventHeader", "HcalFarForwardZDCHits"},
          .m_default_output_tags = {"HcalFarForwardZDCRawHits_TK",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                                    "HcalFarForwardZDCRawHitLinks_TK",
#endif
                                    "HcalFarForwardZDCRawHitAssociations_TK"},
          .m_default_cfg =
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
          .level = JEventLevel::Timeslice},
      app));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterHitReco_factory>(
      JOmniFactoryGeneratorT<CalorimeterHitReco_factory>::TypedWiring{
          .m_tag                 = "HcalFarForwardZDCRecHits_TK",
          .m_default_input_tags  = {"HcalFarForwardZDCRawHits_TK"},
          .m_default_output_tags = {"HcalFarForwardZDCRecHits_TK"},
          .m_default_cfg =
              {
                  .capADC          = 65536,
                  .dyRangeADC      = 1000. * dd4hep::MeV,
                  .pedMeanADC      = 400,
                  .pedSigmaADC     = 2,
                  .resolutionTDC   = 10 * dd4hep::picosecond,
                  .thresholdFactor = 3.0,
                  .thresholdValue  = 0.0,
                  .sampFrac        = "1.0",
                  .readout         = "HcalFarForwardZDCHits_TK",
                  .layerField      = "layer",
                  .sectorField     = "system",
              },
          .level = JEventLevel::Timeslice},
      app));

  app->Add(new JOmniFactoryGeneratorT<HEXPLIT_factory>(
      JOmniFactoryGeneratorT<HEXPLIT_factory>::TypedWiring{
          .m_tag                 = "HcalFarForwardZDCSubcellHits_TK",
          .m_default_input_tags  = {"HcalFarForwardZDCRecHits_TK"},
          .m_default_output_tags = {"HcalFarForwardZDCSubcellHits_TK"},
          .m_default_cfg =
              {
                  .MIP           = 472. * dd4hep::keV,
                  .Emin_in_MIPs  = 0.5,
                  .delta_in_MIPs = 0.01,
                  .tmax          = 269 * dd4hep::ns,
              },
          .level = JEventLevel::Timeslice},
      app));

//   app->Add(new JOmniFactoryGeneratorT<ImagingTopoCluster_factory>(
//       JOmniFactoryGeneratorT<ImagingTopoCluster_factory>::TypedWiring{
//           .m_tag                 = "HcalFarForwardZDCImagingProtoClusters_TK",
//           .m_default_input_tags  = {"HcalFarForwardZDCSubcellHits_TK"},
//           .m_default_output_tags = {"HcalFarForwardZDCImagingProtoClusters_TK"},
//           .m_default_cfg =
//               {
//                   .neighbourLayersRange = 1,
//                   .sameLayerDistXY =
//                       {"0.5 * HcalFarForwardZDC_SiPMonTile_HexSideLength",
//                        "0.5 * HcalFarForwardZDC_SiPMonTile_HexSideLength * sin(pi / 3)"},
//                   .diffLayerDistXY =
//                       {"0.5 * HcalFarForwardZDC_SiPMonTile_HexSideLength",
//                        "0.5 * HcalFarForwardZDC_SiPMonTile_HexSideLength * sin(pi / 3)"},
//                   .sameLayerMode        = eicrecon::ImagingTopoClusterConfig::ELayerMode::xy,
//                   .sectorDist           = 10.0 * dd4hep::cm,
//                   .minClusterHitEdep    = 50.0 * dd4hep::keV,
//                   .minClusterCenterEdep = 3.0 * dd4hep::MeV,
//                   .minClusterEdep       = 11.0 * dd4hep::MeV,
//                   .minClusterNhits      = 30,
//               },
//           .level = JEventLevel::Timeslice},
//       app));

//   app->Add(new JOmniFactoryGeneratorT<CalorimeterIslandCluster_factory>(
//       JOmniFactoryGeneratorT<CalorimeterIslandCluster_factory>::TypedWiring{
//           .m_tag                 = "HcalFarForwardZDCIslandProtoClusters_TK",
//           .m_default_input_tags  = {"HcalFarForwardZDCSubcellHits_TK"},
//           .m_default_output_tags = {"HcalFarForwardZDCIslandProtoClusters_TK"},
//           .m_default_cfg =
//               {.adjacencyMatrix{},
//                .peakNeighbourhoodMatrix{},
//                .readout{},
//                .sectorDist  = 1.5 * dd4hep::cm,
//                .localDistXY = {"0.9 * HcalFarForwardZDC_SiPMonTile_HexSideLength",
//                                "0.76 * HcalFarForwardZDC_SiPMonTile_HexSideLength * sin(pi / 3)"},
//                .localDistXZ{},
//                .localDistYZ{},
//                .globalDistRPhi{},
//                .globalDistEtaPhi{},
//                .dimScaledLocalDistXY{},
//                .splitCluster         = false,
//                .minClusterHitEdep    = 100.0 * dd4hep::keV,
//                .minClusterCenterEdep = 1.0 * dd4hep::MeV,
//                .transverseEnergyProfileMetric{}, // = "globalDistEtaPhi",
//                .transverseEnergyProfileScale{},  // = 1.,
//                .transverseEnergyProfileScaleUnits{}},
//           .level = JEventLevel::Timeslice},
//       app));

//   app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
//       JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>::TypedWiring{
//           .m_tag = "HcalFarForwardZDCClustersWithoutShapes_TK",
//           .m_default_input_tags =
//               {
//                   "HcalFarForwardZDCImagingProtoClusters_TK", // edm4eic::ProtoClusterCollection
// #if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
//                   "HcalFarForwardZDCRawHitLinks", // edm4eic::MCRecoCalorimeterHitLink
// #endif
//                   "HcalFarForwardZDCRawHitAssociations" // edm4eic::MCRecoCalorimeterHitAssociationCollection
//               },
//           .m_default_output_tags = {"HcalFarForwardZDCClustersWithoutShapes_TK",
// #if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
//                                     "HcalFarForwardZDCClusterLinksWithoutShapes_TK",
// #endif
//                                     "HcalFarForwardZDCClusterAssociationsWithoutShapes_TK"},
//           .m_default_cfg = {.energyWeight        = "log",
//                             .sampFrac            = 0.0203,
//                             .logWeightBaseCoeffs = {5.8, 0.65, 0.31},
//                             .logWeightBase_Eref  = 50 * dd4hep::GeV},
//           .level         = JEventLevel::Timeslice},
//       app));

//   app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>(
//       JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>::TypedWiring{
//           .m_tag                 = "HcalFarForwardZDCClusters_TK",
//           .m_default_input_tags  = {"HcalFarForwardZDCClustersWithoutShapes_TK",
//                                     "HcalFarForwardZDCClusterAssociationsWithoutShapes_TK"},
//           .m_default_output_tags = {"HcalFarForwardZDCClusters_TK",
// #if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
//                                     "HcalFarForwardZDCClusterLinks_TK",
// #endif
//                                     "HcalFarForwardZDCClusterAssociations_TK"},
//           .m_default_cfg = {.longitudinalShowerInfoAvailable = true,
//                             .energyWeight                    = "log",
//                             .sampFrac                        = 0.0203,
//                             .logWeightBaseCoeffs             = {5.8, 0.65, 0.31},
//                             .logWeightBase_Eref              = 50 * dd4hep::GeV},
//           .level         = JEventLevel::Timeslice},
//       app));

//   app->Add(new JOmniFactoryGeneratorT<CalorimeterTruthClustering_factory>(
//       JOmniFactoryGeneratorT<CalorimeterTruthClustering_factory>::TypedWiring{
//           .m_tag                 = "HcalFarForwardZDCTruthProtoClusters_TK",
//           .m_default_input_tags  = {"HcalFarForwardZDCRecHits_TK", "HcalFarForwardZDCHits"},
//           .m_default_output_tags = {"HcalFarForwardZDCTruthProtoClusters_TK"},
//           .m_default_cfg         = {},
//           .level                 = JEventLevel::Timeslice},
//       app));

//   //Clusters with the baseline algorithm (no HEXPLIT)
//   app->Add(new JOmniFactoryGeneratorT<CalorimeterIslandCluster_factory>(
//       JOmniFactoryGeneratorT<CalorimeterIslandCluster_factory>::TypedWiring{
//           .m_tag                 = "HcalFarForwardZDCIslandProtoClustersBaseline_TK",
//           .m_default_input_tags  = {"HcalFarForwardZDCRecHits_TK"},
//           .m_default_output_tags = {"HcalFarForwardZDCIslandProtoClustersBaseline_TK"},
//           .m_default_cfg         = {.adjacencyMatrix{},
//                                     .peakNeighbourhoodMatrix{},
//                                     .readout{},
//                                     .sectorDist  = 5.0 * dd4hep::cm,
//                                     .localDistXY = {50 * dd4hep::cm, 50 * dd4hep::cm},
//                                     .localDistXZ{},
//                                     .localDistYZ{},
//                                     .globalDistRPhi{},
//                                     .globalDistEtaPhi{},
//                                     .dimScaledLocalDistXY{},
//                                     .splitCluster                  = true,
//                                     .minClusterHitEdep             = 0.1 * dd4hep::MeV,
//                                     .minClusterCenterEdep          = 3.0 * dd4hep::MeV,
//                                     .transverseEnergyProfileMetric = "globalDistEtaPhi",
//                                     .transverseEnergyProfileScale  = 1.,
//                                     .transverseEnergyProfileScaleUnits{}},
//           .level                 = JEventLevel::Timeslice},
//       app));

//   app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
//       JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>::TypedWiring{
//           .m_tag = "HcalFarForwardZDCTruthClustersWithoutShapes_TK",
//           .m_default_input_tags =
//               {
//                   "HcalFarForwardZDCTruthProtoClusters_TK", // edm4eic::ProtoClusterCollection
// #if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
//                   "HcalFarForwardZDCRawHitLinks_TK", // edm4eic::MCRecoCalorimeterHitLink
// #endif
//                   "HcalFarForwardZDCRawHitAssociations_TK" // edm4eic::MCRecoCalorimeterHitAssociationCollection
//               },
//           .m_default_output_tags = {"HcalFarForwardZDCTruthClustersWithoutShapes_TK",
// #if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
//                                     "HcalFarForwardZDCTruthClusterLinksWithoutShapes_TK",
// #endif
//                                     "HcalFarForwardZDCTruthClusterAssociationsWithoutShapes_TK"},
//           .m_default_cfg =
//               {
//                   .energyWeight    = "log",
//                   .sampFrac        = 1.0,
//                   .logWeightBase   = 3.6,
//                   .enableEtaBounds = false,
//               },
//           .level = JEventLevel::Timeslice},
//       app));

//   app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>(
//       JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>::TypedWiring{
//           .m_tag                 = "HcalFarForwardZDCTruthClusters_TK",
//           .m_default_input_tags  = {"HcalFarForwardZDCTruthClustersWithoutShapes_TK",
//                                     "HcalFarForwardZDCTruthClusterAssociationsWithoutShapes_TK"},
//           .m_default_output_tags = {"HcalFarForwardZDCTruthClusters_TK",
// #if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
//                                     "HcalFarForwardZDCTruthClusterLinks_TK",
// #endif
//                                     "HcalFarForwardZDCTruthClusterAssociations_TK"},
//           .m_default_cfg = {.longitudinalShowerInfoAvailable = true,
//                             .energyWeight                    = "log",
//                             .logWeightBase                   = 3.6},
//           .level         = JEventLevel::Timeslice},
//       app));

//   app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
//       JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>::TypedWiring{
//           .m_tag = "HcalFarForwardZDCClustersBaselineWithoutShapes_TK",
//           .m_default_input_tags =
//               {
//                   "HcalFarForwardZDCIslandProtoClustersBaseline_TK", // edm4eic::ProtoClusterCollection
// #if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
//                   "HcalFarForwardZDCRawHitLinks_TK", // edm4eic::MCRecoCalorimeterHitLink
// #endif
//                   "HcalFarForwardZDCRawHitAssociations_TK" // edm4eic::MCRecoCalorimeterHitAssociationCollection
//               },
//           .m_default_output_tags = {"HcalFarForwardZDCClustersBaselineWithoutShapes_TK",
// #if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
//                                     "HcalFarForwardZDCClusterLinksBaselineWithoutShapes_TK",
// #endif
//                                     "HcalFarForwardZDCClusterAssociationsBaselineWithoutShapes_TK"},
//           .m_default_cfg =
//               {
//                   .energyWeight    = "log",
//                   .sampFrac        = 0.0203,
//                   .logWeightBase   = 6.2,
//                   .enableEtaBounds = false,
//               },
//           .level = JEventLevel::Timeslice},
//       app));

//   app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>(
//       JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>::TypedWiring{
//           .m_tag                 = "HcalFarForwardZDCClustersBaseline_TK",
//           .m_default_input_tags  = {"HcalFarForwardZDCClustersBaselineWithoutShapes_TK",
//                                     "HcalFarForwardZDCClusterAssociationsBaselineWithoutShapes_TK"},
//           .m_default_output_tags = {"HcalFarForwardZDCClustersBaseline_TK",
// #if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
//                                     "HcalFarForwardZDCClusterLinksBaseline_TK",
// #endif
//                                     "HcalFarForwardZDCClusterAssociationsBaseline_TK"},
//           .m_default_cfg = {.longitudinalShowerInfoAvailable = true,
//                             .energyWeight                    = "log",
//                             .sampFrac                        = 0.0203,
//                             .logWeightBase                   = 6.2},
//           .level         = JEventLevel::Timeslice},
//       app));
}
// }
