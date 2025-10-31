// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2021 - 2025, Chao Peng, Sylvester Joosten, Whitney Armstrong, David Lawrence, Friederike Bock, Wouter Deconinck, Kolja Kauder, Sebouh Paul, Akio Ogawa

#include <DD4hep/Detector.h>
#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplication.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/Utils/JTypeInfo.h>
#include <fmt/core.h>
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

extern "C" {
void InitPlugin(JApplication* app) {

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
      fmt::format("{}", 1.0 / EcalEndcapP_sampFrac);
  const double EcalEndcapP_nPhotonPerGeV          = 1500;
  const double EcalEndcapP_PhotonCollectionEff    = 0.5;
  const unsigned long long EcalEndcapP_totalPixel = 4 * 159565ULL;

  int EcalEndcapP_homogeneousFlag = 0;
  try {
    auto detector               = app->GetService<DD4hep_service>()->detector();
    EcalEndcapP_homogeneousFlag = detector->constant<int>("ForwardEcal_Homogeneous_Scfi");
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
        "EcalEndcapPRawHits", {"EventHeader", "EcalEndcapPHits"},
        {"EcalEndcapPRawHits", "EcalEndcapPRawHitAssociations"},
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
        app // TODO: Remove me once fixed
        ));
  } else if (EcalEndcapP_homogeneousFlag == 2) {
    app->Add(new JOmniFactoryGeneratorT<CalorimeterHitDigi_factory>(
        "EcalEndcapPRawHits", {"EventHeader", "EcalEndcapPHits"},
        {"EcalEndcapPRawHits", "EcalEndcapPRawHitAssociations"},
        {
            .eRes = {0.0, 0.022, 0.0}, // just constant term 2.2% based on MC data comparison
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
            .corrMeanScale             = EcalEndcapP_corrMeanScale,
            .readout                   = "EcalEndcapPHits",
            .fields                    = {"fiber_x", "fiber_y", "x", "y"},
        },
        app // TODO: Remove me once fixed
        ));
  }

  app->Add(new JOmniFactoryGeneratorT<CalorimeterHitReco_factory>(
      "EcalEndcapPRecHits", {"EcalEndcapPRawHits"}, {"EcalEndcapPRecHits"},
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
      app // TODO: Remove me once fixed
      ));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterTruthClustering_factory>(
      "EcalEndcapPTruthProtoClusters", {"EcalEndcapPRecHits", "EcalEndcapPHits"},
      {"EcalEndcapPTruthProtoClusters"},
      app // TODO: Remove me once fixed
      ));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterIslandCluster_factory>(
      "EcalEndcapPIslandProtoClusters", {"EcalEndcapPRecHits"}, {"EcalEndcapPIslandProtoClusters"},
      {.adjacencyMatrix{},
       .peakNeighbourhoodMatrix{},
       .readout{},
       .sectorDist = 5.0 * dd4hep::cm,
       .localDistXY{},
       .localDistXZ{},
       .localDistYZ{},
       .globalDistRPhi{},
       .globalDistEtaPhi{},
       .dimScaledLocalDistXY          = {1.5, 1.5},
       .splitCluster                  = false,
       .minClusterHitEdep             = 0.0 * dd4hep::MeV,
       .minClusterCenterEdep          = 60.0 * dd4hep::MeV,
       .transverseEnergyProfileMetric = "dimScaledLocalDistXY",
       .transverseEnergyProfileScale  = 1.,
       .transverseEnergyProfileScaleUnits{}},
      app // TODO: Remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
      "EcalEndcapPTruthClustersWithoutShapes",
      {
          "EcalEndcapPTruthProtoClusters", // edm4eic::ProtoClusterCollection
          "EcalEndcapPRawHitAssociations"  // edm4eic::MCRecoCalorimeterHitAssociationCollection
      },
      {"EcalEndcapPTruthClustersWithoutShapes",             // edm4eic::Cluster
       "EcalEndcapPTruthClusterAssociationsWithoutShapes"}, // edm4eic::MCRecoClusterParticleAssociation
      {.energyWeight = "log", .sampFrac = 1.0, .logWeightBase = 6.2, .enableEtaBounds = true},
      app // TODO: Remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>(
      "EcalEndcapPTruthClusters",
      {"EcalEndcapPTruthClustersWithoutShapes", "EcalEndcapPTruthClusterAssociationsWithoutShapes"},
      {"EcalEndcapPTruthClusters", "EcalEndcapPTruthClusterAssociations"},
      {.energyWeight = "log", .logWeightBase = 6.2}, app));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
      "EcalEndcapPClustersWithoutShapes",
      {
          "EcalEndcapPIslandProtoClusters", // edm4eic::ProtoClusterCollection
          "EcalEndcapPRawHitAssociations"   // edm4eic::MCRecoCalorimeterHitAssociationCollection
      },
      {"EcalEndcapPClustersWithoutShapes",             // edm4eic::Cluster
       "EcalEndcapPClusterAssociationsWithoutShapes"}, // edm4eic::MCRecoClusterParticleAssociation
      {
          .energyWeight    = "log",
          .sampFrac        = 1.0,
          .logWeightBase   = 3.6,
          .enableEtaBounds = false,
      },
      app // TODO: Remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>(
      "EcalEndcapPClusters",
      {"EcalEndcapPClustersWithoutShapes", "EcalEndcapPClusterAssociationsWithoutShapes"},
      {"EcalEndcapPClusters", "EcalEndcapPClusterAssociations"},
      {.energyWeight = "log", .logWeightBase = 3.6}, app));

  app->Add(new JOmniFactoryGeneratorT<TrackClusterMergeSplitter_factory>(
      "EcalEndcapPSplitMergeProtoClusters",
      {"EcalEndcapPIslandProtoClusters", "CalorimeterTrackProjections"},
      {"EcalEndcapPSplitMergeProtoClusters"},
      {.idCalo                       = "EcalEndcapP_ID",
       .minSigCut                    = -2.0,
       .avgEP                        = 1.0,
       .sigEP                        = 0.10,
       .drAdd                        = 0.30,
       .sampFrac                     = 1.0,
       .transverseEnergyProfileScale = 1.0},
      app // TODO: remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
      "EcalEndcapPSplitMergeClustersWithoutShapes",
      {
          "EcalEndcapPSplitMergeProtoClusters", // edm4eic::ProtoClusterCollection
          "EcalEndcapPRawHitAssociations" // edm4hep::MCRecoCalorimeterHitAssociationCollection
      },
      {"EcalEndcapPSplitMergeClustersWithoutShapes",             // edm4eic::Cluster
       "EcalEndcapPSplitMergeClusterAssociationsWithoutShapes"}, // edm4eic::MCRecoClusterParticleAssociation
      {.energyWeight = "log", .sampFrac = 1.0, .logWeightBase = 3.6, .enableEtaBounds = false},
      app // TODO: Remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>(
      "EcalEndcapPSplitMergeClusters",
      {"EcalEndcapPSplitMergeClustersWithoutShapes",
       "EcalEndcapPSplitMergeClusterAssociationsWithoutShapes"},
      {"EcalEndcapPSplitMergeClusters", "EcalEndcapPSplitMergeClusterAssociations"},
      {.energyWeight = "log", .logWeightBase = 3.6}, app));
}
}
