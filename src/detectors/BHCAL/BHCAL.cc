// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2024 David Lawrence, Derek Anderson, Wouter Deconinck

#include <Evaluator/DD4hepUnits.h>
#include <JANA/Components/JOmniFactoryGeneratorT.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/Utils/JTypeInfo.h>
#include <memory>
#include <string>
#include <variant>
#include <vector>

#include "algorithms/calorimetry/CalorimeterHitDigiConfig.h"
#include "algorithms/calorimetry/CalorimeterIslandClusterConfig.h"
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
  using jana::components::JOmniFactoryGeneratorT;

  InitJANAPlugin(app);

  // Make sure digi and reco use the same value
  decltype(CalorimeterHitDigiConfig::capADC) HcalBarrel_capADC         = 65536; //65536,  16bit ADC
  decltype(CalorimeterHitDigiConfig::dyRangeADC) HcalBarrel_dyRangeADC = 1.0 * dd4hep::GeV;
  decltype(CalorimeterHitDigiConfig::pedMeanADC) HcalBarrel_pedMeanADC = 300;
  decltype(CalorimeterHitDigiConfig::pedSigmaADC) HcalBarrel_pedSigmaADC = 2;
  decltype(CalorimeterHitDigiConfig::resolutionTDC) HcalBarrel_resolutionTDC =
      1 * dd4hep::picosecond;

  // Set default adjacency matrix. Magic constants:
  //  320 - number of tiles per row
  decltype(CalorimeterIslandClusterConfig::adjacencyMatrix) HcalBarrel_adjacencyMatrix =
      "("
      // check for vertically adjacent tiles
      "  ( (abs(eta_1 - eta_2) == 1) && (abs(phi_1 - phi_2) == 0) ) ||"
      // check for horizontally adjacent tiles
      "  ( (abs(eta_1 - eta_2) == 0) && (abs(phi_1 - phi_2) == 1) ) ||"
      // check for horizontally adjacent tiles at wraparound
      "  ( (abs(eta_1 - eta_2) == 0) && (abs(phi_1 - phi_2) == (320 - 1)) )"
      ") == 1";

  app->Add(new JOmniFactoryGeneratorT<CalorimeterHitDigi_factory>(
      "HcalBarrelRawHits", {"EventHeader", "HcalBarrelHits"},
      {"HcalBarrelRawHits", "HcalBarrelRawHitAssociations"},
      {
          .eRes          = {},
          .tRes          = 0.0 * dd4hep::ns,
          .threshold     = 0.0, // Use ADC cut instead
          .capADC        = HcalBarrel_capADC,
          .capTime       = 100, // given in ns, 4 samples in HGCROC
          .dyRangeADC    = HcalBarrel_dyRangeADC,
          .pedMeanADC    = HcalBarrel_pedMeanADC,
          .pedSigmaADC   = HcalBarrel_pedSigmaADC,
          .resolutionTDC = HcalBarrel_resolutionTDC,
          .corrMeanScale = "1.0",
          .readout       = "HcalBarrelHits",
      }));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterHitReco_factory>(
      "HcalBarrelRecHits", {"HcalBarrelRawHits"}, {"HcalBarrelRecHits"},
      {
          .capADC          = HcalBarrel_capADC,
          .dyRangeADC      = HcalBarrel_dyRangeADC,
          .pedMeanADC      = HcalBarrel_pedMeanADC,
          .pedSigmaADC     = HcalBarrel_pedSigmaADC, // not used; relying on energy cut
          .resolutionTDC   = HcalBarrel_resolutionTDC,
          .thresholdFactor = 0.0,     // not used; relying on flat ADC cut
          .thresholdValue  = 33,      // pedSigmaADC + thresholdValue = half-MIP (333 ADC)
          .sampFrac        = "0.033", // average, from sPHENIX simulations
          .readout         = "HcalBarrelHits",
          .layerField      = "",
          .sectorField     = "",
      }));

  // --------------------------------------------------------------------
  // If needed, merge adjacent phi tiles into towers. By default,
  // NO merging will be done. This can be changed at runtime.
  // --------------------------------------------------------------------
  app->Add(new JOmniFactoryGeneratorT<CalorimeterHitsMerger_factory>(
      "HcalBarrelMergedHits", {"HcalBarrelRecHits"}, {"HcalBarrelMergedHits"},
      {.readout = "HcalBarrelHits", .fieldTransformations = {"phi:phi"}}));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterTruthClustering_factory>(
      "HcalBarrelTruthProtoClusters", {"HcalBarrelRecHits", "HcalBarrelHits"},
      {"HcalBarrelTruthProtoClusters"}));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterIslandCluster_factory>(
      "HcalBarrelIslandProtoClusters", {"HcalBarrelRecHits"}, {"HcalBarrelIslandProtoClusters"},
      {.adjacencyMatrix = HcalBarrel_adjacencyMatrix,
       .peakNeighbourhoodMatrix{},
       .readout    = "HcalBarrelHits",
       .sectorDist = 5.0 * dd4hep::cm,
       .localDistXY{},
       .localDistXZ{},
       .localDistYZ{},
       .globalDistRPhi{},
       .globalDistEtaPhi{},
       .dimScaledLocalDistXY{},
       .splitCluster                  = false,
       .minClusterHitEdep             = 5.0 * dd4hep::MeV,
       .minClusterCenterEdep          = 30.0 * dd4hep::MeV,
       .transverseEnergyProfileMetric = "globalDistEtaPhi",
       .transverseEnergyProfileScale  = 1.,
       .transverseEnergyProfileScaleUnits{}}));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
      "HcalBarrelClustersWithoutShapes",
      {
          "HcalBarrelIslandProtoClusters", // edm4eic::ProtoClusterCollection
          "HcalBarrelRawHitAssociations"   // edm4eic::MCRecoCalorimeterHitAssociationCollection
      },
      {"HcalBarrelClustersWithoutShapes",             // edm4eic::Cluster
       "HcalBarrelClusterAssociationsWithoutShapes"}, // edm4eic::MCRecoClusterParticleAssociation
      {.energyWeight = "log", .sampFrac = 1.0, .logWeightBase = 6.2, .enableEtaBounds = false}));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>(
      "HcalBarrelClusters",
      {"HcalBarrelClustersWithoutShapes", "HcalBarrelClusterAssociationsWithoutShapes"},
      {"HcalBarrelClusters", "HcalBarrelClusterAssociations"},
      {.energyWeight = "log", .logWeightBase = 6.2}));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
      "HcalBarrelTruthClustersWithoutShapes",
      {
          "HcalBarrelTruthProtoClusters", // edm4eic::ProtoClusterCollection
          "HcalBarrelRawHitAssociations"  // edm4eic::MCRecoCalorimeterHitAssociationCollection
      },
      {"HcalBarrelTruthClustersWithoutShapes",             // edm4eic::Cluster
       "HcalBarrelTruthClusterAssociationsWithoutShapes"}, // edm4eic::MCRecoClusterParticleAssociation
      {.energyWeight = "log", .sampFrac = 1.0, .logWeightBase = 6.2, .enableEtaBounds = false}));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>(
      "HcalBarrelTruthClusters",
      {"HcalBarrelTruthClustersWithoutShapes", "HcalBarrelTruthClusterAssociationsWithoutShapes"},
      {"HcalBarrelTruthClusters", "HcalBarrelTruthClusterAssociations"},
      {.energyWeight = "log", .logWeightBase = 6.2}));

  app->Add(new JOmniFactoryGeneratorT<TrackClusterMergeSplitter_factory>(
      "HcalBarrelSplitMergeProtoClusters",
      {"HcalBarrelIslandProtoClusters", "CalorimeterTrackProjections"},
      {"HcalBarrelSplitMergeProtoClusters"},
      {.idCalo                       = "HcalBarrel_ID",
       .minSigCut                    = -2.0,
       .avgEP                        = 0.50,
       .sigEP                        = 0.25,
       .drAdd                        = 0.40,
       .sampFrac                     = 1.0,
       .transverseEnergyProfileScale = 1.0}));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
      "HcalBarrelSplitMergeClustersWithoutShapes",
      {
          "HcalBarrelSplitMergeProtoClusters", // edm4eic::ProtoClusterCollection
          "HcalBarrelRawHitAssociations"       // edm4eic::MCRecoCalorimeterHitAssociationCollection
      },
      {"HcalBarrelSplitMergeClustersWithoutShapes",             // edm4eic::Cluster
       "HcalBarrelSplitMergeClusterAssociationsWithoutShapes"}, // edm4eic::MCRecoClusterParticleAssociation
      {.energyWeight = "log", .sampFrac = 1.0, .logWeightBase = 6.2, .enableEtaBounds = false}));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>(
      "HcalBarrelSplitMergeClusters",
      {"HcalBarrelSplitMergeClustersWithoutShapes",
       "HcalBarrelSplitMergeClusterAssociationsWithoutShapes"},
      {"HcalBarrelSplitMergeClusters", "HcalBarrelSplitMergeClusterAssociations"}, {}));
}
}
