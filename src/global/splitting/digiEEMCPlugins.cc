// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2025 Sylvester Joosten, Chao, Chao Peng, Whitney Armstrong, Thomas Britton, David Lawrence, Dhevan Gangadharan, Wouter Deconinck, Dmitry Kalinkin, Derek Anderson

#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplicationFwd.h>
#include <edm4eic/EDM4eicVersion.h>
#include <JANA/Utils/JTypeInfo.h>
#include <cmath>
#include <string>
#include <variant>
#include <vector>

#include "algorithms/calorimetry/CalorimeterHitDigiConfig.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/calorimetry/CalorimeterClusterRecoCoG_factory.h"
#include "factories/calorimetry/CalorimeterHitDigi_factory.h"
#include "factories/calorimetry/CalorimeterHitReco_factory.h"
#include "factories/calorimetry/CalorimeterIslandCluster_factory.h"
#include "factories/calorimetry/CalorimeterParticleIDPostML_factory.h"
#include "factories/calorimetry/CalorimeterParticleIDPreML_factory.h"
#include "factories/calorimetry/CalorimeterClusterShape_factory.h"
#include "factories/calorimetry/CalorimeterTruthClustering_factory.h"
#include "factories/calorimetry/TrackClusterMergeSplitter_factory.h"
#include "factories/meta/ONNXInference_factory.h"

// extern "C" {
void InitPlugin_digiEEMC(JApplication* app) {

  using namespace eicrecon;

  InitJANAPlugin(app);

  // Make sure digi and reco use the same value
  decltype(CalorimeterHitDigiConfig::capADC) EcalEndcapN_capADC         = 16384; //65536,  16bit ADC
  decltype(CalorimeterHitDigiConfig::dyRangeADC) EcalEndcapN_dyRangeADC = 20.0 * dd4hep::GeV;
  decltype(CalorimeterHitDigiConfig::pedMeanADC) EcalEndcapN_pedMeanADC = 20;
  decltype(CalorimeterHitDigiConfig::pedSigmaADC) EcalEndcapN_pedSigmaADC = 1;
  decltype(CalorimeterHitDigiConfig::resolutionTDC) EcalEndcapN_resolutionTDC =
      10 * dd4hep::picosecond;
  app->Add(new JOmniFactoryGeneratorT<CalorimeterHitDigi_factory>(
      JOmniFactoryGeneratorT<CalorimeterHitDigi_factory>::TypedWiring{
          .m_tag                 = "EcalEndcapNRawHits_TK",
          .m_default_input_tags  = {"EventHeader", "EcalEndcapNHits"},
          .m_default_output_tags = {"EcalEndcapNRawHits_TK",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                                    "EcalEndcapNRawHitLinks_TK",
#endif
                                    "EcalEndcapNRawHitAssociations_TK"},
          .m_default_cfg =
              {
                  .eRes        = {0.0 * sqrt(dd4hep::GeV), 0.0, 0.0 * dd4hep::GeV},
                  .tRes        = 0.0 * dd4hep::ns,
                  .threshold   = 0.0 * dd4hep::MeV, // Use ADC cut instead
                  .readoutType = "sipm",
                  // 18. pe/MeV is measured with PMT at 25% QE
                  .lightYield = 18. / 0.25 / dd4hep::MeV,
                  // Based on slide 6 of https://indico.bnl.gov/event/29076/contributions/110749/attachments/63706/109457/Calo_meeting_Jun25_Updated.pdf
                  // Geometric factor for 16 of 3x3 mm^2 sensors covering 20x20 mm^2 area for sensor with 28% QE
                  .photonDetectionEfficiency = (16 * (3. * 3.) / (20. * 20.)) * 0.28,
                  // S14160-3015PS, 16 sensors per cell
                  .numEffectiveSipmPixels = 39984ULL * 16,
                  .capADC                 = EcalEndcapN_capADC,
                  .dyRangeADC             = EcalEndcapN_dyRangeADC,
                  .pedMeanADC             = EcalEndcapN_pedMeanADC,
                  .pedSigmaADC            = EcalEndcapN_pedSigmaADC,
                  .resolutionTDC          = EcalEndcapN_resolutionTDC,
                  .corrMeanScale          = "1.0",
                  .readout                = "EcalEndcapNHits",
              },
          .level = JEventLevel::Timeslice},
      app));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterHitReco_factory>(
      JOmniFactoryGeneratorT<CalorimeterHitReco_factory>::TypedWiring{
          .m_tag                 = "EcalEndcapNRecHits_TK",
          .m_default_input_tags  = {"EcalEndcapNRawHits_TK"},
          .m_default_output_tags = {"EcalEndcapNRecHits_TK"},
          .m_default_cfg =
              {
                  .capADC          = EcalEndcapN_capADC,
                  .dyRangeADC      = EcalEndcapN_dyRangeADC,
                  .pedMeanADC      = EcalEndcapN_pedMeanADC,
                  .pedSigmaADC     = EcalEndcapN_pedSigmaADC,
                  .resolutionTDC   = EcalEndcapN_resolutionTDC,
                  .thresholdFactor = 0.0,
                  .thresholdValue  = 4.0, // (20. GeV / 16384) * 4 ~= 5 MeV
                  .sampFrac        = "0.96",
                  .readout         = "EcalEndcapNHits",
              },
          .level = JEventLevel::Timeslice},
      app));

