// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2024 David Lawrence, Derek Anderson, Wouter Deconinck

#include <edm4eic/EDM4eicVersion.h>
#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplication.h>
#include <memory>

#include "algorithms/calorimetry/CalorimeterHitDigiConfig.h"
#include "algorithms/calorimetry/CalorimeterIslandClusterConfig.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/calorimetry/CalorimeterClusterRecoCoG_factory.h"
#include "factories/calorimetry/CalorimeterHitDigi_factory.h"
#include "factories/calorimetry/CalorimeterHitReco_factory.h"
#include "factories/calorimetry/CalorimeterHitsMerger_factory.h"
#include "factories/calorimetry/CalorimeterIslandCluster_factory.h"
#include "factories/calorimetry/CalorimeterTruthClustering_factory.h"
#include "factories/calorimetry/TrackClusterMergeSplitter_factory.h"

namespace eicrecon {

  // --------------------------------------------------------------------------
  // Helper method to generate appropriate phi map
  // --------------------------------------------------------------------------
  std::string MakePhiMap(const std::size_t nMerge = 1) {

    // convert num to merge to string
    const std::string sNum = std::to_string(nMerge);

    // construct mapping
    std::string map  = "";
    if (nMerge > 1) {
      map = "phi-(" + sNum + "*((phi/" + sNum + ")-floor(phi/" + sNum + ")))";
    } else {
      map = "phi";
    }
    return map;

  }  // end 'MakePhiMap(std::size_t)'

  // --------------------------------------------------------------------------
  // Helper method to generate appropriate adjacency matrix
  // --------------------------------------------------------------------------
  std::string MakeAdjacencyMatrix(const std::size_t nMerge = 1) {

    // set up checks at wraparound.
    // magic constants:
    //   320 - number of tiles per row
    const std::string sMerge    = std::to_string(nMerge);
    const std::string wrapDef   = "(abs(phi_1 - phi_2) == (320 - 1))";
    const std::string wrapMerge = "(abs(320 - abs(phi_1 - phi_2)) <= " + sMerge + ")";

    // combine strings into horizontal adjacency conditions
    const std::string phiAdjacent  = "(abs(phi_1 - phi_2) == " + sMerge + ")";
    const std::string wrapAdjacent = nMerge > 1 ? wrapMerge : wrapDef;

    // put everything together
    std::string matrix("");
    matrix += "(";
    // check for vertically adjacent tiles
    matrix += "  ( (abs(eta_1 - eta_2) == 1) && (abs(phi_1 - phi_2) == 0) ) ||";
    // check for horizontally adjacent tiles
    matrix += "  ( (abs(eta_1 - eta_2) == 0) && " + phiAdjacent + " ) ||";
    // check for horizontally adjacent tiles at wraparound
    matrix += "  ( (abs(eta_1 - eta_2) == 0) && " + wrapAdjacent + " )";
    matrix += ") == 1";
    return matrix;

  }  // end 'MakeAdjacencyMatrix(std::size_t)'

}



