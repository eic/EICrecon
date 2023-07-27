// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include "extensions/jana/JChainFactoryGeneratorT.h"
#include "extensions/jana/JChainMultifactoryGeneratorT.h"

#include "factories/calorimetry/CalorimeterClusterRecoCoG_factoryT.h"
#include "factories/calorimetry/CalorimeterHitDigi_factoryT.h"
#include "factories/calorimetry/CalorimeterHitReco_factoryT.h"
#include "factories/calorimetry/CalorimeterHitsMerger_factoryT.h"
#include "factories/calorimetry/CalorimeterTruthClustering_factoryT.h"

#include "ProtoCluster_factory_EcalBarrelScFiProtoClusters.h"

#include "CalorimeterHit_factory_EcalBarrelImagingRecHits.h"
#include "ProtoCluster_factory_EcalBarrelImagingProtoClusters.h"
#include "Cluster_factory_EcalBarrelImagingClusters.h"
#include "Cluster_factory_EcalBarrelImagingMergedClusters.h"

extern "C" {
    void InitPlugin(JApplication *app) {

        using namespace eicrecon;

        InitJANAPlugin(app);

        app->Add(new JChainMultifactoryGeneratorT<CalorimeterHitDigi_factoryT>(
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
        app->Add(new JChainMultifactoryGeneratorT<CalorimeterHitReco_factoryT>(
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
          new JChainMultifactoryGeneratorT<CalorimeterClusterRecoCoG_factoryT>(
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

        app->Add(new JChainMultifactoryGeneratorT<CalorimeterHitDigi_factoryT>(
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
        app->Add(new JChainMultifactoryGeneratorT<ProtoCluster_factory_EcalBarrelImagingProtoClusters>(
          "EcalBarrelImagingProtoClusters", {"EcalBarrelImagingRecHits"}, {"EcalBarrelImagingProtoClusters"},
          {
            .neighbourLayersRange = 2,                    //  # id diff for adjacent layer
            .localDistXY          = {2.0 * dd4hep::mm, 2 * dd4hep::mm},     //  # same layer
            .layerDistEtaPhi      = {10 * dd4hep::mrad, 10 * dd4hep::mrad}, //  # adjacent layer
            .sectorDist           = 3.0 * dd4hep::cm,
            .minClusterHitEdep    = 0,
            .minClusterCenterEdep = 0,
            .minClusterEdep       = 100 * dd4hep::MeV,
            .minClusterNhits      = 10, // From Maria Z. comment in PR
          },
          app   // TODO: Remove me once fixed
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
    }
}
