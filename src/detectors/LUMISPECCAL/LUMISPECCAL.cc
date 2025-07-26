// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2025 Sylvester Joosten, Chao, Chao Peng, Whitney Armstrong, David Lawrence, Dhevan Gangadharan, Nathan Brei,, Wouter Deconinck, Dmitry Kalinkin, Derek Anderson

#include <Evaluator/DD4hepUnits.h>
#include <JANA/Components/JOmniFactoryGeneratorT.h>
#include <JANA/JApplication.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/Utils/JEventLevel.h>
#include <JANA/Utils/JTypeInfo.h>
#include <cmath>
#include <string>
#include <variant>
#include <vector>

#include "factories/calorimetry/CalorimeterClusterRecoCoG_factory.h"
#include "factories/calorimetry/CalorimeterClusterShape_factory.h"
#include "factories/calorimetry/CalorimeterHitDigi_factory.h"
#include "factories/calorimetry/CalorimeterHitReco_factory.h"
#include "factories/calorimetry/CalorimeterIslandCluster_factory.h"
#include "factories/calorimetry/CalorimeterTruthClustering_factory.h"

extern "C" {
void InitPlugin(JApplication* app) {

  using namespace eicrecon;
  using jana::components::JOmniFactoryGeneratorT;

  InitJANAPlugin(app);

  app->Add(new JOmniFactoryGeneratorT<CalorimeterHitDigi_factory>(
      "EcalLumiSpecRawHits", {"EventHeader", "EcalLumiSpecHits"},
      {"EcalLumiSpecRawHits", "EcalLumiSpecRawHitAssociations"},
      {
          .eRes          = {0.0 * sqrt(dd4hep::GeV), 0.02, 0.0 * dd4hep::GeV}, // flat 2%
          .tRes          = 0.0 * dd4hep::ns,
          .capADC        = 16384,
          .dyRangeADC    = 20 * dd4hep::GeV,
          .pedMeanADC    = 100,
          .pedSigmaADC   = 1,
          .resolutionTDC = 10 * dd4hep::picosecond,
          .corrMeanScale = "1.0",
          .readout       = "EcalLumiSpecHits",
      }));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterHitReco_factory>(
      "EcalLumiSpecRecHits", {"EcalLumiSpecRawHits"}, {"EcalLumiSpecRecHits"},
      {
          .capADC          = 16384,
          .dyRangeADC      = 20. * dd4hep::GeV,
          .pedMeanADC      = 100,
          .pedSigmaADC     = 1,
          .resolutionTDC   = 10 * dd4hep::picosecond,
          .thresholdFactor = 0.0,
          .thresholdValue  = 2.0,
          .sampFrac        = "1.0",
          .readout         = "EcalLumiSpecHits",
      }));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterTruthClustering_factory>(
      "EcalLumiSpecTruthProtoClusters", {"EcalLumiSpecRecHits", "EcalLumiSpecHits"},
      {"EcalLumiSpecTruthProtoClusters"}));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterIslandCluster_factory>(
      "EcalLumiSpecIslandProtoClusters", {"EcalLumiSpecRecHits"},
      {"EcalLumiSpecIslandProtoClusters"},
      {
          .adjacencyMatrix =
              "(sector_1 == sector_2) && ((abs(floor(module_1 / 10) - floor(module_2 / 10)) + "
              "abs(fmod(module_1, 10) - fmod(module_2, 10))) == 1)",
          .peakNeighbourhoodMatrix{},
          .readout    = "EcalLumiSpecHits",
          .sectorDist = 0.0 * dd4hep::cm,
          .localDistXY{},
          .localDistXZ{},
          .localDistYZ{},
          .globalDistRPhi{},
          .globalDistEtaPhi{},
          .dimScaledLocalDistXY{},
          .splitCluster                  = true,
          .minClusterHitEdep             = 1.0 * dd4hep::MeV,
          .minClusterCenterEdep          = 30.0 * dd4hep::MeV,
          .transverseEnergyProfileMetric = "localDistXY",
          .transverseEnergyProfileScale  = 10. * dd4hep::mm,
          .transverseEnergyProfileScaleUnits{},
      }));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
      "EcalLumiSpecClustersWithoutShapes",
      {
          "EcalLumiSpecIslandProtoClusters", // edm4eic::ProtoClusterCollection
          "EcalLumiSpecRawHitAssociations"   // edm4eic::MCRecoCalorimeterHitAssociationCollection
      },
      {"EcalLumiSpecClustersWithoutShapes",             // edm4eic::Cluster
       "EcalLumiSpecClusterAssociationsWithoutShapes"}, // edm4eic::MCRecoClusterParticleAssociation
      {.energyWeight = "log", .sampFrac = 1.0, .logWeightBase = 3.6, .enableEtaBounds = false}));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>(
      "EcalLumiSpecClusters",
      {"EcalLumiSpecClustersWithoutShapes", "EcalLumiSpecClusterAssociationsWithoutShapes"},
      {"EcalLumiSpecClusters", "EcalLumiSpecClusterAssociations"},
      {.energyWeight = "log", .logWeightBase = 3.6}));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
      "EcalLumiSpecTruthClustersWithoutShapes",
      {
          "EcalLumiSpecTruthProtoClusters", // edm4eic::ProtoClusterCollection
          "EcalLumiSpecRawHitAssociations"  // edm4eic::MCRecoCalorimeterHitAssociationCollection
      },
      {"EcalLumiSpecTruthClustersWithoutShapes",             // edm4eic::Cluster
       "EcalLumiSpecTruthClusterAssociationsWithoutShapes"}, // edm4eic::MCRecoClusterParticleAssociation
      {.energyWeight = "log", .sampFrac = 1.0, .logWeightBase = 4.6, .enableEtaBounds = false}));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>(
      "EcalLumiSpecTruthClusters",
      {"EcalLumiSpecTruthClustersWithoutShapes",
       "EcalLumiSpecTruthClusterAssociationsWithoutShapes"},
      {"EcalLumiSpecTruthClusters", "EcalLumiSpecTruthClusterAssociations"},
      {.energyWeight = "log", .logWeightBase = 4.6}));
}
}