//   app->Add(new JOmniFactoryGeneratorT<CalorimeterTruthClustering_factory>(
//       JOmniFactoryGeneratorT<CalorimeterTruthClustering_factory>::TypedWiring{
//           .m_tag                 = "EcalEndcapNTruthProtoClusters_TK",
//           .m_default_input_tags  = {"EcalEndcapNRecHits_TK", "EcalEndcapNHits"},
//           .m_default_output_tags = {"EcalEndcapNTruthProtoClusters_TK"},
//           .m_default_cfg         = {},
//           .level                 = JEventLevel::Timeslice},
//       app));
//   app->Add(new JOmniFactoryGeneratorT<CalorimeterIslandCluster_factory>(
//       JOmniFactoryGeneratorT<CalorimeterIslandCluster_factory>::TypedWiring{
//           .m_tag                 = "EcalEndcapNIslandProtoClusters_TK",
//           .m_default_input_tags  = {"EcalEndcapNRecHits_TK"},
//           .m_default_output_tags = {"EcalEndcapNIslandProtoClusters_TK"},
//           .m_default_cfg =
//               {
//                   .adjacencyMatrix = "(abs(row_1 - row_2) + abs(column_1 - column_2)) == 1",
//                   .peakNeighbourhoodMatrix =
//                       "max(abs(row_1 - row_2), abs(column_1 - column_2)) == 1",
//                   .readout    = "EcalEndcapNHits",
//                   .sectorDist = 5.0 * dd4hep::cm,
//                   .localDistXY{},
//                   .localDistXZ{},
//                   .localDistYZ{},
//                   .globalDistRPhi{},
//                   .globalDistEtaPhi{},
//                   .dimScaledLocalDistXY{},
//                   .splitCluster                  = true,
//                   .minClusterHitEdep             = 1.0 * dd4hep::MeV,
//                   .minClusterCenterEdep          = 30.0 * dd4hep::MeV,
//                   .transverseEnergyProfileMetric = "globalDistEtaPhi",
//                   .transverseEnergyProfileScale  = 0.08,
//                   .transverseEnergyProfileScaleUnits{},
//               },
//           .level = JEventLevel::Timeslice},
//       app));

//   app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
//       JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>::TypedWiring{
//           .m_tag = "EcalEndcapNTruthClustersWithoutShapes_TK",
//           .m_default_input_tags =
//               {
//                   "EcalEndcapNTruthProtoClusters_TK", // edm4eic::ProtoClusterCollection
// #if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
//                   "EcalEndcapNRawHitLinks_TK", // edm4eic::MCRecoCalorimeterHitLink
// #endif
//                   "EcalEndcapNRawHitAssociations_TK" // edm4eic::MCRecoCalorimeterHitAssociationCollection
//               },
//           .m_default_output_tags = {"EcalEndcapNTruthClustersWithoutShapes_TK",
// #if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
//                                     "EcalEndcapNTruthClusterLinksWithoutShapes_TK",
// #endif
//                                     "EcalEndcapNTruthClusterAssociationsWithoutShapes_TK"},
//           .m_default_cfg =
//               {
//                   .energyWeight    = "log",
//                   .sampFrac        = 1.0,
//                   .logWeightBase   = 4.6,
//                   .enableEtaBounds = false,
//               },
//           .level = JEventLevel::Timeslice},
//       app));

