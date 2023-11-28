// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplication.h>
#include <math.h>
#include <string>

#include "algorithms/interfaces/WithPodConfig.h"
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
          "EcalLumiSpecRawHits", {"LumiSpecCALHits"}, {"EcalLumiSpecRawHits"},
          {
            .eRes = {0.0 * sqrt(dd4hep::GeV), 0.02, 0.0 * dd4hep::GeV}, // flat 2%
            .tRes = 0.0 * dd4hep::ns,
            .capADC = 16384,
            .dyRangeADC = 20 * dd4hep::GeV,
            .pedMeanADC = 100,
            .pedSigmaADC = 1,
            .resolutionTDC = 10 * dd4hep::picosecond,
            .corrMeanScale = 1.0,
          },
          app   // TODO: Remove me once fixed
        ));
        app->Add(new JOmniFactoryGeneratorT<CalorimeterHitReco_factory>(
          "EcalLumiSpecRecHits", {"EcalLumiSpecRawHits"}, {"EcalLumiSpecRecHits"},
          {
            .capADC = 16384,
            .dyRangeADC = 20. * dd4hep::GeV,
            .pedMeanADC = 100,
            .pedSigmaADC = 1,
            .resolutionTDC = 10 * dd4hep::picosecond,
            .thresholdFactor = 0.0,
            .thresholdValue = 2.0,
            .sampFrac = 1.0,
            .readout = "LumiSpecCALHits",
            .sectorField = "sector",
          },
          app   // TODO: Remove me once fixed
        ));
        app->Add(new JOmniFactoryGeneratorT<CalorimeterTruthClustering_factory>(
          "EcalLumiSpecTruthProtoClusters", {"EcalLumiSpecRecHits", "LumiSpecCALHits"}, {"EcalLumiSpecTruthProtoClusters"},
          app   // TODO: Remove me once fixed
        ));
        app->Add(new JOmniFactoryGeneratorT<CalorimeterIslandCluster_factory>(
          "EcalLumiSpecIslandProtoClusters", {"EcalLumiSpecRecHits"}, {"EcalLumiSpecIslandProtoClusters"},
          {
            .adjacencyMatrix = "(sector_1 == sector_2) && ((abs(floor(module_1 / 10) - floor(module_2 / 10)) + abs(fmod(module_1, 10) - fmod(module_2, 10))) == 1)",
            .readout = "LumiSpecCALHits",
            .sectorDist = 0.0 * dd4hep::cm,
            .splitCluster=true,
            .minClusterHitEdep = 1.0 * dd4hep::MeV,
            .minClusterCenterEdep = 30.0 * dd4hep::MeV,
            .transverseEnergyProfileMetric = "localDistXY",
            .transverseEnergyProfileScale = 10. * dd4hep::mm,
          },
          app   // TODO: Remove me once fixed
        ));

        app->Add(
          new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
             "EcalLumiSpecClusters",
            {"EcalLumiSpecIslandProtoClusters",  // edm4eic::ProtoClusterCollection
             "LumiSpecCALHits"},                 // edm4hep::SimCalorimeterHitCollection
            {"EcalLumiSpecClusters",             // edm4eic::Cluster
             "EcalLumiSpecClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
            {
              {"energyWeight", "log"},
              {"samplingFraction", 1.0},
              {"logWeightBase", 3.6},
              {"enableEtaBounds", false}
            },
            app   // TODO: Remove me once fixed
          )
        );

        app->Add(
          new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
             "EcalLumiSpecTruthClusters",
            {"EcalLumiSpecTruthProtoClusters",        // edm4eic::ProtoClusterCollection
             "LumiSpecCALHits"},                      // edm4hep::SimCalorimeterHitCollection
            {"EcalLumiSpecTruthClusters",             // edm4eic::Cluster
             "EcalLumiSpecTruthClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
            {
              {"energyWeight", "log"},
              {"samplingFraction", 1.0},
              {"logWeightBase", 4.6},
              {"enableEtaBounds", false}
            },
            app   // TODO: Remove me once fixed
          )
        );
    }
}
