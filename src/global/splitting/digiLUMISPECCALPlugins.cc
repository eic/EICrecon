// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2025 Sylvester Joosten, Chao, Chao Peng, Whitney Armstrong, David Lawrence, Dhevan Gangadharan, Nathan Brei,, Wouter Deconinck, Dmitry Kalinkin, Derek Anderson

#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplicationFwd.h>
#include <edm4eic/EDM4eicVersion.h>
#include <JANA/Utils/JTypeInfo.h>
#include <cmath>
#include <string>
#include <variant>
#include <vector>

#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/calorimetry/CalorimeterClusterRecoCoG_factory.h"
#include "factories/calorimetry/CalorimeterClusterShape_factory.h"
#include "factories/calorimetry/CalorimeterHitDigi_factory.h"
#include "factories/calorimetry/CalorimeterHitReco_factory.h"
#include "factories/calorimetry/CalorimeterIslandCluster_factory.h"
#include "factories/calorimetry/CalorimeterTruthClustering_factory.h"

// extern "C" {
void InitPlugin_digiLUMISPECCAL(JApplication* app) {

  using namespace eicrecon;

  InitJANAPlugin(app);

  app->Add(new JOmniFactoryGeneratorT<CalorimeterHitDigi_factory>(
      "EcalLumiSpecRawHits_TK", {"EventHeader", "EcalLumiSpecHits"},
      {"EcalLumiSpecRawHits_TK",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
       "EcalLumiSpecRawHitLinks_TK",
#endif
       "EcalLumiSpecRawHitAssociations_TK"},
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
      },
      app // TODO: Remove me once fixed
      ));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterHitReco_factory>(
      "EcalLumiSpecRecHits_TK", {"EcalLumiSpecRawHits_TK"}, {"EcalLumiSpecRecHits_TK"},
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
      },
      app // TODO: Remove me once fixed
      ));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterTruthClustering_factory>(
      "EcalLumiSpecTruthProtoClusters_TK", {"EcalLumiSpecRecHits_TK", "EcalLumiSpecHits"},
      {"EcalLumiSpecTruthProtoClusters_TK"},
      app // TODO: Remove me once fixed
      ));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterIslandCluster_factory>(
      "EcalLumiSpecIslandProtoClusters_TK", {"EcalLumiSpecRecHits_TK"},
      {"EcalLumiSpecIslandProtoClusters_TK"},
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
      },
      app // TODO: Remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
      "EcalLumiSpecClustersWithoutShapes_TK",
      {
          "EcalLumiSpecIslandProtoClusters_TK", // edm4eic::ProtoClusterCollection
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
          "EcalLumiSpecRawHitLinks_TK", // edm4eic::MCRecoCalorimeterHitLink
#endif
          "EcalLumiSpecRawHitAssociations_TK" // edm4eic::MCRecoCalorimeterHitAssociationCollection
      },
      {"EcalLumiSpecClustersWithoutShapes_TK",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
       "EcalLumiSpecClusterLinksWithoutShapes_TK",
#endif
       "EcalLumiSpecClusterAssociationsWithoutShapes_TK"}, // edm4eic::MCRecoClusterParticleAssociation
      {.energyWeight = "log", .sampFrac = 1.0, .logWeightBase = 3.6, .enableEtaBounds = false},
      app // TODO: Remove me once fixed
      ));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>(
      "EcalLumiSpecClusters_TK",
      {"EcalLumiSpecClustersWithoutShapes_TK", "EcalLumiSpecClusterAssociationsWithoutShapes_TK"},
      {"EcalLumiSpecClusters_TK",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
       "EcalLumiSpecClusterLinks_TK",
#endif
       "EcalLumiSpecClusterAssociations_TK"},
      {.energyWeight = "log", .logWeightBase = 3.6}, app));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
      "EcalLumiSpecTruthClustersWithoutShapes_TK",
      {
          "EcalLumiSpecTruthProtoClusters_TK", // edm4eic::ProtoClusterCollection
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
          "EcalLumiSpecRawHitLinks_TK", // edm4eic::MCRecoCalorimeterHitLink
#endif
          "EcalLumiSpecRawHitAssociations_TK" // edm4eic::MCRecoCalorimeterHitAssociationCollection
      },
      {"EcalLumiSpecTruthClustersWithoutShapes_TK",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
       "EcalLumiSpecTruthClusterLinksWithoutShapes_TK",
#endif
       "EcalLumiSpecTruthClusterAssociationsWithoutShapes_TK"}, // edm4eic::MCRecoClusterParticleAssociation
      {.energyWeight = "log", .sampFrac = 1.0, .logWeightBase = 4.6, .enableEtaBounds = false},
      app // TODO: Remove me once fixed
      ));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>(
      "EcalLumiSpecTruthClusters_TK",
      {"EcalLumiSpecTruthClustersWithoutShapes_TK",
       "EcalLumiSpecTruthClusterAssociationsWithoutShapes_TK"},
      {"EcalLumiSpecTruthClusters_TK",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
       "EcalLumiSpecTruthClusterLinks_TK",
#endif
       "EcalLumiSpecTruthClusterAssociations_TK"},
      {.energyWeight = "log", .logWeightBase = 4.6}, app));
}
// }
