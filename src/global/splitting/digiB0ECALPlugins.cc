// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2025 Whitney Armstrong, Sylvester Joosten, Chao Peng, David Lawrence, Wouter Deconinck, Kolja Kauder, Nathan Brei, Dmitry Kalinkin, Derek Anderson, Michael Pitt

#include <edm4eic/EDM4eicVersion.h>
#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplicationFwd.h>
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
void InitPlugin_digiB0ECAL(JApplication* app) {

  using namespace eicrecon;

  InitJANAPlugin(app);

  app->Add(new JOmniFactoryGeneratorT<CalorimeterHitDigi_factory>(
      JOmniFactoryGeneratorT<CalorimeterHitDigi_factory>::TypedWiring{
          .m_tag                 = "B0ECalRawHits_TK",
          .m_default_input_tags  = {"EventHeader", "B0ECalHits"},
          .m_default_output_tags = {"B0ECalRawHits_TK",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                                    "B0ECalRawHitLinks_TK",
#endif
                                    "B0ECalRawHitAssociations_TK"},
          .m_default_cfg =
              {
                  // The stochastic term is set using light yield in PbOW4 of N_photons = 145.75 / GeV / mm, for 6x6 mm2 sensors with PDE=0.18 (a=1/sqrt(145.75*36*0.18))
                  .eRes          = {0.0326 * sqrt(dd4hep::GeV), 0.00, 0.0 * dd4hep::GeV},
                  .tRes          = 0.0 * dd4hep::ns,
                  .threshold     = 5.0 * dd4hep::MeV,
                  .capADC        = 16384,
                  .dyRangeADC    = 170 * dd4hep::GeV,
                  .pedMeanADC    = 100,
                  .pedSigmaADC   = 1,
                  .resolutionTDC = 1e-11,
                  .corrMeanScale = "1.0",
                  .readout       = "B0ECalHits",
              },
          .level = JEventLevel::Timeslice},
      app));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterHitReco_factory>(
      JOmniFactoryGeneratorT<CalorimeterHitReco_factory>::TypedWiring{
          .m_tag                 = "B0ECalRecHits_TK",
          .m_default_input_tags  = {"B0ECalRawHits_TK"},
          .m_default_output_tags = {"B0ECalRecHits_TK"},
          .m_default_cfg =
              {
                  .capADC          = 16384,
                  .dyRangeADC      = 170. * dd4hep::GeV,
                  .pedMeanADC      = 100,
                  .pedSigmaADC     = 1,
                  .resolutionTDC   = 1e-11,
                  .thresholdFactor = 0.0,
                  .thresholdValue  = 1.0, // using threshold of 10 photons = 10 MeV = 1 ADC
                  .sampFrac        = "0.998",
                  .readout         = "B0ECalHits",
                  .sectorField     = "sector",
              },
          .level = JEventLevel::Timeslice},
      app));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterTruthClustering_factory>(
      JOmniFactoryGeneratorT<CalorimeterTruthClustering_factory>::TypedWiring{
          .m_tag                 = "B0ECalTruthProtoClusters_TK",
          .m_default_input_tags  = {"B0ECalRecHits_TK", "B0ECalHits"},
          .m_default_output_tags = {"B0ECalTruthProtoClusters_TK"},
          .m_default_cfg         = {},
          .level                 = JEventLevel::Timeslice},
      app));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterIslandCluster_factory>(
      JOmniFactoryGeneratorT<CalorimeterIslandCluster_factory>::TypedWiring{
          .m_tag                 = "B0ECalIslandProtoClusters_TK",
          .m_default_input_tags  = {"B0ECalRecHits_TK"},
          .m_default_output_tags = {"B0ECalIslandProtoClusters_TK"},
          .m_default_cfg =
              {
                  .adjacencyMatrix{},
                  .peakNeighbourhoodMatrix{},
                  .readout{},
                  .sectorDist = 5.0 * dd4hep::cm,
                  .localDistXY{},
                  .localDistXZ{},
                  .localDistYZ{},
                  .globalDistRPhi{},
                  .globalDistEtaPhi{},
                  .dimScaledLocalDistXY          = {1.8, 1.8},
                  .splitCluster                  = false,
                  .minClusterHitEdep             = 1.0 * dd4hep::MeV,
                  .minClusterCenterEdep          = 30.0 * dd4hep::MeV,
                  .transverseEnergyProfileMetric = "globalDistEtaPhi",
                  .transverseEnergyProfileScale  = 1.,
                  .transverseEnergyProfileScaleUnits{},
              },
          .level = JEventLevel::Timeslice},
      app));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
      JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>::TypedWiring{
          .m_tag = "B0ECalClustersWithoutShapes_TK",
          .m_default_input_tags =
              {
                  "B0ECalIslandProtoClusters_TK", // edm4eic::ProtoClusterCollection
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                  "B0ECalRawHitLinks_TK", // edm4eic::MCRecoCalorimeterHitLink
#endif
                  "B0ECalRawHitAssociations_TK" // edm4eic::MCRecoCalorimeterHitAssociationCollection
              },
          .m_default_output_tags = {"B0ECalClustersWithoutShapes_TK",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                                    "B0ECalClusterLinksWithoutShapes_TK",
#endif
                                    "B0ECalClusterAssociationsWithoutShapes_TK"},
          .m_default_cfg =
              {
                  .energyWeight    = "log",
                  .sampFrac        = 1.0,
                  .logWeightBase   = 3.6,
                  .enableEtaBounds = false,
              },
          .level = JEventLevel::Timeslice},
      app));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>(
      JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>::TypedWiring{
          .m_tag                 = "B0ECalClusters_TK",
          .m_default_input_tags  = {"B0ECalClustersWithoutShapes_TK",
                                    "B0ECalClusterAssociationsWithoutShapes_TK"},
          .m_default_output_tags = {"B0ECalClusters_TK",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                                    "B0ECalClusterLinks_TK",
#endif
                                    "B0ECalClusterAssociations_TK"},
          .m_default_cfg = {.energyWeight = "log", .logWeightBase = 3.6},
          .level         = JEventLevel::Timeslice},
      app));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
      JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>::TypedWiring{
          .m_tag = "B0ECalTruthClustersWithoutShapes_TK",
          .m_default_input_tags =
              {
                  "B0ECalTruthProtoClusters_TK", // edm4eic::ProtoClusterCollection
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                  "B0ECalRawHitLinks_TK", // edm4eic::MCRecoCalorimeterHitLink
#endif
                  "B0ECalRawHitAssociations_TK" // edm4eic::MCRecoCalorimeterHitAssociationCollection
              },
          .m_default_output_tags = {"B0ECalTruthClustersWithoutShapes_TK",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                                    "B0ECalTruthClusterLinksWithoutShapes_TK",
#endif
                                    "B0ECalTruthClusterAssociationsWithoutShapes_TK"},
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
          .m_tag                 = "B0ECalTruthClusters_TK",
          .m_default_input_tags  = {"B0ECalTruthClustersWithoutShapes_TK",
                                    "B0ECalTruthClusterAssociationsWithoutShapes_TK"},
          .m_default_output_tags = {"B0ECalTruthClusters_TK",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                                    "B0ECalTruthClusterLinks_TK",
#endif
                                    "B0ECalTruthClusterAssociations_TK"},
          .m_default_cfg = {.energyWeight = "log", .logWeightBase = 6.2},
          .level         = JEventLevel::Timeslice},
      app));
}
// }
