// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2025 Sylvester Joosten, Chao, Chao Peng, Whitney Armstrong, David Lawrence, Friederike Bock, Nathan Brei, Wouter Deconinck, Dmitry Kalinkin, Derek Anderson

#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplicationFwd.h>
#include <string>
#include <variant>

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

extern "C" {
void InitPlugin(JApplication* app) {

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
      "HcalEndcapNRawHits", {"HcalEndcapNHits"},
      {"HcalEndcapNRawHits", "HcalEndcapNRawHitAssociations"},
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
      app // TODO: Remove me once fixed
      ));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterHitReco_factory>(
      "HcalEndcapNRecHits", {"HcalEndcapNRawHits"}, {"HcalEndcapNRecHits"},
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
      app // TODO: Remove me once fixed
      ));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterHitsMerger_factory>(
      "HcalEndcapNMergedHits", {"HcalEndcapNRecHits"}, {"HcalEndcapNMergedHits"},
      {.readout = "HcalEndcapNHits", .fieldTransformations = {"layer:4", "slice:0"}},
      app // TODO: Remove me once fixed
      ));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterTruthClustering_factory>(
      "HcalEndcapNTruthProtoClusters", {"HcalEndcapNMergedHits", "HcalEndcapNHits"},
      {"HcalEndcapNTruthProtoClusters"},
      app // TODO: Remove me once fixed
      ));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterIslandCluster_factory>(
      "HcalEndcapNIslandProtoClusters", {"HcalEndcapNMergedHits"},
      {"HcalEndcapNIslandProtoClusters"},
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
      app // TODO: Remove me once fixed
      ));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
      "HcalEndcapNTruthClustersWithoutShapes",
      {
          "HcalEndcapNTruthProtoClusters", // edm4eic::ProtoClusterCollection
          "HcalEndcapNRawHitAssociations"  // edm4eic::MCRecoCalorimeterHitAssociationCollection
      },
      {"HcalEndcapNTruthClustersWithoutShapes",             // edm4eic::Cluster
       "HcalEndcapNTruthClusterAssociationsWithoutShapes"}, // edm4eic::MCRecoClusterParticleAssociation
      {.energyWeight = "log", .sampFrac = 1.0, .logWeightBase = 6.2, .enableEtaBounds = false},
      app // TODO: Remove me once fixed
      ));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>(
      "HcalEndcapNTruthClusters",
      {"HcalEndcapNTruthClustersWithoutShapes", "HcalEndcapNTruthClusterAssociationsWithoutShapes"},
      {"HcalEndcapNTruthClusters", "HcalEndcapNTruthClusterAssociations"},
      {.energyWeight = "log", .logWeightBase = 6.2}, app));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
      "HcalEndcapNClustersWithoutShapes",
      {
          "HcalEndcapNIslandProtoClusters", // edm4eic::ProtoClusterCollection
          "HcalEndcapNRawHitAssociations"   // edm4eic::MCRecoCalorimeterHitAssociationCollection
      },
      {"HcalEndcapNClustersWithoutShapes",             // edm4eic::Cluster
       "HcalEndcapNClusterAssociationsWithoutShapes"}, // edm4eic::MCRecoClusterParticleAssociation
      {
          .energyWeight    = "log",
          .sampFrac        = 1.0,
          .logWeightBase   = 6.2,
          .enableEtaBounds = false,
      },
      app // TODO: Remove me once fixed
      ));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>(
      "HcalEndcapNClusters",
      {"HcalEndcapNClustersWithoutShapes", "HcalEndcapNClusterAssociationsWithoutShapes"},
      {"HcalEndcapNClusters", "HcalEndcapNClusterAssociations"},
      {.energyWeight = "log", .logWeightBase = 6.2}, app));
  app->Add(new JOmniFactoryGeneratorT<TrackClusterMergeSplitter_factory>(
      "HcalEndcapNSplitMergeProtoClusters",
      {"HcalEndcapNIslandProtoClusters", "CalorimeterTrackProjections"},
      {"HcalEndcapNSplitMergeProtoClusters"},
      {.idCalo                       = "HcalEndcapN_ID",
       .minSigCut                    = -2.0,
       .avgEP                        = 0.60,
       .sigEP                        = 0.40,
       .drAdd                        = 0.40,
       .sampFrac                     = 1.0,
       .transverseEnergyProfileScale = 1.0},
      app // TODO: remove me once fixed
      ));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
      "HcalEndcapNSplitMergeClustersWithoutShapes",
      {
          "HcalEndcapNSplitMergeProtoClusters", // edm4eic::ProtoClusterCollection
          "HcalEndcapNRawHitAssociations" // edm4hep::MCRecoCalorimeterHitAssociationCollection
      },
      {"HcalEndcapNSplitMergeClustersWithoutShapes",             // edm4eic::Cluster
       "HcalEndcapNSplitMergeClusterAssociationsWithoutShapes"}, // edm4eic::MCRecoClusterParticleAssociation
      {.energyWeight = "log", .sampFrac = 1.0, .logWeightBase = 6.2, .enableEtaBounds = false},
      app // TODO: Remove me once fixed
      ));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>(
      "HcalEndcapNSplitMergeClusters",
      {"HcalEndcapNSplitMergeClustersWithoutShapes",
       "HcalEndcapNSplitMergeClusterAssociationsWithoutShapes"},
      {"HcalEndcapNSplitMergeClusters", "HcalEndcapNSplitMergeClusterAssociations"},
      {.energyWeight = "log", .logWeightBase = 6.2}, app));
}
}
