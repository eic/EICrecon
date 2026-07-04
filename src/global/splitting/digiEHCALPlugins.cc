// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2025 Sylvester Joosten, Chao, Chao Peng, Whitney Armstrong, David Lawrence, Friederike Bock, Nathan Brei, Wouter Deconinck, Dmitry Kalinkin, Derek Anderson

#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplicationFwd.h>
#include <edm4eic/EDM4eicVersion.h>
#include <JANA/Utils/JTypeInfo.h>
#include <string>
#include <variant>
#include <vector>

#include "algorithms/calorimetry/CalorimeterHitDigiConfig.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/calorimetry/CalorimeterClusterRecoCoG_factory.h"
#include "factories/calorimetry/CalorimeterClusterShape_factory.h"
#include "factories/calorimetry/CalorimeterHitDigi_factory.h"
#include "factories/calorimetry/CalorimeterHitReco_factory.h"
#include "factories/calorimetry/CalorimeterHitsMerger_factory.h"
#include "factories/calorimetry/CalorimeterIslandCluster_factory.h"
#include "factories/calorimetry/CalorimeterTruthClustering_factory.h"
#include "factories/calorimetry/TrackClusterMergeSplitter_factory.h"

// extern "C" {
void InitPluginEHCAL_digi(JApplication* app) {

  using namespace eicrecon;

  InitJANAPlugin(app);
  // Make sure digi and reco use the same value
  decltype(CalorimeterHitDigiConfig::capADC) HcalEndcapN_capADC =
      32768; // assuming 15 bit ADC like FHCal
  decltype(CalorimeterHitDigiConfig::dyRangeADC) HcalEndcapN_dyRangeADC =
      200 * dd4hep::MeV; // to be verified with simulations
  decltype(CalorimeterHitDigiConfig::pedMeanADC) HcalEndcapN_pedMeanADC   = 10;
  decltype(CalorimeterHitDigiConfig::pedSigmaADC) HcalEndcapN_pedSigmaADC = 2;
  decltype(CalorimeterHitDigiConfig::resolutionTDC) HcalEndcapN_resolutionTDC =
      10 * dd4hep::picosecond;

  app->Add(new JOmniFactoryGeneratorT<CalorimeterHitDigi_factory>(
      JOmniFactoryGeneratorT<CalorimeterHitDigi_factory>::TypedWiring{
          .m_tag                 = "HcalEndcapNRawHits_TK",
          .m_default_input_tags  = {"EventHeader", "HcalEndcapNHits"},
          .m_default_output_tags = {"HcalEndcapNRawHits_TK",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                                    "HcalEndcapNRawHitLinks_TK",
#endif
                                    "HcalEndcapNRawHitAssociations_TK"},
          .m_default_cfg =
              {
                  .eRes{},
                  .tRes          = 0.0 * dd4hep::ns,
                  .capADC        = HcalEndcapN_capADC,
                  .capTime       = 100, // given in ns, 4 samples in HGCROC
                  .dyRangeADC    = HcalEndcapN_dyRangeADC,
                  .pedMeanADC    = HcalEndcapN_pedMeanADC,
                  .pedSigmaADC   = HcalEndcapN_pedSigmaADC,
                  .resolutionTDC = HcalEndcapN_resolutionTDC,
                  .corrMeanScale = "1.0",
                  .readout       = "HcalEndcapNHits",
              },
          .level = JEventLevel::Timeslice},
      app));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterHitReco_factory>(
      JOmniFactoryGeneratorT<CalorimeterHitReco_factory>::TypedWiring{
          .m_tag                 = "HcalEndcapNRecHits_TK",
          .m_default_input_tags  = {"HcalEndcapNRawHits_TK"},
          .m_default_output_tags = {"HcalEndcapNRecHits_TK"},
          .m_default_cfg =
              {
                  .capADC          = HcalEndcapN_capADC,
                  .dyRangeADC      = HcalEndcapN_dyRangeADC,
                  .pedMeanADC      = HcalEndcapN_pedMeanADC,
                  .pedSigmaADC     = HcalEndcapN_pedSigmaADC,
                  .resolutionTDC   = HcalEndcapN_resolutionTDC,
                  .thresholdFactor = 0.0,
                  .thresholdValue =
                      41.0, // 0.1875 MeV deposition out of 200 MeV max (per layer) --> adc = 10 + 0.1875 / 200 * 32768 == 41
                  .sampFrac =
                      "0.0095", // from latest study - implement at level of reco hits rather than clusters
                  .readout = "HcalEndcapNHits",
              },
          .level = JEventLevel::Timeslice},
      app));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterHitsMerger_factory>(
      JOmniFactoryGeneratorT<CalorimeterHitsMerger_factory>::TypedWiring{
          .m_tag                 = "HcalEndcapNMergedHits_TK",
          .m_default_input_tags  = {"HcalEndcapNRecHits_TK"},
          .m_default_output_tags = {"HcalEndcapNMergedHits_TK"},
          .m_default_cfg         = {.readout              = "HcalEndcapNHits",
                                    .fieldTransformations = {"layer:4", "slice:0"}},
          .level                 = JEventLevel::Timeslice},
      app));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterTruthClustering_factory>(
      JOmniFactoryGeneratorT<CalorimeterTruthClustering_factory>::TypedWiring{
          .m_tag                 = "HcalEndcapNTruthProtoClusters_TK",
          .m_default_input_tags  = {"HcalEndcapNMergedHits_TK", "HcalEndcapNHits"},
          .m_default_output_tags = {"HcalEndcapNTruthProtoClusters_TK"},
          .m_default_cfg         = {},
          .level                 = JEventLevel::Timeslice},
      app));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterIslandCluster_factory>(
      JOmniFactoryGeneratorT<CalorimeterIslandCluster_factory>::TypedWiring{
          .m_tag                 = "HcalEndcapNIslandProtoClusters_TK",
          .m_default_input_tags  = {"HcalEndcapNMergedHits_TK"},
          .m_default_output_tags = {"HcalEndcapNIslandProtoClusters_TK"},
          .m_default_cfg =
              {
                  .adjacencyMatrix{},
                  .peakNeighbourhoodMatrix{},
                  .readout{},
                  .sectorDist  = 5.0 * dd4hep::cm,
                  .localDistXY = {15 * dd4hep::cm, 15 * dd4hep::cm},
                  .localDistXZ{},
                  .localDistYZ{},
                  .globalDistRPhi{},
                  .globalDistEtaPhi{},
                  .dimScaledLocalDistXY{},
                  .splitCluster                  = true,
                  .minClusterHitEdep             = 0.0 * dd4hep::MeV,
                  .minClusterCenterEdep          = 30.0 * dd4hep::MeV,
                  .transverseEnergyProfileMetric = "globalDistEtaPhi",
                  .transverseEnergyProfileScale  = 1.,
                  .transverseEnergyProfileScaleUnits{},
              },
          .level = JEventLevel::Timeslice},
      app));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
      JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>::TypedWiring{
          .m_tag = "HcalEndcapNTruthClustersWithoutShapes_TK",
          .m_default_input_tags =
              {
                  "HcalEndcapNTruthProtoClusters_TK", // edm4eic::ProtoClusterCollection
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                  "HcalEndcapNRawHitLinks_TK", // edm4eic::MCRecoCalorimeterHitLink
#endif
                  "HcalEndcapNRawHitAssociations_TK" // edm4eic::MCRecoCalorimeterHitAssociationCollection
              },
          .m_default_output_tags = {"HcalEndcapNTruthClustersWithoutShapes_TK",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                                    "HcalEndcapNTruthClusterLinksWithoutShapes_TK",
#endif
                                    "HcalEndcapNTruthClusterAssociationsWithoutShapes_TK"},
          .m_default_cfg =
              {
                  .energyWeight    = "log",
                  .sampFrac        = 1.0,
                  .logWeightBase   = 6.2,
                  .enableEtaBounds = false,
              },
          .level = JEventLevel::Timeslice},
      app));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>(
      JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>::TypedWiring{
          .m_tag                 = "HcalEndcapNTruthClusters_TK",
          .m_default_input_tags  = {"HcalEndcapNTruthClustersWithoutShapes_TK",
                                    "HcalEndcapNTruthClusterAssociationsWithoutShapes_TK"},
          .m_default_output_tags = {"HcalEndcapNTruthClusters_TK",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                                    "HcalEndcapNTruthClusterLinks_TK",
#endif
                                    "HcalEndcapNTruthClusterAssociations_TK"},
          .m_default_cfg = {.energyWeight = "log", .logWeightBase = 6.2},
          .level         = JEventLevel::Timeslice},
      app));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
      JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>::TypedWiring{
          .m_tag = "HcalEndcapNClustersWithoutShapes_TK",
          .m_default_input_tags =
              {
                  "HcalEndcapNIslandProtoClusters_TK", // edm4eic::ProtoClusterCollection
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                  "HcalEndcapNRawHitLinks_TK", // edm4eic::MCRecoCalorimeterHitLink
#endif
                  "HcalEndcapNRawHitAssociations_TK" // edm4eic::MCRecoCalorimeterHitAssociationCollection
              },
          .m_default_output_tags = {"HcalEndcapNClustersWithoutShapes_TK",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                                    "HcalEndcapNClusterLinksWithoutShapes_TK",
#endif
                                    "HcalEndcapNClusterAssociationsWithoutShapes_TK"},
          .m_default_cfg =
              {
                  .energyWeight    = "log",
                  .sampFrac        = 1.0,
                  .logWeightBase   = 6.2,
                  .enableEtaBounds = false,
              },
          .level = JEventLevel::Timeslice},
      app));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>(
      JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>::TypedWiring{
          .m_tag                 = "HcalEndcapNClusters_TK",
          .m_default_input_tags  = {"HcalEndcapNClustersWithoutShapes_TK",
                                    "HcalEndcapNClusterAssociationsWithoutShapes_TK"},
          .m_default_output_tags = {"HcalEndcapNClusters_TK",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                                    "HcalEndcapNClusterLinks_TK",
#endif
                                    "HcalEndcapNClusterAssociations_TK"},
          .m_default_cfg = {.energyWeight = "log", .logWeightBase = 6.2},
          .level         = JEventLevel::Timeslice},
      app));
  app->Add(new JOmniFactoryGeneratorT<TrackClusterMergeSplitter_factory>(
      JOmniFactoryGeneratorT<TrackClusterMergeSplitter_factory>::TypedWiring{
          .m_tag                 = "HcalEndcapNSplitMergeProtoClusters_TK",
          .m_default_input_tags  = {"HcalEndcapNIslandProtoClusters_TK",
                                    "CalorimeterTrackProjections"},
          .m_default_output_tags = {"HcalEndcapNSplitMergeProtoClusters_TK"},
          .m_default_cfg         = {.idCalo                       = "HcalEndcapN_ID",
                                    .minSigCut                    = -2.0,
                                    .avgEP                        = 0.60,
                                    .sigEP                        = 0.40,
                                    .drAdd                        = 0.40,
                                    .sampFrac                     = 1.0,
                                    .transverseEnergyProfileScale = 1.0},
          .level                 = JEventLevel::Timeslice},
      app));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
      JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>::TypedWiring{
          .m_tag = "HcalEndcapNSplitMergeClustersWithoutShapes_TK",
          .m_default_input_tags =
              {
                  "HcalEndcapNSplitMergeProtoClusters_TK", // edm4eic::ProtoClusterCollection
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                  "HcalEndcapNRawHitLinks_TK", // edm4eic::MCRecoCalorimeterHitLink
#endif
                  "HcalEndcapNRawHitAssociations_TK" // edm4hep::MCRecoCalorimeterHitAssociationCollection
              },
          .m_default_output_tags = {"HcalEndcapNSplitMergeClustersWithoutShapes_TK",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                                    "HcalEndcapNSplitMergeClusterLinksWithoutShapes_TK",
#endif
                                    "HcalEndcapNSplitMergeClusterAssociationsWithoutShapes_TK"},
          .m_default_cfg =
              {
                  .energyWeight    = "log",
                  .sampFrac        = 1.0,
                  .logWeightBase   = 6.2,
                  .enableEtaBounds = false,
              },
          .level = JEventLevel::Timeslice},
      app));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>(
      JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>::TypedWiring{
          .m_tag                 = "HcalEndcapNSplitMergeClusters_TK",
          .m_default_input_tags  = {"HcalEndcapNSplitMergeClustersWithoutShapes_TK",
                                    "HcalEndcapNSplitMergeClusterAssociationsWithoutShapes_TK"},
          .m_default_output_tags = {"HcalEndcapNSplitMergeClusters_TK",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                                    "HcalEndcapNSplitMergeClusterLinks_TK",
#endif
                                    "HcalEndcapNSplitMergeClusterAssociations_TK"},
          .m_default_cfg = {.energyWeight = "log", .logWeightBase = 6.2},
          .level         = JEventLevel::Timeslice},
      app));
}
// }
