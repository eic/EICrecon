// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplicationFwd.h>
#include <cmath>
#include <edm4eic/EDM4eicVersion.h>
#include <string>

#include "algorithms/calorimetry/CalorimeterHitDigiConfig.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/calorimetry/CalorimeterClusterRecoCoG_factory.h"
#include "factories/calorimetry/CalorimeterHitDigi_factory.h"
#include "factories/calorimetry/CalorimeterHitReco_factory.h"
#include "factories/calorimetry/CalorimeterIslandCluster_factory.h"
#if EDM4EIC_VERSION_MAJOR >= 8
#include "factories/calorimetry/CalorimeterParticleIDPostML_factory.h"
#include "factories/calorimetry/CalorimeterParticleIDPreML_factory.h"
#endif
#include "factories/calorimetry/CalorimeterTruthClustering_factory.h"
#include "factories/calorimetry/CalorimeterClusterShape_factory.h"
#include "factories/calorimetry/TrackClusterMergeSplitter_factory.h"
#if EDM4EIC_VERSION_MAJOR >= 8
#include "factories/meta/ONNXInference_factory.h"
#endif

extern "C" {
void InitPlugin(JApplication* app) {

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
      "EcalEndcapNRawHits", {"EcalEndcapNHits"},
#if EDM4EIC_VERSION_MAJOR >= 7
      {"EcalEndcapNRawHits", "EcalEndcapNRawHitAssociations"},
#else
      {"EcalEndcapNRawHits"},
#endif
      {
          .eRes          = {0.0 * sqrt(dd4hep::GeV), 0.0, 0.0 * dd4hep::GeV},
          .tRes          = 0.0 * dd4hep::ns,
          .threshold     = 0.0 * dd4hep::MeV, // Use ADC cut instead
          .capADC        = EcalEndcapN_capADC,
          .dyRangeADC    = EcalEndcapN_dyRangeADC,
          .pedMeanADC    = EcalEndcapN_pedMeanADC,
          .pedSigmaADC   = EcalEndcapN_pedSigmaADC,
          .resolutionTDC = EcalEndcapN_resolutionTDC,
          .corrMeanScale = "1.0",
          .readout       = "EcalEndcapNHits",
      },
      app // TODO: Remove me once fixed
      ));
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
      },
      app // TODO: Remove me once fixed
      ));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterTruthClustering_factory>(
      "EcalEndcapNTruthProtoClusters", {"EcalEndcapNRecHits", "EcalEndcapNHits"},
      {"EcalEndcapNTruthProtoClusters"},
      app // TODO: Remove me once fixed
      ));
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
      },
      app // TODO: Remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
      "EcalEndcapNTruthClustersWithoutShapes",
      {
        "EcalEndcapNTruthProtoClusters", // edm4eic::ProtoClusterCollection
#if EDM4EIC_VERSION_MAJOR >= 7
            "EcalEndcapNRawHitAssociations"
      }, // edm4eic::MCRecoCalorimeterHitAssociationCollection
#else
            "EcalEndcapNHits"
      }, // edm4hep::SimCalorimeterHitCollection
#endif
      {"EcalEndcapNTruthClustersWithoutShapes",             // edm4eic::Cluster
       "EcalEndcapNTruthClusterAssociationsWithoutShapes"}, // edm4eic::MCRecoClusterParticleAssociation
      {.energyWeight = "log", .sampFrac = 1.0, .logWeightBase = 4.6, .enableEtaBounds = false},
      app // TODO: Remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>(
      "EcalEndcapNTruthClusters",
      {"EcalEndcapNTruthClustersWithoutShapes", "EcalEndcapNTruthClusterAssociationsWithoutShapes"},
      {"EcalEndcapNTruthClusters", "EcalEndcapNTruthClusterAssociations"},
      {.energyWeight = "log", .logWeightBase = 4.6}, app));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
#if EDM4EIC_VERSION_MAJOR >= 8
      "EcalEndcapNClustersWithoutPIDAndShapes",
#else
      "EcalEndcapNClustersWithoutShapes",
#endif
      {
        "EcalEndcapNIslandProtoClusters", // edm4eic::ProtoClusterCollection
#if EDM4EIC_VERSION_MAJOR >= 7
            "EcalEndcapNRawHitAssociations"
      }, // edm4eic::MCRecoCalorimeterHitAssociationCollection
#else
            "EcalEndcapNHits"
      },                                               // edm4hep::SimCalorimeterHitCollection
#endif
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
      },
      app // TODO: Remove me once fixed
      ));

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
      {.energyWeight = "log", .logWeightBase = 3.6}, app));

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
      },
      app));
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
      },
      app));
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
      },
      app));
#endif

  app->Add(new JOmniFactoryGeneratorT<TrackClusterMergeSplitter_factory>(
      "EcalEndcapNSplitMergeProtoClusters",
      {"EcalEndcapNTrackClusterMatches", "EcalEndcapNClustersWithoutPID", "CalorimeterTrackProjections"},
      {
        "EcalEndcapNSplitMergeProtoClusters",
#if EDM4EIC_VERSION_MAJOR >= 8 && EDM4EIC_VERSION_MINOR >= 4
            "EcalEndcapNTrackSplitMergeProtoClusterMatches"
      },
#endif
      {.minSigCut                    = -1.0,
       .avgEP                        = 1.0,
       .sigEP                        = 0.10,
       .drAdd                        = 0.08,
       .surfaceToUse                 = 1,
       .transverseEnergyProfileScale = 1.0},
      app // TODO: remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
      "EcalEndcapNSplitMergeClustersWithoutShapes",
      {
        "EcalEndcapNSplitMergeProtoClusters",
#if EDM4EIC_VERSION_MAJOR >= 7
            "EcalEndcapNRawHitAssociations"
      },
#else
                  "EcalEndcapNHits"
            },
#endif
      {"EcalEndcapNSplitMergeClustersWithoutShapes",
       "EcalEndcapNSplitMergeClusterAssociationsWithoutShapes"},
      {
          .energyWeight    = "log",
          .sampFrac        = 1.0,
          .logWeightBase   = 3.6,
          .enableEtaBounds = false,
      },
      app // TODO: Remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>(
      "EcalEndcapNSplitMergeClusters",
      {"EcalEndcapNSplitMergeClustersWithoutShapes",
       "EcalEndcapNSplitMergeClusterAssociationsWithoutShapes"},
      {"EcalEndcapNSplitMergeClusters", "EcalEndcapNSplitMergeClusterAssociations"},
      {.energyWeight = "log", .logWeightBase = 3.6}, app));
}
}
