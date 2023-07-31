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

#include "ProtoCluster_factory_ZDCEcalIslandProtoClusters.h"

extern "C" {
    void InitPlugin(JApplication *app) {

        using namespace eicrecon;

        InitJANAPlugin(app);

        app->Add(new JChainMultifactoryGeneratorT<CalorimeterHitDigi_factoryT>(
	  "ZDCEcalRawHits", {"ZDCEcalHits"}, {"ZDCEcalRawHits"},
          {
            .tRes = 0.0 * dd4hep::ns,
            .capADC = 8096,
            .dyRangeADC = 100 * dd4hep::MeV,
            .pedMeanADC = 400,
            .pedSigmaADC = 3.2,
            .resolutionTDC = 10 * dd4hep::picosecond,
            .corrMeanScale = 1.0,
          },
          app   // TODO: Remove me once fixed
	));
        app->Add(new JChainMultifactoryGeneratorT<CalorimeterHitReco_factoryT>(
	  "ZDCEcalRecHits", {"ZDCEcalRawHits"}, {"ZDCEcalRecHits"},
          {
            .capADC = 8096,
            .dyRangeADC = 100. * dd4hep::MeV,
            .pedMeanADC = 400,
            .pedSigmaADC = 3.2,
            .resolutionTDC = 10 * dd4hep::picosecond,
            .thresholdFactor = 4.0,
            .thresholdValue = 0.0,
            .sampFrac = 1.0,
            .readout = "ZDCEcalHits",
          },
          app   // TODO: Remove me once fixed
        ));
        app->Add(new JChainMultifactoryGeneratorT<CalorimeterTruthClustering_factoryT>(
          "ZDCEcalTruthProtoClusters", {"ZDCEcalRecHits", "ZDCEcalHits"}, {"ZDCEcalTruthProtoClusters"},
          app   // TODO: Remove me once fixed
        ));
        app->Add(new JChainFactoryGeneratorT<ProtoCluster_factory_ZDCEcalIslandProtoClusters>(
	  {"ZDCEcalRecHits"}, "ZDCEcalIslandProtoClusters"
        ));

        app->Add(
          new JChainMultifactoryGeneratorT<CalorimeterClusterRecoCoG_factoryT>(
             "ZDCEcalTruthClusters",
            {"ZDCEcalTruthProtoClusters",        // edm4eic::ProtoClusterCollection
             "ZDCEcalHits"},                     // edm4hep::SimCalorimeterHitCollection
            {"ZDCEcalTruthClusters",             // edm4eic::Cluster
             "ZDCEcalTruthClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
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
             "ZDCEcalClusters",
            {"ZDCEcalIslandProtoClusters",  // edm4eic::ProtoClusterCollection
             "ZDCEcalHits"},                // edm4hep::SimCalorimeterHitCollection
            {"ZDCEcalClusters",             // edm4eic::Cluster
             "ZDCEcalClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
            {
              .energyWeight = "log",
              .moduleDimZName = "",
              .sampFrac = 1.0,
              .logWeightBase = 6.2,
              .depthCorrection = 0.0,
              .enableEtaBounds = false,
            },
            app   // TODO: Remove me once fixed
          )
        );
    }
}
