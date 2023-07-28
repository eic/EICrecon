// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include "extensions/jana/JChainFactoryGeneratorT.h"
#include "extensions/jana/JChainMultifactoryGeneratorT.h"

#include "factories/calorimetry/CalorimeterClusterRecoCoG_factoryT.h"
#include "factories/calorimetry/CalorimeterHitDigi_factoryT.h"
#include "factories/calorimetry/CalorimeterHitReco_factoryT.h"

#include "ProtoCluster_factory_EcalBarrelSciGlassTruthProtoClusters.h"
#include "ProtoCluster_factory_EcalBarrelSciGlassProtoClusters.h"
#include "Cluster_factory_EcalBarrelSciGlassMergedTruthClusters.h"

#include "CalorimeterHit_factory_EcalBarrelScFiMergedHits.h"
#include "ProtoCluster_factory_EcalBarrelScFiProtoClusters.h"

#include "CalorimeterHit_factory_EcalBarrelImagingRecHits.h"
#include "ProtoCluster_factory_EcalBarrelImagingProtoClusters.h"
#include "Cluster_factory_EcalBarrelImagingClusters.h"
#include "Cluster_factory_EcalBarrelImagingMergedClusters.h"

namespace eicrecon {
  using RawCalorimeterHit_factory_EcalBarrelSciGlassRawHits = CalorimeterHitDigi_factoryT<>;
  using RawCalorimeterHit_factory_EcalBarrelScFiRawHits = CalorimeterHitDigi_factoryT<>;
  using RawCalorimeterHit_factory_EcalBarrelImagingRawHits = CalorimeterHitDigi_factoryT<>;
  using CalorimeterHit_factory_EcalBarrelSciGlassRecHits = CalorimeterHitReco_factoryT<>;
  using CalorimeterHit_factory_EcalBarrelScFiRecHits = CalorimeterHitReco_factoryT<>;
  using Cluster_factory_EcalBarrelSciGlassTruthClusters = CalorimeterClusterRecoCoG_factoryT<>;
  using Cluster_factory_EcalBarrelSciGlassClusters = CalorimeterClusterRecoCoG_factoryT<>;
  using Cluster_factory_EcalBarrelScFiClusters = CalorimeterClusterRecoCoG_factoryT<>;
}

