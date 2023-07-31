// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include "extensions/jana/JChainFactoryGeneratorT.h"
#include "extensions/jana/JChainMultifactoryGeneratorT.h"

#include "factories/calorimetry/CalorimeterClusterRecoCoG_factoryT.h"
#include "factories/calorimetry/CalorimeterHitDigi_factoryT.h"
#include "factories/calorimetry/CalorimeterHitReco_factoryT.h"
#include "factories/calorimetry/CalorimeterTruthClustering_factoryT.h"

#include "ProtoCluster_factory_EcalLumiSpecIslandProtoClusters.h"

extern "C" {
    void InitPlugin(JApplication *app) {

        using namespace eicrecon;

        InitJANAPlugin(app);

        app->Add(new JChainMultifactoryGeneratorT<CalorimeterHitDigi_factoryT>(
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
        app->Add(new JChainMultifactoryGeneratorT<CalorimeterHitReco_factoryT>(
          "EcalLumiSpecRecHits", {"EcalLumiSpecRawHits"}, {"EcalLumiSpecRecHits"},
          {
            .capADC = 16384,
            .dyRangeADC = 20. * dd4hep::GeV,
            .pedMeanADC = 100,
            .pedSigmaADC = 1,
            .resolutionTDC = 10 * dd4hep::picosecond,
            .thresholdFactor = 4.0,
            .thresholdValue = 3.0,
            .sampFrac = 1.0,
            .readout = "LumiSpecCALHits",
            .sectorField = "sector",
          },
          app   // TODO: Remove me once fixed
        ));
        app->Add(new JChainMultifactoryGeneratorT<CalorimeterTruthClustering_factoryT>(
          "EcalLumiSpecTruthProtoClusters", {"EcalLumiSpecRecHits", "LumiSpecCALHits"}, {"EcalLumiSpecTruthProtoClusters"},
          app   // TODO: Remove me once fixed
        ));
        app->Add(new JChainFactoryGeneratorT<ProtoCluster_factory_EcalLumiSpecIslandProtoClusters>(
          {"EcalLumiSpecRecHits"}, "EcalLumiSpecIslandProtoClusters"
        ));

        app->Add(
          new JChainMultifactoryGeneratorT<CalorimeterClusterRecoCoG_factoryT>(
             "EcalLumiSpecClusters",
            {"EcalLumiSpecIslandProtoClusters",  // edm4eic::ProtoClusterCollection
             "LumiSpecCALHits"},                 // edm4hep::SimCalorimeterHitCollection
            {"EcalLumiSpecClusters",             // edm4eic::Cluster
             "EcalLumiSpecClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
            {
              .energyWeight = "log",
              .moduleDimZName = "",
              .sampFrac = 1.0,
              .logWeightBase = 3.6,
              .depthCorrection = 0.0,
              .enableEtaBounds = false
            },
            app   // TODO: Remove me once fixed
          )
        );

        app->Add(
          new JChainMultifactoryGeneratorT<CalorimeterClusterRecoCoG_factoryT>(
             "EcalLumiSpecTruthClusters",
            {"EcalLumiSpecTruthProtoClusters",        // edm4eic::ProtoClusterCollection
             "LumiSpecCALHits"},                      // edm4hep::SimCalorimeterHitCollection
            {"EcalLumiSpecTruthClusters",             // edm4eic::Cluster
             "EcalLumiSpecTruthClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
            {
              .energyWeight = "log",
              .moduleDimZName = "",
              .sampFrac = 1.0,
              .logWeightBase = 4.6,
              .depthCorrection = 0.0,
              .enableEtaBounds = false
            },
            app   // TODO: Remove me once fixed
          )
        );
    }
}
