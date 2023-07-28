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

#include "ProtoCluster_factory_B0ECalIslandProtoClusters.h"


namespace eicrecon {
  using RawCalorimeterHit_factory_B0ECalRawHits = CalorimeterHitDigi_factoryT<>;
  using CalorimeterHit_factory_B0ECalRecHits = CalorimeterHitReco_factoryT<>;
  using ProtoCluster_factory_B0ECalTruthProtoClusters = CalorimeterTruthClustering_factoryT<>;
  using Cluster_factory_B0ECalTruthClusters = CalorimeterClusterRecoCoG_factoryT<>;
  using Cluster_factory_B0ECalClusters = CalorimeterClusterRecoCoG_factoryT<>;
}


extern "C" {
    void InitPlugin(JApplication *app) {

        using namespace eicrecon;

        InitJANAPlugin(app);

        app->Add(new JChainMultifactoryGeneratorT<RawCalorimeterHit_factory_B0ECalRawHits>(
          "B0ECalRawHits", {"B0ECalHits"}, {"B0ECalRawHits"},
          {
            .eRes = {0.0 * sqrt(dd4hep::GeV), 0.02, 0.0 * dd4hep::GeV},
            .tRes = 0.0 * dd4hep::ns,
            .capADC = 16384,
            .dyRangeADC = 20 * dd4hep::GeV,
            .pedMeanADC = 100,
            .pedSigmaADC = 1,
            .resolutionTDC = 1e-11,
            .corrMeanScale = 1.0,
          },
          app   // TODO: Remove me once fixed
        ));
        app->Add(new JChainMultifactoryGeneratorT<CalorimeterHit_factory_B0ECalRecHits>(
          "B0ECalRecHits", {"B0ECalRawHits"}, {"B0ECalRecHits"},
          {
            .capADC = 16384,
            .dyRangeADC = 20. * dd4hep::GeV,
            .pedMeanADC = 100,
            .pedSigmaADC = 1,
            .resolutionTDC = 1e-11,
            .thresholdFactor = 4.0,
            .thresholdValue = 3.0,
            .sampFrac = 0.998,
            .readout = "B0ECalHits",
            .sectorField = "sector",
          },
          app   // TODO: Remove me once fixed
        ));
        app->Add(new JChainMultifactoryGeneratorT<ProtoCluster_factory_B0ECalTruthProtoClusters>(
          "B0ECalTruthProtoClusters", {"B0ECalRecHits", "B0ECalHits"}, {"B0ECalTruthProtoClusters"},
          app   // TODO: Remove me once fixed
        ));
        app->Add(new JChainFactoryGeneratorT<ProtoCluster_factory_B0ECalIslandProtoClusters>(
          {"B0ECalRecHits"}, "B0ECalIslandProtoClusters"
        ));

        app->Add(
          new JChainMultifactoryGeneratorT<Cluster_factory_B0ECalClusters>(
             "B0ECalClusters",
            {"B0ECalIslandProtoClusters",  // edm4eic::ProtoClusterCollection
             "B0ECalHits"},                // edm4hep::SimCalorimeterHitCollection
            {"B0ECalClusters",             // edm4eic::Cluster
             "B0ECalClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
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
          new JChainMultifactoryGeneratorT<Cluster_factory_B0ECalTruthClusters>(
             "B0ECalTruthClusters",
            {"B0ECalTruthProtoClusters",        // edm4eic::ProtoClusterCollection
             "B0ECalHits"},                     // edm4hep::SimCalorimeterHitCollection
            {"B0ECalTruthClusters",             // edm4eic::Cluster
             "B0ECalTruthClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
            {
              .energyWeight = "log",
              .moduleDimZName = "",
              .sampFrac = 1.0,
              .logWeightBase = 6.2,
              .depthCorrection = 0.0,
              .enableEtaBounds = false
            },
            app   // TODO: Remove me once fixed
          )
        );
    }
}
