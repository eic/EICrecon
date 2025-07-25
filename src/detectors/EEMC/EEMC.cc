// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2025 Sylvester Joosten, Chao, Chao Peng, Whitney Armstrong, Thomas Britton, David Lawrence, Dhevan Gangadharan, Wouter Deconinck, Dmitry Kalinkin, Derek Anderson

#include <Evaluator/DD4hepUnits.h>
#include <JANA/Components/JOmniFactoryGeneratorT.h>
#include <JANA/JApplication.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/Utils/JEventLevel.h>
#include <JANA/Utils/JTypeInfo.h>
#include <edm4eic/EDM4eicVersion.h>
#include <cmath>
#include <string>
#include <variant>
#include <vector>

#include "algorithms/calorimetry/CalorimeterHitDigiConfig.h"
#include "factories/calorimetry/CalorimeterClusterRecoCoG_factory.h"
#include "factories/calorimetry/CalorimeterHitDigi_factory.h"
#include "factories/calorimetry/CalorimeterHitReco_factory.h"
#include "factories/calorimetry/CalorimeterIslandCluster_factory.h"
#if EDM4EIC_VERSION_MAJOR >= 8
#include "factories/calorimetry/CalorimeterParticleIDPostML_factory.h"
#include "factories/calorimetry/CalorimeterParticleIDPreML_factory.h"
#endif
#include "factories/calorimetry/CalorimeterClusterShape_factory.h"
#include "factories/calorimetry/CalorimeterTruthClustering_factory.h"
#include "factories/calorimetry/TrackClusterMergeSplitter_factory.h"
#if EDM4EIC_VERSION_MAJOR >= 8
#include "factories/meta/ONNXInference_factory.h"
#endif

extern "C" {
void InitPlugin(JApplication* app) {

  using namespace eicrecon;
  using jana::components::JOmniFactoryGeneratorT;

  InitJANAPlugin(app);

  // Make sure digi and reco use the same value
  decltype(CalorimeterHitDigiConfig::capADC) EcalEndcapN_capADC         = 16384; //65536,  16bit ADC
  decltype(CalorimeterHitDigiConfig::dyRangeADC) EcalEndcapN_dyRangeADC = 20.0 * dd4hep::GeV;
  decltype(CalorimeterHitDigiConfig::pedMeanADC) EcalEndcapN_pedMeanADC = 20;
  decltype(CalorimeterHitDigiConfig::pedSigmaADC) EcalEndcapN_pedSigmaADC = 1;
  decltype(CalorimeterHitDigiConfig::resolutionTDC) EcalEndcapN_resolutionTDC =
      10 * dd4hep::picosecond;
  app->Add(new JOmniFactoryGeneratorT<CalorimeterHitDigi_factory>(
      "EcalEndcapNRawHits", {"EventHeader", "EcalEndcapNHits"},
      {"EcalEndcapNRawHits", "EcalEndcapNRawHitAssociations"},
      {
          .eRes        = {0.0 * sqrt(dd4hep::GeV), 0.0, 0.0 * dd4hep::GeV},
          .tRes        = 0.0 * dd4hep::ns,
          .threshold   = 0.0 * dd4hep::MeV, // Use ADC cut instead
          .readoutType = "sipm",
          .lightYield  = 300. / dd4hep::MeV,
          // See simulation study by A. Hoghmrtsyan https://indico.bnl.gov/event/20415/
          // This includes quantum efficiency of the SiPM
          .photonDetectionEfficiency = 17. / 300.,
          // S14160-6015PS, 4 sensors per cell
          .numEffectiveSipmPixels = 159565 * 4,
          .capADC                 = EcalEndcapN_capADC,
          .dyRangeADC             = EcalEndcapN_dyRangeADC,
          .pedMeanADC             = EcalEndcapN_pedMeanADC,
          .pedSigmaADC            = EcalEndcapN_pedSigmaADC,
          .resolutionTDC          = EcalEndcapN_resolutionTDC,
          .corrMeanScale          = "1.0",
          .readout                = "EcalEndcapNHits",
      }));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterHitReco_factory>(
      "EcalEndcapNRecHits", {"EcalEndcapNRawHits"}, {"EcalEndcapNRecHits"},
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
      }));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterTruthClustering_factory>(
      "EcalEndcapNTruthProtoClusters", {"EcalEndcapNRecHits", "EcalEndcapNHits"},
      {"EcalEndcapNTruthProtoClusters"}));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterIslandCluster_factory>(
      "EcalEndcapNIslandProtoClusters", {"EcalEndcapNRecHits"}, {"EcalEndcapNIslandProtoClusters"},
      {
          .adjacencyMatrix         = "(abs(row_1 - row_2) + abs(column_1 - column_2)) == 1",
          .peakNeighbourhoodMatrix = "max(abs(row_1 - row_2), abs(column_1 - column_2)) == 1",
          .readout                 = "EcalEndcapNHits",
          .sectorDist              = 5.0 * dd4hep::cm,
          .localDistXY{},
          .localDistXZ{},
          .localDistYZ{},
          .globalDistRPhi{},
          .globalDistEtaPhi{},
          .dimScaledLocalDistXY{},
          .splitCluster                  = true,
          .minClusterHitEdep             = 1.0 * dd4hep::MeV,
          .minClusterCenterEdep          = 30.0 * dd4hep::MeV,
          .transverseEnergyProfileMetric = "globalDistEtaPhi",
          .transverseEnergyProfileScale  = 0.08,
          .transverseEnergyProfileScaleUnits{},
      }));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
      "EcalEndcapNTruthClustersWithoutShapes",
      {
          "EcalEndcapNTruthProtoClusters", // edm4eic::ProtoClusterCollection
          "EcalEndcapNRawHitAssociations"  // edm4eic::MCRecoCalorimeterHitAssociationCollection
      },
      {"EcalEndcapNTruthClustersWithoutShapes",             // edm4eic::Cluster
       "EcalEndcapNTruthClusterAssociationsWithoutShapes"}, // edm4eic::MCRecoClusterParticleAssociation
      {.energyWeight = "log", .sampFrac = 1.0, .logWeightBase = 4.6, .enableEtaBounds = false}));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>(
      "EcalEndcapNTruthClusters",
      {"EcalEndcapNTruthClustersWithoutShapes", "EcalEndcapNTruthClusterAssociationsWithoutShapes"},
      {"EcalEndcapNTruthClusters", "EcalEndcapNTruthClusterAssociations"},
      {.energyWeight = "log", .logWeightBase = 4.6}));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
