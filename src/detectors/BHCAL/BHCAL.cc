// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplication.h>
#include <string>

#include "algorithms/calorimetry/CalorimeterHitDigiConfig.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/calorimetry/CalorimeterClusterRecoCoG_factory.h"
#include "factories/calorimetry/CalorimeterHitDigi_factory.h"
#include "factories/calorimetry/CalorimeterHitReco_factory.h"
#include "factories/calorimetry/CalorimeterIslandCluster_factory.h"
#include "factories/calorimetry/CalorimeterTruthClustering_factory.h"

// TEST
#include <iostream>
#include <DD4hep/Detector.h>
#include <DD4hep/DetElement.h>
#include <DD4hep/IDDescriptor.h>
#include <DD4hep/Readout.h>
#include <DDSegmentation/BitFieldCoder.h>
#include "services/geometry/dd4hep/DD4hep_service.h"

extern "C" {

    bool UseSectorIndexedBHCalReadout(JApplication *app) {

      using namespace eicrecon;

      // grab detector
      auto service    = app -> GetService<DD4hep_service>();
      auto detector   = service -> detector(); 
      auto descriptor = detector -> readout("HcalBarrelHits").idSpec();

      // check if sector field is present
      bool useSectorIndex = false;
      try {
        auto sector = descriptor.field("sector");
        return true;
      } catch(...) {
        return false;
      }

    }

    void InitPlugin(JApplication *app) {

        using namespace eicrecon;

        InitJANAPlugin(app);

        // Make sure digi and reco use the same value
        decltype(CalorimeterHitDigiConfig::capADC)        HcalBarrel_capADC = 65536; //65536,  16bit ADC
        decltype(CalorimeterHitDigiConfig::dyRangeADC)    HcalBarrel_dyRangeADC = 1.0 * dd4hep::GeV;
        decltype(CalorimeterHitDigiConfig::pedMeanADC)    HcalBarrel_pedMeanADC = 300;
        decltype(CalorimeterHitDigiConfig::pedSigmaADC)   HcalBarrel_pedSigmaADC = 2;
        decltype(CalorimeterHitDigiConfig::resolutionTDC) HcalBarrel_resolutionTDC = 1 * dd4hep::picosecond;

        // Set default adjacency matrix. Magic constants:
        //  1512 - 64 * 24
        //  64   - number of rows in the barrel
        //  24   - number of towers per row along eta
        //  4    - 5, the number of tiles per tower, - 1
        decltype(CalorimeterIslandClusterConfig::adjacencyMatrix) HcalBarrel_adjacentMatrix =
          "("
          // check for vertically adjacent tiles
          "  ( (abs(tower_1 - tower_2) == 1) && (abs(tile_1 - tile_2) == 0) ) ||"
          // check for horizontally adjacent tiles in the same tower
          "  ( (abs(tower_1 - tower_2) == 0) && (abs(tile_1 - tile_2) == 1) ) ||"
          // check for horizontally adjacent tiles in neighboring towers along phi
          "  ( ((tower_1 - tower_2) == -24)    && ((tile_1 - tile_2) == 4)      ) ||"
          "  ( ((tower_1 - tower_2) == 24)     && ((tile_1 - tile_2) == -4)     ) ||"
          // check for horizontally adjacent tiles in neighboring towers along phi at the wraparound
          "  ( ((tower_1 - tower_2) == -1512)  && ((tile_1 - tile_2) == -4)     ) ||"
          "  ( ((tower_1 - tower_2) == 1512)   && ((tile_1 - tile_2) == 4)      )"
          ") == 1";

        // If using readout structure with sector indices, check adjacency using those
        if ( UseSectorIndexedBHCalReadout(app) ) {
          HcalBarrel_adjacentMatrix =
            "("
            "  abs(fmod(tower_1, 24) - fmod(tower_2, 24))"
            "  + min("
            "      abs((sector_1 - sector_2) * (2 * 5) + (floor(tower_1 / 24) - floor(tower_2 / 24)) * 5 + fmod(tile_1, 5) - fmod(tile_2, 5)),"
            "      (32 * 2 * 5) - abs((sector_1 - sector_2) * (2 * 5) + (floor(tower_1 / 24) - floor(tower_2 / 24)) * 5 + fmod(tile_1, 5) - fmod(tile_2, 5))"
            "    )"
            ") == 1";
        }

        app->Add(new JOmniFactoryGeneratorT<CalorimeterHitDigi_factory>(
          "HcalBarrelRawHits", {"HcalBarrelHits"}, {"HcalBarrelRawHits"},
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
            .corrMeanScale = 1.0,
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
            .sampFrac = 0.033, // average, from sPHENIX simulations
            .readout = "HcalBarrelHits",
            .layerField = "",
            .sectorField = "",
          },
          app   // TODO: Remove me once fixed
        ));
        app->Add(new JOmniFactoryGeneratorT<CalorimeterTruthClustering_factory>(
          "HcalBarrelTruthProtoClusters", {"HcalBarrelRecHits", "HcalBarrelHits"}, {"HcalBarrelTruthProtoClusters"},
          app   // TODO: Remove me once fixed
        ));
        app->Add(new JOmniFactoryGeneratorT<CalorimeterIslandCluster_factory>(
          "HcalBarrelIslandProtoClusters", {"HcalBarrelRecHits"}, {"HcalBarrelIslandProtoClusters"},
          {
            .adjacencyMatrix = HcalBarrel_adjacentMatrix,
            .readout = "HcalBarrelHits",
            .sectorDist = 5.0 * dd4hep::cm,
            .localDistXY = {15*dd4hep::mm, 15*dd4hep::mm},
            .dimScaledLocalDistXY = {50.0*dd4hep::mm, 50.0*dd4hep::mm},
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
             "HcalBarrelHits"},                // edm4hep::SimCalorimeterHitCollection
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
             "HcalBarrelHits"},                     // edm4hep::SimCalorimeterHitCollection
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

    }
}
