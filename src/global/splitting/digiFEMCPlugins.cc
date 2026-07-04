// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2021 - 2025, Chao Peng, Sylvester Joosten, Whitney Armstrong, David Lawrence, Friederike Bock, Wouter Deconinck, Kolja Kauder, Sebouh Paul, Akio Ogawa

#include <DD4hep/Detector.h>
#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplication.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/Utils/JTypeInfo.h>
#include <edm4eic/EDM4eicVersion.h>
#include <fmt/format.h>
#include <spdlog/logger.h>
#include <cmath>
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
#include "factories/calorimetry/CalorimeterIslandCluster_factory.h"
#include "factories/calorimetry/CalorimeterTruthClustering_factory.h"
#include "factories/calorimetry/TrackClusterMergeSplitter_factory.h"
#include "services/geometry/dd4hep/DD4hep_service.h"
#include "services/log/Log_service.h"

// extern "C" {
void InitPlugin_digiFEMC(JApplication* app) {

  using namespace eicrecon;

  InitJANAPlugin(app);

  auto log_service = app->GetService<Log_service>();
  auto mLog        = log_service->logger("FEMC");

  // Make sure digi and reco use the same value
  decltype(CalorimeterHitDigiConfig::capADC) EcalEndcapP_capADC =
      16384; //16384, assuming 14 bits. For approximate HGCROC resolution use 65536
  decltype(CalorimeterHitDigiConfig::dyRangeADC) EcalEndcapP_dyRangeADC   = 100 * dd4hep::GeV;
  decltype(CalorimeterHitDigiConfig::pedMeanADC) EcalEndcapP_pedMeanADC   = 200;
  decltype(CalorimeterHitDigiConfig::pedSigmaADC) EcalEndcapP_pedSigmaADC = 2.4576;
  decltype(CalorimeterHitDigiConfig::resolutionTDC) EcalEndcapP_resolutionTDC =
      10 * dd4hep::picosecond;
  const double EcalEndcapP_sampFrac = 0.029043; // updated with ratio to ScFi model
  decltype(CalorimeterHitDigiConfig::corrMeanScale) EcalEndcapP_corrMeanScale =
      fmt::format("{}", 1.0 / EcalEndcapP_sampFrac); //only used for ScFi model
  const double EcalEndcapP_nPhotonPerGeV          = 1500;
  const double EcalEndcapP_PhotonCollectionEff    = 0.285;
  const unsigned long long EcalEndcapP_totalPixel = 4 * 159565ULL;

  int EcalEndcapP_homogeneousFlag = 0;
  try {
    auto detector               = app->GetService<DD4hep_service>()->detector();
    EcalEndcapP_homogeneousFlag = detector->constant<int>("EcalEndcapP_Homogeneous_ScFi");
    if (EcalEndcapP_homogeneousFlag <= 1) {
      mLog->info("Homogeneous geometry loaded");
    } else {
      mLog->info("ScFi geometry loaded");
    }
  } catch (...) {
    // Variable not present apply legacy homogeneous geometry implementation
    EcalEndcapP_homogeneousFlag = 0;
  };

  if (EcalEndcapP_homogeneousFlag <= 1) {
    app->Add(new JOmniFactoryGeneratorT<CalorimeterHitDigi_factory>(
        JOmniFactoryGeneratorT<CalorimeterHitDigi_factory>::TypedWiring{
            .m_tag                 = "EcalEndcapPRawHits_TK",
            .m_default_input_tags  = {"EventHeader", "EcalEndcapPHits"},
            .m_default_output_tags = {"EcalEndcapPRawHits_TK",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                                      "EcalEndcapPRawHitLinks_TK",
#endif
                                      "EcalEndcapPRawHitAssociations_TK"},
            .m_default_cfg =
                {
                    .eRes = {0.11333 * sqrt(dd4hep::GeV), 0.03,
                             0.0 * dd4hep::GeV}, // (11.333% / sqrt(E)) \oplus 3%
                    .tRes = 0.0,
                    .threshold =
                        0.0, // 15MeV threshold for a single tower will be applied on ADC at Reco below
                    .readoutType = "sipm",
                    .lightYield  = EcalEndcapP_nPhotonPerGeV / EcalEndcapP_PhotonCollectionEff,
                    .photonDetectionEfficiency = EcalEndcapP_PhotonCollectionEff,
                    .numEffectiveSipmPixels    = EcalEndcapP_totalPixel,
                    .capADC                    = EcalEndcapP_capADC,
                    .capTime                   = 100, // given in ns, 4 samples in HGCROC
                    .dyRangeADC                = EcalEndcapP_dyRangeADC,
                    .pedMeanADC                = EcalEndcapP_pedMeanADC,
                    .pedSigmaADC               = EcalEndcapP_pedSigmaADC,
                    .resolutionTDC             = EcalEndcapP_resolutionTDC,
                    .corrMeanScale             = "1.0",
                    .readout                   = "EcalEndcapPHits",
                },
            .level = JEventLevel::Timeslice},
        app));
  } else if (EcalEndcapP_homogeneousFlag == 2) {
    app->Add(new JOmniFactoryGeneratorT<CalorimeterHitDigi_factory>(
        JOmniFactoryGeneratorT<CalorimeterHitDigi_factory>::TypedWiring{
            .m_tag                 = "EcalEndcapPRawHits_TK",
            .m_default_input_tags  = {"EventHeader", "EcalEndcapPHits"},
            .m_default_output_tags = {"EcalEndcapPRawHits_TK",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                                      "EcalEndcapPRawHitLinks_TK",
#endif
                                      "EcalEndcapPRawHitAssociations_TK"},
            .m_default_cfg =
                {
                    .eRes = {0.0, 0.022,
                             0.0}, // just constant term 2.2% based on MC data comparison
                    .tRes = 0.0,
                    .threshold =
                        0.0, // 15MeV threshold for a single tower will be applied on ADC at Reco below
                    .readoutType = "sipm",
                    .lightYield  = EcalEndcapP_nPhotonPerGeV / EcalEndcapP_PhotonCollectionEff /
                                  EcalEndcapP_sampFrac,
                    .photonDetectionEfficiency = EcalEndcapP_PhotonCollectionEff,
                    .numEffectiveSipmPixels    = EcalEndcapP_totalPixel,
                    .capADC                    = EcalEndcapP_capADC,
                    .capTime                   = 100, // given in ns, 4 samples in HGCROC
                    .dyRangeADC                = EcalEndcapP_dyRangeADC,
                    .pedMeanADC                = EcalEndcapP_pedMeanADC,
                    .pedSigmaADC               = EcalEndcapP_pedSigmaADC,
                    .resolutionTDC             = EcalEndcapP_resolutionTDC,
                    .corrMeanScale             = EcalEndcapP_corrMeanScale,
                    .readout                   = "EcalEndcapPHits",
                    .fields                    = {"fiber_x", "fiber_y"},
                },
            .level = JEventLevel::Timeslice},
        app));
  }

  app->Add(new JOmniFactoryGeneratorT<CalorimeterHitReco_factory>(
      JOmniFactoryGeneratorT<CalorimeterHitReco_factory>::TypedWiring{
          .m_tag                 = "EcalEndcapPRecHits_TK",
          .m_default_input_tags  = {"EcalEndcapPRawHits_TK"},
          .m_default_output_tags = {"EcalEndcapPRecHits_TK"},
          .m_default_cfg =
              {
                  .capADC          = EcalEndcapP_capADC,
                  .dyRangeADC      = EcalEndcapP_dyRangeADC,
                  .pedMeanADC      = EcalEndcapP_pedMeanADC,
                  .pedSigmaADC     = EcalEndcapP_pedSigmaADC,
                  .resolutionTDC   = EcalEndcapP_resolutionTDC,
                  .thresholdFactor = 0.0,
                  .thresholdValue =
                      3, // The ADC of a 15 MeV particle is adc = 200 + 15 * 0.03 * ( 1.0 + 0) / 3000 * 16384 = 200 + 2.4576
                  // 15 MeV = 2.4576, but adc=llround(dE) and cut off is "<". So 3 here = 15.25MeV
                  .sampFrac = "1.00", // already taken care in DIGI code above
                  .readout  = "EcalEndcapPHits",
              },
          .level = JEventLevel::Timeslice},
      app));

//   app->Add(new JOmniFactoryGeneratorT<CalorimeterTruthClustering_factory>(
//       JOmniFactoryGeneratorT<CalorimeterTruthClustering_factory>::TypedWiring{
//           .m_tag                 = "EcalEndcapPTruthProtoClusters",
//           .m_default_input_tags  = {"EcalEndcapPRecHits", "EcalEndcapPHits"},
//           .m_default_output_tags = {"EcalEndcapPTruthProtoClusters"},
//           .m_default_cfg         = {},
//           .level                 = JEventLevel::Timeslice},
//       app));
//   app->Add(new JOmniFactoryGeneratorT<CalorimeterIslandCluster_factory>(
//       JOmniFactoryGeneratorT<CalorimeterIslandCluster_factory>::TypedWiring{
//           .m_tag                 = "EcalEndcapPIslandProtoClusters",
//           .m_default_input_tags  = {"EcalEndcapPRecHits"},
//           .m_default_output_tags = {"EcalEndcapPIslandProtoClusters"},
//           .m_default_cfg         = {.adjacencyMatrix{},
//                                     .peakNeighbourhoodMatrix{},
//                                     .readout{},
//                                     .sectorDist = 5.0 * dd4hep::cm,
//                                     .localDistXY{},
//                                     .localDistXZ{},
//                                     .localDistYZ{},
//                                     .globalDistRPhi{},
//                                     .globalDistEtaPhi{},
//                                     .dimScaledLocalDistXY          = {1.5, 1.5},
//                                     .splitCluster                  = false,
//                                     .minClusterHitEdep             = 0.0 * dd4hep::MeV,
//                                     .minClusterCenterEdep          = 60.0 * dd4hep::MeV,
//                                     .transverseEnergyProfileMetric = "dimScaledLocalDistXY",
//                                     .transverseEnergyProfileScale  = 1.,
//                                     .transverseEnergyProfileScaleUnits{}},
//           .level                 = JEventLevel::Timeslice},
//       app));

//   app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
//       JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>::TypedWiring{
//           .m_tag = "EcalEndcapPTruthClustersWithoutShapes",
//           .m_default_input_tags =
//               {
//                   "EcalEndcapPTruthProtoClusters", // edm4eic::ProtoClusterCollection
// #if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
//                   "EcalEndcapPRawHitLinks", // edm4eic::MCRecoCalorimeterHitLink
// #endif
//                   "EcalEndcapPRawHitAssociations" // edm4eic::MCRecoCalorimeterHitAssociationCollection
//               },
//           .m_default_output_tags = {"EcalEndcapPTruthClustersWithoutShapes",
// #if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
//                                     "EcalEndcapPTruthClusterLinksWithoutShapes",
// #endif
//                                     "EcalEndcapPTruthClusterAssociationsWithoutShapes"},
//           .m_default_cfg =
//               {
//                   .energyWeight    = "log",
//                   .sampFrac        = 1.0,
//                   .logWeightBase   = 6.2,
//                   .enableEtaBounds = true,
//               },
//           .level = JEventLevel::Timeslice},
//       app));

//   app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>(
//       JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>::TypedWiring{
//           .m_tag                 = "EcalEndcapPTruthClusters",
//           .m_default_input_tags  = {"EcalEndcapPTruthClustersWithoutShapes",
//                                     "EcalEndcapPTruthClusterAssociationsWithoutShapes"},
//           .m_default_output_tags = {"EcalEndcapPTruthClusters",
// #if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
//                                     "EcalEndcapPTruthClusterLinks",
// #endif
//                                     "EcalEndcapPTruthClusterAssociations"},
//           .m_default_cfg = {.energyWeight = "log", .logWeightBase = 6.2},
//           .level         = JEventLevel::Timeslice},
//       app));

//   app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
//       JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>::TypedWiring{
//           .m_tag = "EcalEndcapPClustersWithoutShapes",
//           .m_default_input_tags =
//               {
//                   "EcalEndcapPIslandProtoClusters", // edm4eic::ProtoClusterCollection
// #if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
//                   "EcalEndcapPRawHitLinks", // edm4eic::MCRecoCalorimeterHitLink
// #endif
//                   "EcalEndcapPRawHitAssociations" // edm4eic::MCRecoCalorimeterHitAssociationCollection
//               },
//           .m_default_output_tags = {"EcalEndcapPClustersWithoutShapes",
// #if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
//                                     "EcalEndcapPClusterLinksWithoutShapes",
// #endif
//                                     "EcalEndcapPClusterAssociationsWithoutShapes"},
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
//           .m_tag                 = "EcalEndcapPClusters",
//           .m_default_input_tags  = {"EcalEndcapPClustersWithoutShapes",
//                                     "EcalEndcapPClusterAssociationsWithoutShapes"},
//           .m_default_output_tags = {"EcalEndcapPClusters",
// #if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
//                                     "EcalEndcapPClusterLinks",
// #endif
//                                     "EcalEndcapPClusterAssociations"},
//           .m_default_cfg = {.energyWeight = "log", .logWeightBase = 3.6},
//           .level         = JEventLevel::Timeslice},
//       app));

//   app->Add(new JOmniFactoryGeneratorT<TrackClusterMergeSplitter_factory>(
//       JOmniFactoryGeneratorT<TrackClusterMergeSplitter_factory>::TypedWiring{
//           .m_tag                = "EcalEndcapPSplitMergeProtoClusters",
//           .m_default_input_tags = {"EcalEndcapPIslandProtoClusters", "CalorimeterTrackProjections"},
//           .m_default_output_tags = {"EcalEndcapPSplitMergeProtoClusters"},
//           .m_default_cfg         = {.idCalo                       = "EcalEndcapP_ID",
//                                     .minSigCut                    = -2.0,
//                                     .avgEP                        = 1.0,
//                                     .sigEP                        = 0.10,
//                                     .drAdd                        = 0.30,
//                                     .sampFrac                     = 1.0,
//                                     .transverseEnergyProfileScale = 1.0},
//           .level                 = JEventLevel::Timeslice},
//       app));

//   app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
//       JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>::TypedWiring{
//           .m_tag = "EcalEndcapPSplitMergeClustersWithoutShapes",
//           .m_default_input_tags =
//               {
//                   "EcalEndcapPSplitMergeProtoClusters", // edm4eic::ProtoClusterCollection
// #if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
//                   "EcalEndcapPRawHitLinks", // edm4eic::MCRecoCalorimeterHitLink
// #endif
//                   "EcalEndcapPRawHitAssociations" // edm4hep::MCRecoCalorimeterHitAssociationCollection
//               },
//           .m_default_output_tags = {"EcalEndcapPSplitMergeClustersWithoutShapes",
// #if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
//                                     "EcalEndcapPSplitMergeClusterLinksWithoutShapes",
// #endif
//                                     "EcalEndcapPSplitMergeClusterAssociationsWithoutShapes"},
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
//           .m_tag                 = "EcalEndcapPSplitMergeClusters",
//           .m_default_input_tags  = {"EcalEndcapPSplitMergeClustersWithoutShapes",
//                                     "EcalEndcapPSplitMergeClusterAssociationsWithoutShapes"},
//           .m_default_output_tags = {"EcalEndcapPSplitMergeClusters",
// #if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
//                                     "EcalEndcapPSplitMergeClusterLinks",
// #endif
//                                     "EcalEndcapPSplitMergeClusterAssociations"},
//           .m_default_cfg = {.energyWeight = "log", .logWeightBase = 3.6},
//           .level         = JEventLevel::Timeslice},
//       app));
}
// }