//   app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>(
//       JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>::TypedWiring{
//           .m_tag                 = "EcalEndcapNTruthClusters_TK",
//           .m_default_input_tags  = {"EcalEndcapNTruthClustersWithoutShapes_TK",
//                                     "EcalEndcapNTruthClusterAssociationsWithoutShapes_TK"},
//           .m_default_output_tags = {"EcalEndcapNTruthClusters_TK",
// #if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
//                                     "EcalEndcapNTruthClusterLinks_TK",
// #endif
//                                     "EcalEndcapNTruthClusterAssociations_TK"},
//           .m_default_cfg = {.energyWeight = "log", .logWeightBase = 4.6},
//           .level         = JEventLevel::Timeslice},
//       app));

//   app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
//       JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>::TypedWiring{
//           .m_tag = "EcalEndcapNClustersWithoutPIDAndShapes_TK",
//           .m_default_input_tags =
//               {
//                   "EcalEndcapNIslandProtoClusters_TK", // edm4eic::ProtoClusterCollection
// #if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
//                   "EcalEndcapNRawHitLinks_TK", // edm4eic::MCRecoCalorimeterHitLink
// #endif
//                   "EcalEndcapNRawHitAssociations_TK" // edm4eic::MCRecoCalorimeterHitAssociationCollection
//               },
//           .m_default_output_tags =
//               {"EcalEndcapNClustersWithoutPIDAndShapes_TK", // edm4eic::Cluster
// #if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
//                "EcalEndcapNClusterLinksWithoutPIDAndShapes_TK", // edm4eic::MCRecoClusterParticleLink
// #endif
//                "EcalEndcapNClusterAssociationsWithoutPIDAndShapes_TK"},
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
//           .m_tag                 = "EcalEndcapNClustersWithoutPID_TK",
//           .m_default_input_tags  = {"EcalEndcapNClustersWithoutPIDAndShapes_TK",
//                                     "EcalEndcapNClusterAssociationsWithoutPIDAndShapes_TK"},
//           .m_default_output_tags = {"EcalEndcapNClustersWithoutPID_TK",
// #if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
//                                     "EcalEndcapNClusterLinksWithoutPID_TK",
// #endif
//                                     "EcalEndcapNClusterAssociationsWithoutPID_TK"},
//           .m_default_cfg = {.energyWeight = "log", .logWeightBase = 3.6},
//           .level         = JEventLevel::Timeslice},
//       app));

//   app->Add(new JOmniFactoryGeneratorT<TrackClusterMergeSplitter_factory>(
//       JOmniFactoryGeneratorT<TrackClusterMergeSplitter_factory>::TypedWiring{
//           .m_tag                 = "EcalEndcapNSplitMergeProtoClusters_TK",
//           .m_default_input_tags  = {"EcalEndcapNIslandProtoClusters_TK",
//                                     "CalorimeterTrackProjections"},
//           .m_default_output_tags = {"EcalEndcapNSplitMergeProtoClusters_TK"},
//           .m_default_cfg         = {.idCalo                       = "EcalEndcapN_ID",
//                                     .minSigCut                    = -1.0,
//                                     .avgEP                        = 1.0,
//                                     .sigEP                        = 0.10,
//                                     .drAdd                        = 0.08,
//                                     .sampFrac                     = 1.0,
//                                     .transverseEnergyProfileScale = 1.0},
//           .level                 = JEventLevel::Timeslice},
//       app));

