// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplication.h>
#include <string>

#include "algorithms/interfaces/WithPodConfig.h"
#include "extensions/jana/JChainMultifactoryGeneratorT.h"
#include "factories/calorimetry/CalorimeterClusterRecoCoG_factoryT.h"
#include "factories/calorimetry/CalorimeterHitDigi_factoryT.h"
#include "factories/calorimetry/CalorimeterHitReco_factoryT.h"
#include "factories/calorimetry/CalorimeterIslandCluster_factoryT.h"
#include "factories/calorimetry/CalorimeterTruthClustering_factoryT.h"

extern "C" {
    void InitPlugin(JApplication *app) {

        using namespace eicrecon;

        InitJANAPlugin(app);

        app->Add(new JChainMultifactoryGeneratorT<CalorimeterHitDigi_factoryT>(
         "ZDCRawHits", {"HcalFarForwardZDCHits"}, {"ZDCRawHits"},
          {
            .tRes = 0.0 * dd4hep::ns,
            .capADC = 32768,
            .dyRangeADC = 200 * dd4hep::MeV,
            .pedMeanADC = 400,
            .pedSigmaADC = 10,
            .resolutionTDC = 10 * dd4hep::picosecond,
            .corrMeanScale = 1.0,
          },
          app   // TODO: Remove me once fixed
        ));
        app->Add(new JChainMultifactoryGeneratorT<CalorimeterHitReco_factoryT>(
          "ZDCRecHits", {"ZDCRawHits"}, {"ZDCRecHits"},
          {
            .capADC = 32678,
            .dyRangeADC = 200. * dd4hep::MeV,
            .pedMeanADC = 400,
            .pedSigmaADC = 10,
            .resolutionTDC = 10 * dd4hep::picosecond,
            .thresholdFactor = 0.0,
            .thresholdValue = -100.0,
            .sampFrac = 1.0,
            .readout = "HcalFarForwardZDCHits",
          },
          app   // TODO: Remove me once fixed
        ));
        app->Add(new JChainMultifactoryGeneratorT<CalorimeterTruthClustering_factoryT>(
          "ZDCTruthProtoClusters", {"ZDCRecHits", "ZDCHits"}, {"ZDCTruthProtoClusters"},
          app   // TODO: Remove me once fixed
        ));
        app->Add(new JChainMultifactoryGeneratorT<CalorimeterIslandCluster_factoryT>(
          "ZDCIslandProtoClusters", {"ZDCRecHits"}, {"ZDCIslandProtoClusters"},
          {
            .sectorDist = 5.0 * dd4hep::cm,
            .localDistXY = {50 * dd4hep::cm, 50 * dd4hep::cm},
            .dimScaledLocalDistXY = {50.0*dd4hep::mm, 50.0*dd4hep::mm},
            .splitCluster = true,
            .minClusterHitEdep = 0.1 * dd4hep::MeV,
            .minClusterCenterEdep = 3.0 * dd4hep::MeV,
            .transverseEnergyProfileMetric = "globalDistEtaPhi",
            .transverseEnergyProfileScale = 1.,
          },
          app   // TODO: Remove me once fixed
        ));

        app->Add(
          new JChainMultifactoryGeneratorT<CalorimeterClusterRecoCoG_factoryT>(
             "ZDCTruthClusters",
            {"ZDCTruthProtoClusters",        // edm4eic::ProtoClusterCollection
             "ZDCHits"},                     // edm4hep::SimCalorimeterHitCollection
            {"ZDCTruthClusters",             // edm4eic::Cluster
             "ZDCTruthClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
            {
              .energyWeight = "log",
              .sampFrac = 1.0,
              .logWeightBase = 3.6,
              .enableEtaBounds = false
            },
            app   // TODO: Remove me once fixed
          )
        );

        app->Add(
          new JChainMultifactoryGeneratorT<CalorimeterClusterRecoCoG_factoryT>(
             "ZDCClusters",
            {"ZDCIslandProtoClusters",  // edm4eic::ProtoClusterCollection
             "ZDCHits"},                // edm4hep::SimCalorimeterHitCollection
            {"ZDCClusters",             // edm4eic::Cluster
             "ZDCClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
            {
              .energyWeight = "log",
              .sampFrac = 1.0,
              .logWeightBase = 6.2,
              .enableEtaBounds = false,
            },
            app   // TODO: Remove me once fixed
          )
        );
    }
}
