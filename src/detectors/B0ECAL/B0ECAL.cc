// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplication.h>
#include <math.h>
#include <string>

#include "algorithms/calorimetry/CalorimeterClusterRecoCoGConfig.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/calorimetry/CalorimeterClusterRecoCoG_factory.h"
#include "factories/calorimetry/CalorimeterHitDigi_factory.h"
#include "factories/calorimetry/CalorimeterHitReco_factory.h"
#include "factories/calorimetry/CalorimeterIslandCluster_factory.h"
#include "factories/calorimetry/CalorimeterTruthClustering_factory.h"

extern "C" {
    void InitPlugin(JApplication *app) {

        using namespace eicrecon;

        InitJANAPlugin(app);

        app->Add(new JOmniFactoryGeneratorT<CalorimeterHitDigi_factory>(
          "B0ECalRawHits", {"B0ECalHits"}, {"B0ECalRawHits"},
          {
            .eRes = {0.0 * sqrt(dd4hep::GeV), 0.02, 0.0 * dd4hep::GeV},
            .tRes = 0.0 * dd4hep::ns,
            .threshold= 5.0 * dd4hep::MeV,
            .capADC = 16384,
            .dyRangeADC = 20 * dd4hep::GeV,
            .pedMeanADC = 100,
            .pedSigmaADC = 1,
            .resolutionTDC = 1e-11,
            .corrMeanScale = 1.0,
          },
          app
        ));
        app->Add(new JOmniFactoryGeneratorT<CalorimeterHitReco_factory>(
          "B0ECalRecHits", {"B0ECalRawHits"}, {"B0ECalRecHits"},
          {
            .capADC = 16384,
            .dyRangeADC = 20. * dd4hep::GeV,
            .pedMeanADC = 100,
            .pedSigmaADC = 1,
            .resolutionTDC = 1e-11,
            .thresholdFactor = 0.0,
            .thresholdValue = 0.0,
            .sampFrac = 0.998,
            .readout = "B0ECalHits",
            .sectorField = "sector",
          },
          app
        ));
        app->Add(new JOmniFactoryGeneratorT<CalorimeterTruthClustering_factory>(
          "B0ECalTruthProtoClusters", {"B0ECalRecHits", "B0ECalHits"}, {"B0ECalTruthProtoClusters"},
          app
        ));
        app->Add(new JOmniFactoryGeneratorT<CalorimeterIslandCluster_factory>(
          "B0ECalIslandProtoClusters", {"B0ECalRecHits"}, {"B0ECalIslandProtoClusters"},
          {
            .sectorDist = 5.0 * dd4hep::cm,
            .dimScaledLocalDistXY = {1.8,1.8},
            .splitCluster = false,
            .minClusterHitEdep = 1.0 * dd4hep::MeV,
            .minClusterCenterEdep = 30.0 * dd4hep::MeV,
            .transverseEnergyProfileMetric = "globalDistEtaPhi",
            .transverseEnergyProfileScale = 1.,
          },
          app
        ));

        app->Add(
          new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
             "B0ECalClusters",
            {"B0ECalIslandProtoClusters",  // edm4eic::ProtoClusterCollection
             "B0ECalHits"},                // edm4hep::SimCalorimeterHitCollection
            {"B0ECalClusters",             // edm4eic::Cluster
             "B0ECalClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
            {
              .energyWeight = "log",
              .sampFrac = 1.0,
              .logWeightBase = 3.6,
              .enableEtaBounds = false
            },
            app
          )
        );

        app->Add(
          new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
             "B0ECalTruthClusters",
            {"B0ECalTruthProtoClusters",        // edm4eic::ProtoClusterCollection
             "B0ECalHits"},                     // edm4hep::SimCalorimeterHitCollection
            {"B0ECalTruthClusters",             // edm4eic::Cluster
             "B0ECalTruthClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
            {
              .energyWeight = "log",
              .sampFrac = 1.0,
              .logWeightBase = 6.2,
              .enableEtaBounds = false
            },
            app
          )
        );
    }
}