//   app->Add(new JOmniFactoryGeneratorT<CalorimeterParticleIDPreML_factory>(
//       JOmniFactoryGeneratorT<CalorimeterParticleIDPreML_factory>::TypedWiring{
//           .m_tag = "EcalEndcapNParticleIDPreML_TK",
//           .m_default_input_tags =
//               {
//                   "EcalEndcapNClustersWithoutPID_TK",
//                   "EcalEndcapNClusterAssociationsWithoutPID_TK",
//               },
//           .m_default_output_tags =
//               {
//                   "EcalEndcapNParticleIDInput_features_TK",
//                   "EcalEndcapNParticleIDTarget_TK",
//               },
//           .m_default_cfg = {},
//           .level         = JEventLevel::Timeslice},
//       app));
//   app->Add(new JOmniFactoryGeneratorT<ONNXInference_factory>(
//       JOmniFactoryGeneratorT<ONNXInference_factory>::TypedWiring{
//           .m_tag = "EcalEndcapNParticleIDInference_TK",
//           .m_default_input_tags =
//               {
//                   "EcalEndcapNParticleIDInput_features_TK",
//               },
//           .m_default_output_tags =
//               {
//                   "EcalEndcapNParticleIDOutput_label_TK",
//                   "EcalEndcapNParticleIDOutput_probability_tensor_TK",
//               },
//           .m_default_cfg =
//               {
//                   .modelPath = "calibrations/onnx/EcalEndcapN_pi_rejection.onnx",
//               },
//           .level = JEventLevel::Timeslice},
//       app));
//   app->Add(new JOmniFactoryGeneratorT<CalorimeterParticleIDPostML_factory>(
//       JOmniFactoryGeneratorT<CalorimeterParticleIDPostML_factory>::TypedWiring{
//           .m_tag = "EcalEndcapNParticleIDPostML_TK",
//           .m_default_input_tags =
//               {
//                   "EcalEndcapNClustersWithoutPID_TK",
//                   "EcalEndcapNClusterAssociationsWithoutPID_TK",
//                   "EcalEndcapNParticleIDOutput_probability_tensor_TK",
//               },
//           .m_default_output_tags =
//               {
//                   "EcalEndcapNClusters_TK",
// #if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
//                   "EcalEndcapNClusterLinks_TK",
// #endif
//                   "EcalEndcapNClusterAssociations_TK",
//                   "EcalEndcapNClusterParticleIDs_TK",
//               },
//           .m_default_cfg = {},
//           .level         = JEventLevel::Timeslice},
//       app));

//   app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
//       JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>::TypedWiring{
//           .m_tag = "EcalEndcapNSplitMergeClustersWithoutShapes_TK",
//           .m_default_input_tags =
//               {
//                   "EcalEndcapNSplitMergeProtoClusters_TK", // edm4eic::ProtoClusterCollection
// #if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
//                   "EcalEndcapNRawHitLinks_TK", // edm4eic::MCRecoCalorimeterHitLink
// #endif
//                   "EcalEndcapNRawHitAssociations_TK" // edm4hep::MCRecoCalorimeterHitAssociationCollection
//               },
//           .m_default_output_tags = {"EcalEndcapNSplitMergeClustersWithoutShapes_TK",
// #if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
//                                     "EcalEndcapNSplitMergeClusterLinksWithoutShapes_TK",
// #endif
//                                     "EcalEndcapNSplitMergeClusterAssociationsWithoutShapes_TK"},
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
//           .m_tag                 = "EcalEndcapNSplitMergeClusters_TK",
//           .m_default_input_tags  = {"EcalEndcapNSplitMergeClustersWithoutShapes_TK",
//                                     "EcalEndcapNSplitMergeClusterAssociationsWithoutShapes_TK"},
//           .m_default_output_tags = {"EcalEndcapNSplitMergeClusters_TK",
// #if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
//                                     "EcalEndcapNSplitMergeClusterLinks_TK",
// #endif
//                                     "EcalEndcapNSplitMergeClusterAssociations_TK"},
//           .m_default_cfg = {.energyWeight = "log", .logWeightBase = 3.6},
//           .level         = JEventLevel::Timeslice},
//       app));
}
// }