extern "C" {

    void InitPlugin(JApplication *app) {

        using namespace eicrecon;

        InitJANAPlugin(app);

        // Make sure digi and reco use the same value
        decltype(CalorimeterHitDigiConfig::capADC)        HcalBarrel_capADC = 65536; //65536,  16bit ADC
        decltype(CalorimeterHitDigiConfig::dyRangeADC)    HcalBarrel_dyRangeADC = 1.0 * dd4hep::GeV;
        decltype(CalorimeterHitDigiConfig::pedMeanADC)    HcalBarrel_pedMeanADC = 300;
        decltype(CalorimeterHitDigiConfig::pedSigmaADC)   HcalBarrel_pedSigmaADC = 2;
        decltype(CalorimeterHitDigiConfig::resolutionTDC) HcalBarrel_resolutionTDC = 1 * dd4hep::picosecond;

        // ---------------------------------------------------------------------
        // if needed, merge adjacent tiles into a tower and adjust maps/matrices
        // ---------------------------------------------------------------------
        const std::size_t nPhiToMerge = 1;
        const std::string phiMap      = MakePhiMap(nPhiToMerge);
        const std::string adjMatrix   = MakeAdjacencyMatrix(nPhiToMerge);

        // set matrix
        decltype(CalorimeterIslandClusterConfig::adjacencyMatrix) HcalBarrel_adjacencyMatrix = adjMatrix;

        app->Add(new JOmniFactoryGeneratorT<CalorimeterHitDigi_factory>(
          "HcalBarrelRawHits",
          {"HcalBarrelHits"},
#if EDM4EIC_VERSION_MAJOR >= 7
          {"HcalBarrelRawHits", "HcalBarrelRawHitAssociations"},
#else
          {"HcalBarrelRawHits"},
#endif
          {
            .eRes = {},
            .tRes = 0.0 * dd4hep::ns,
            .threshold = 0.0, // Use ADC cut instead
            .capADC        = HcalBarrel_capADC,
            .capTime = 100, // given in ns, 4 samples in HGCROC
            .dyRangeADC    = HcalBarrel_dyRangeADC,
            .pedMeanADC    = HcalBarrel_pedMeanADC,
            .pedSigmaADC   = HcalBarrel_pedSigmaADC,
            .resolutionTDC = HcalBarrel_resolutionTDC,
            .corrMeanScale = "1.0",
            .readout = "HcalBarrelHits",
          },
          app   // TODO: Remove me once fixed
        ));

        app->Add(new JOmniFactoryGeneratorT<CalorimeterHitReco_factory>(
          "HcalBarrelRecHits", {"HcalBarrelRawHits"}, {"HcalBarrelRecHits"},
          {
            .capADC        = HcalBarrel_capADC,
            .dyRangeADC    = HcalBarrel_dyRangeADC,
            .pedMeanADC    = HcalBarrel_pedMeanADC,
            .pedSigmaADC   = HcalBarrel_pedSigmaADC, // not used; relying on energy cut
            .resolutionTDC = HcalBarrel_resolutionTDC,
            .thresholdFactor = 0.0, // not used; relying on flat ADC cut
            .thresholdValue = 33, // pedSigmaADC + thresholdValue = half-MIP (333 ADC)
            .sampFrac = "0.033", // average, from sPHENIX simulations
            .readout = "HcalBarrelHits",
            .layerField = "",
            .sectorField = "",
          },
          app   // TODO: Remove me once fixed
        ));

        app->Add(new JOmniFactoryGeneratorT<CalorimeterHitsMerger_factory>(
          "HcalBarrelMergedHits", {"HcalBarrelRecHits"}, {"HcalBarrelMergedHits"},
          {
            .readout = "HcalBarrelHits",
            .fields = {"phi"},
            .mappings = {phiMap}
          },
          app   // TODO: Remove me once fixed
        ));

        app->Add(new JOmniFactoryGeneratorT<CalorimeterTruthClustering_factory>(
          "HcalBarrelTruthProtoClusters", {"HcalBarrelRecHits", "HcalBarrelHits"}, {"HcalBarrelTruthProtoClusters"},
          app   // TODO: Remove me once fixed
        ));

        app->Add(new JOmniFactoryGeneratorT<CalorimeterIslandCluster_factory>(
          "HcalBarrelIslandProtoClusters", {"HcalBarrelMergedHits"}, {"HcalBarrelIslandProtoClusters"},
          {
            .adjacencyMatrix = HcalBarrel_adjacencyMatrix,
            .readout = "HcalBarrelHits",
            .sectorDist = 5.0 * dd4hep::cm,
            .splitCluster = false,
            .minClusterHitEdep = 5.0 * dd4hep::MeV,
            .minClusterCenterEdep = 30.0 * dd4hep::MeV,
            .transverseEnergyProfileMetric = "globalDistEtaPhi",
            .transverseEnergyProfileScale = 1.,
          },
          app   // TODO: Remove me once fixed
        ));

        app->Add(
          new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
             "HcalBarrelClusters",
            {"HcalBarrelIslandProtoClusters",  // edm4eic::ProtoClusterCollection
#if EDM4EIC_VERSION_MAJOR >= 7
             "HcalBarrelRawHitAssociations"},  // edm4eic::MCRecoCalorimeterHitAssociationCollection
#else
             "HcalBarrelHits"},                // edm4hep::SimCalorimeterHitCollection
#endif
            {"HcalBarrelClusters",             // edm4eic::Cluster
             "HcalBarrelClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
            {
              .energyWeight = "log",
              .sampFrac = 1.0,
              .logWeightBase = 6.2,
              .enableEtaBounds = false
            },
            app   // TODO: Remove me once fixed
          )
        );

        app->Add(
          new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
             "HcalBarrelTruthClusters",
            {"HcalBarrelTruthProtoClusters",        // edm4eic::ProtoClusterCollection
#if EDM4EIC_VERSION_MAJOR >= 7
             "HcalBarrelRawHitAssociations"},       // edm4eic::MCRecoCalorimeterHitAssociationCollection
#else
             "HcalBarrelHits"},                     // edm4hep::SimCalorimeterHitCollection
#endif
            {"HcalBarrelTruthClusters",             // edm4eic::Cluster
             "HcalBarrelTruthClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
            {
              .energyWeight = "log",
              .sampFrac = 1.0,
              .logWeightBase = 6.2,
              .enableEtaBounds = false
            },
            app   // TODO: Remove me once fixed
          )
        );

        app->Add(
          new JOmniFactoryGeneratorT<TrackClusterMergeSplitter_factory>(
            "HcalBarrelSplitMergeProtoClusters",
            {"HcalBarrelIslandProtoClusters",
             "CalorimeterTrackProjections"},
            {"HcalBarrelSplitMergeProtoClusters"},
            {
              .idCalo = "HcalBarrel_ID",
              .minSigCut = -2.0,
              .avgEP = 0.50,
              .sigEP = 0.25,
              .drAdd = 0.40,
              .sampFrac = 1.0,
              .transverseEnergyProfileScale = 1.0
            },
            app   // TODO: remove me once fixed
          )
        );

        app->Add(
          new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
             "HcalBarrelSplitMergeClusters",
            {"HcalBarrelSplitMergeProtoClusters",        // edm4eic::ProtoClusterCollection
             "HcalBarrelHits"},                          // edm4hep::SimCalorimeterHitCollection
            {"HcalBarrelSplitMergeClusters",             // edm4eic::Cluster
             "HcalBarrelSplitMergeClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
            {
              .energyWeight = "log",
              .sampFrac = 1.0,
              .logWeightBase = 6.2,
              .enableEtaBounds = false
            },
            app   // TODO: Remove me once fixed
          )
        );

    }
}