extern "C" {
    void InitPlugin(JApplication *app) {

        using namespace eicrecon;

        InitJANAPlugin(app);

        app->Add(new JChainMultifactoryGeneratorT<RawCalorimeterHit_factory_EcalBarrelSciGlassRawHits>(
           "EcalBarrelSciGlassRawHits",
           {"EcalBarrelSciGlassHits"},
           {"EcalBarrelSciGlassRawHits"},
           {
             .eRes = {0.0 * sqrt(dd4hep::GeV), 0.0, 0.0 * dd4hep::GeV},
             .tRes = 0.0 * dd4hep::ns,
             .capADC = 16384,
             .dyRangeADC = 20 * dd4hep::GeV,
             .pedMeanADC = 100,
             .pedSigmaADC = 1,
             .resolutionTDC = 10 * dd4hep::picosecond,
             .corrMeanScale = 1.0,
             .fields = {},
           },
           app   // TODO: Remove me once fixed
        ));
        app->Add(new JChainMultifactoryGeneratorT<CalorimeterHit_factory_EcalBarrelSciGlassRecHits>(
          "EcalBarrelSciGlassRecHits", {"EcalBarrelSciGlassRawHits"}, {"EcalBarrelSciGlassRecHits"},
          {
            .capADC = 16384,
            .dyRangeADC = 20. * dd4hep::GeV,
            .pedMeanADC = 100,
            .pedSigmaADC = 1,
            .resolutionTDC = 10 * dd4hep::picosecond,
            .thresholdFactor = 3.0,
            .thresholdValue = 3.0,
            .sampFrac = 0.98,
            .readout = "EcalBarrelSciGlassHits",
            .sectorField = "sector",
          },
          app   // TODO: Remove me once fixed
        ));
        app->Add(new JChainFactoryGeneratorT<ProtoCluster_factory_EcalBarrelSciGlassProtoClusters>(
          {"EcalBarrelSciGlassRecHits"}, "EcalBarrelSciGlassProtoClusters"
        ));
        app->Add(
          new JChainMultifactoryGeneratorT<Cluster_factory_EcalBarrelSciGlassClusters>(
             "EcalBarrelSciGlassClusters",
            {"EcalBarrelSciGlassProtoClusters",        // edm4eic::ProtoClusterCollection
             "EcalBarrelSciGlassHits"},                // edm4hep::SimCalorimeterHitCollection
            {"EcalBarrelSciGlassClusters",             // edm4eic::Cluster
             "EcalBarrelSciGlassClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
            {
              .energyWeight = "log",
              .moduleDimZName = "",
              .sampFrac = 0.92,
              .logWeightBase = 6.2,
              .depthCorrection = 0.0,
              .enableEtaBounds = true
            },
            app   // TODO: Remove me once fixed
          )
        );


        app->Add(new JChainMultifactoryGeneratorT<RawCalorimeterHit_factory_EcalBarrelScFiRawHits>(
           "EcalBarrelScFiRawHits",
           {"EcalBarrelScFiHits"},
           {"EcalBarrelScFiRawHits"},
           {
             .eRes = {0.0 * sqrt(dd4hep::GeV), 0.0, 0.0 * dd4hep::GeV},
             .tRes = 0.0 * dd4hep::ns,
             .capADC = 16384,
             .dyRangeADC = 750 * dd4hep::MeV,
             .pedMeanADC = 20,
             .pedSigmaADC = 0.3,
             .resolutionTDC = 10 * dd4hep::picosecond,
             .corrMeanScale = 1.0,
             .readout = "EcalBarrelScFiHits",
             .fields = {"fiber", "z"},
           },
           app   // TODO: Remove me once fixed
        ));
        app->Add(new JChainMultifactoryGeneratorT<CalorimeterHit_factory_EcalBarrelScFiRecHits>(
          "EcalBarrelScFiRecHits", {"EcalBarrelScFiRawHits"}, {"EcalBarrelScFiRecHits"},
          {
            .capADC = 16384,
            .dyRangeADC = 750. * dd4hep::MeV,
            .pedMeanADC = 20,
            .pedSigmaADC = 0.3,
            .resolutionTDC = 10 * dd4hep::picosecond,
            .thresholdFactor = 36.0488,
            .thresholdValue = 0.0,
            .sampFrac = 0.10200085,
            .readout = "EcalBarrelScFiHits",
            .layerField = "layer",
            .sectorField = "module",
            .localDetFields = {"system"},
            // here we want to use grid center position (XY) but keeps the z information from fiber-segment
            // TODO: a more realistic way to get z is to reconstruct it from timing
            .maskPos = "xy",
            .maskPosFields = {"fiber", "z"},
          },
          app   // TODO: Remove me once fixed
        ));
        app->Add(new JChainFactoryGeneratorT<ProtoCluster_factory_EcalBarrelScFiProtoClusters>(
          {"EcalBarrelScFiRecHits"}, "EcalBarrelScFiProtoClusters"
        ));
        app->Add(
          new JChainMultifactoryGeneratorT<Cluster_factory_EcalBarrelScFiClusters>(
             "EcalBarrelScFiClusters",
            {"EcalBarrelScFiProtoClusters",        // edm4eic::ProtoClusterCollection
             "EcalBarrelScFiHits"},                // edm4hep::SimCalorimeterHitCollection
            {"EcalBarrelScFiClusters",             // edm4eic::Cluster
             "EcalBarrelScFiClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
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

        app->Add(new JChainMultifactoryGeneratorT<RawCalorimeterHit_factory_EcalBarrelImagingRawHits>(
           "EcalBarrelImagingRawHits",
          {"EcalBarrelImagingHits"},
          {"EcalBarrelImagingRawHits"},
          {
             .eRes = {0.0 * sqrt(dd4hep::GeV), 0.02, 0.0 * dd4hep::GeV},
             .tRes = 0.0 * dd4hep::ns,
             .capADC = 8192,
             .dyRangeADC = 3 * dd4hep::MeV,
             .pedMeanADC = 100,
             .pedSigmaADC = 14,
             .resolutionTDC = 10 * dd4hep::picosecond,
             .corrMeanScale = 1.0,
           },
           app   // TODO: Remove me once fixed
        ));
        app->Add(new JChainFactoryGeneratorT<CalorimeterHit_factory_EcalBarrelImagingRecHits>(
          {"EcalBarrelImagingRawHits"}, "EcalBarrelImagingRecHits"
        ));
        app->Add(new JChainFactoryGeneratorT<ProtoCluster_factory_EcalBarrelImagingProtoClusters>(
          {"EcalBarrelImagingRecHits"}, "EcalBarrelImagingProtoClusters"
        ));

        app->Add(new JChainFactoryGeneratorT<Cluster_factory_EcalBarrelImagingClusters>(
          {"EcalBarrelImagingProtoClusters"}, "EcalBarrelImagingClusters"
        ));
        app->Add(new JChainFactoryGeneratorT<Cluster_factory_EcalBarrelImagingMergedClusters>(
          {
            "MCParticles",
            "EcalBarrelScFiClusters",
            "EcalBarrelScFiClusterAssociations",
            "EcalBarrelImagingClusters",
            "EcalBarrelImagingClusterAssociations"
          },
          "EcalBarrelImagingMergedClusters"
        ));

        // Inserted types (so they can be written to output podio file)
        app->Add(new JFactoryGeneratorT<JFactoryT<edm4eic::Cluster>>("EcalBarrelImagingLayers"));
        app->Add(new JFactoryGeneratorT<JFactoryT<edm4eic::MCRecoClusterParticleAssociation>>("EcalBarrelImagingClusterAssociations"));

        app->Add(new JChainFactoryGeneratorT<ProtoCluster_factory_EcalBarrelSciGlassTruthProtoClusters>(
	    {
              "EcalBarrelSciGlassRecHits",
              "EcalBarrelSciGlassHits",
            },
	    "EcalBarrelSciGlassTruthProtoClusters"
	));
        app->Add(
          new JChainMultifactoryGeneratorT<Cluster_factory_EcalBarrelSciGlassTruthClusters>(
             "EcalBarrelSciGlassTruthClusters",
            {"EcalBarrelSciGlassTruthProtoClusters",        // edm4eic::ProtoClusterCollection
             "EcalBarrelSciGlassHits"},                     // edm4hep::SimCalorimeterHitCollection
            {"EcalBarrelSciGlassTruthClusters",             // edm4eic::Cluster
             "EcalBarrelSciGlassTruthClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
             {
               .energyWeight = "log",
               .moduleDimZName = "",
               .sampFrac = 1.0,
               .logWeightBase = 6.2,
               .depthCorrection = 0.0,
               .enableEtaBounds = true
             },
            app   // TODO: Remove me once fixed
          )
        );
        app->Add(new JChainFactoryGeneratorT<Cluster_factory_EcalBarrelSciGlassMergedTruthClusters>(
          {
            "EcalBarrelSciGlassTruthClusters",
            "EcalBarrelSciGlassTruthClusterAssociations"
          },
	  "EcalBarrelSciGlassMergedTruthCluster"
	));
    }
}
