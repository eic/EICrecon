// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include "extensions/jana/JChainFactoryGeneratorT.h"
#include "extensions/jana/JChainMultifactoryGeneratorT.h"

#include "factories/calorimetry/CalorimeterClusterRecoCoG_factoryT.h"
#include "factories/calorimetry/CalorimeterHitDigi_factoryT.h"
#include "factories/calorimetry/CalorimeterHitReco_factoryT.h"

#include "ProtoCluster_factory_HcalBarrelTruthProtoClusters.h"
#include "ProtoCluster_factory_HcalBarrelIslandProtoClusters.h"


namespace eicrecon {
  using RawCalorimeterHit_factory_HcalBarrelRawHits = CalorimeterHitDigi_factoryT<>;
  using CalorimeterHit_factory_HcalBarrelRecHits = CalorimeterHitReco_factoryT<>;
  using Cluster_factory_HcalBarrelTruthClusters = CalorimeterClusterRecoCoG_factoryT<>;
  using Cluster_factory_HcalBarrelClusters = CalorimeterClusterRecoCoG_factoryT<>;
}

extern "C" {
    void InitPlugin(JApplication *app) {

        using namespace eicrecon;

        InitJANAPlugin(app);

        app->Add(new JChainMultifactoryGeneratorT<RawCalorimeterHit_factory_HcalBarrelRawHits>(
          "HcalBarrelRawHits", {"HcalBarrelHits"}, {"HcalBarrelRawHits"},
          {
            .eRes = {},
            .tRes = 0.0 * dd4hep::ns,
            .capADC = 65536,
            .capTime = 100, // given in ns, 4 samples in HGCROC
            .dyRangeADC = 1.0 * dd4hep::GeV,
            .pedMeanADC = 10,
            .pedSigmaADC = 2.0,
            .resolutionTDC = 1.0 * dd4hep::picosecond,
            .corrMeanScale = 1.0,
            .readout = "HcalBarrelHits",
          },
          app   // TODO: Remove me once fixed
	));
        app->Add(new JChainMultifactoryGeneratorT<CalorimeterHit_factory_HcalBarrelRecHits>(
          "HcalBarrelRecHits", {"HcalBarrelRawHits"}, {"HcalBarrelRecHits"},
          {
            .capADC = 65536,
            .dyRangeADC = 1.0 * dd4hep::GeV,
            .pedMeanADC = 10,
            .pedSigmaADC = 2.0,
            .resolutionTDC = 1.0 * dd4hep::picosecond,
            .thresholdFactor = 5.0,
            .thresholdValue = 1.0,
            .sampFrac = 0.033, // average, from sPHENIX simulations
            .readout = "HcalBarrelHits",
            .layerField = "tower",
            .sectorField = "sector",
          },
          app   // TODO: Remove me once fixed
	));
        app->Add(new JChainFactoryGeneratorT<ProtoCluster_factory_HcalBarrelTruthProtoClusters>(
	    {"HcalBarrelRecHits", "HcalBarrelHits"}, "HcalBarrelTruthProtoClusters"
        ));
        app->Add(new JChainFactoryGeneratorT<ProtoCluster_factory_HcalBarrelIslandProtoClusters>(
	    {"HcalBarrelRecHits"}, "HcalBarrelIslandProtoClusters"
	));

        app->Add(
          new JChainMultifactoryGeneratorT<Cluster_factory_HcalBarrelClusters>(
             "HcalBarrelClusters",
            {"HcalBarrelIslandProtoClusters",  // edm4eic::ProtoClusterCollection
             "HcalBarrelHits"},                // edm4hep::SimCalorimeterHitCollection
            {"HcalBarrelClusters",             // edm4eic::Cluster
             "HcalBarrelClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
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

        app->Add(
          new JChainMultifactoryGeneratorT<Cluster_factory_HcalBarrelTruthClusters>(
             "HcalBarrelTruthClusters",
            {"HcalBarrelTruthProtoClusters",        // edm4eic::ProtoClusterCollection
             "HcalBarrelHits"},                     // edm4hep::SimCalorimeterHitCollection
            {"HcalBarrelTruthClusters",             // edm4eic::Cluster
             "HcalBarrelTruthClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
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