#if EDM4EIC_VERSION_MAJOR >= 8
      "EcalEndcapNClustersWithoutPIDAndShapes",
#else
      "EcalEndcapNClustersWithoutShapes",
#endif
      {
          "EcalEndcapNIslandProtoClusters", // edm4eic::ProtoClusterCollection
          "EcalEndcapNRawHitAssociations"   // edm4eic::MCRecoCalorimeterHitAssociationCollection
      },
#if EDM4EIC_VERSION_MAJOR >= 8
      {"EcalEndcapNClustersWithoutPIDAndShapes",             // edm4eic::Cluster
       "EcalEndcapNClusterAssociationsWithoutPIDAndShapes"}, // edm4eic::MCRecoClusterParticleAssociation
#else
      {"EcalEndcapNClustersWithoutShapes",             // edm4eic::Cluster
       "EcalEndcapNClusterAssociationsWithoutShapes"}, // edm4eic::MCRecoClusterParticleAssociation
#endif
      {
          .energyWeight    = "log",
          .sampFrac        = 1.0,
          .logWeightBase   = 3.6,
          .enableEtaBounds = false,
      }));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>(
#if EDM4EIC_VERSION_MAJOR >= 8
      "EcalEndcapNClustersWithoutPID",
      {"EcalEndcapNClustersWithoutPIDAndShapes",
       "EcalEndcapNClusterAssociationsWithoutPIDAndShapes"},
      {"EcalEndcapNClustersWithoutPID", "EcalEndcapNClusterAssociationsWithoutPID"},
#else
      "EcalEndcapNClusters",
      {"EcalEndcapNClustersWithoutShapes", "EcalEndcapNClusterAssociationsWithoutShapes"},
      {"EcalEndcapNClusters", "EcalEndcapNClusterAssociations"},
#endif
      {.energyWeight = "log", .logWeightBase = 3.6}));

  app->Add(new JOmniFactoryGeneratorT<TrackClusterMergeSplitter_factory>(
      "EcalEndcapNSplitMergeProtoClusters",
      {"EcalEndcapNIslandProtoClusters", "CalorimeterTrackProjections"},
      {"EcalEndcapNSplitMergeProtoClusters"},
      {.idCalo                       = "EcalEndcapN_ID",
       .minSigCut                    = -1.0,
       .avgEP                        = 1.0,
       .sigEP                        = 0.10,
       .drAdd                        = 0.08,
       .sampFrac                     = 1.0,
       .transverseEnergyProfileScale = 1.0}));

#if EDM4EIC_VERSION_MAJOR >= 8
  app->Add(new JOmniFactoryGeneratorT<CalorimeterParticleIDPreML_factory>(
      "EcalEndcapNParticleIDPreML",
      {
          "EcalEndcapNClustersWithoutPID",
          "EcalEndcapNClusterAssociationsWithoutPID",
      },
      {
          "EcalEndcapNParticleIDInput_features",
          "EcalEndcapNParticleIDTarget",
      }));
  app->Add(new JOmniFactoryGeneratorT<ONNXInference_factory>(
      "EcalEndcapNParticleIDInference",
      {
          "EcalEndcapNParticleIDInput_features",
      },
      {
          "EcalEndcapNParticleIDOutput_label",
          "EcalEndcapNParticleIDOutput_probability_tensor",
      },
      {
          .modelPath = "calibrations/onnx/EcalEndcapN_pi_rejection.onnx",
      }));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterParticleIDPostML_factory>(
      "EcalEndcapNParticleIDPostML",
      {
          "EcalEndcapNClustersWithoutPID",
          "EcalEndcapNClusterAssociationsWithoutPID",
          "EcalEndcapNParticleIDOutput_probability_tensor",
      },
      {
          "EcalEndcapNClusters",
          "EcalEndcapNClusterAssociations",
          "EcalEndcapNClusterParticleIDs",
      }));
#endif

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
      "EcalEndcapNSplitMergeClustersWithoutShapes",
      {
          "EcalEndcapNSplitMergeProtoClusters", // edm4eic::ProtoClusterCollection
          "EcalEndcapNRawHitAssociations" // edm4hep::MCRecoCalorimeterHitAssociationCollection
      },
      {"EcalEndcapNSplitMergeClustersWithoutShapes",             // edm4eic::Cluster
       "EcalEndcapNSplitMergeClusterAssociationsWithoutShapes"}, // edm4eic::MCRecoClusterParticleAssociation
      {.energyWeight = "log", .sampFrac = 1.0, .logWeightBase = 3.6, .enableEtaBounds = false}));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>(
      "EcalEndcapNSplitMergeClusters",
      {"EcalEndcapNSplitMergeClustersWithoutShapes",
       "EcalEndcapNSplitMergeClusterAssociationsWithoutShapes"},
      {"EcalEndcapNSplitMergeClusters", "EcalEndcapNSplitMergeClusterAssociations"},
      {.energyWeight = "log", .logWeightBase = 3.6}));
}
}
