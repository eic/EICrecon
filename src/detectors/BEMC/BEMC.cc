// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplication.h>
#include <math.h>
#include <string>

#include "algorithms/interfaces/WithPodConfig.h"
#include "extensions/jana/JChainMultifactoryGeneratorT.h"
#include "factories/calorimetry/CalorimeterClusterRecoCoG_factoryT.h"
#include "factories/calorimetry/CalorimeterHitDigi_factoryT.h"
#include "factories/calorimetry/CalorimeterHitReco_factoryT.h"
#include "factories/calorimetry/CalorimeterIslandCluster_factoryT.h"
#include "factories/calorimetry/EnergyPositionClusterMerger_factoryT.h"
#include "factories/calorimetry/ImagingClusterReco_factoryT.h"
#include "factories/calorimetry/ImagingTopoCluster_factoryT.h"
#include "factories/calorimetry/TruthEnergyPositionClusterMerger_factoryT.h"


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
             .capADC = 16384, // 14bit ADC
             .dyRangeADC = 1500 * dd4hep::MeV,
             .pedMeanADC = 100,
             .pedSigmaADC = 1,
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
            .dyRangeADC = 1500. * dd4hep::MeV,
            .pedMeanADC = 100,
            .resolutionTDC = 10 * dd4hep::picosecond,
            .thresholdValue = 5.0, // 16384 ADC counts/1500 MeV * 0.5 MeV (desired threshold) = 5.46
            .sampFrac = 0.10200085,
            .readout = "EcalBarrelScFiHits",
            .layerField = "layer",
            .sectorField = "sector",
            .localDetFields = {"system"},
            // here we want to use grid center position (XY) but keeps the z information from fiber-segment
            // TODO: a more realistic way to get z is to reconstruct it from timing
            .maskPos = "xy",
            .maskPosFields = {"fiber", "z"},
          },
          app   // TODO: Remove me once fixed
        ));
        app->Add(new JChainMultifactoryGeneratorT<CalorimeterIslandCluster_factoryT>(
          "EcalBarrelScFiProtoClusters", {"EcalBarrelScFiRecHits"}, {"EcalBarrelScFiProtoClusters"},
          {
            .sectorDist = 50. * dd4hep::mm,
            .localDistXZ = {40 * dd4hep::mm, 40 * dd4hep::mm},
            .splitCluster = false,
            .minClusterHitEdep = 5.0 * dd4hep::MeV,
            .minClusterCenterEdep = 100.0 * dd4hep::MeV,
          },
          app   // TODO: Remove me once fixed
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
             .pedMeanADC = 14, // Noise floor at 5 keV: 8192 / 3 * 0.005
             .pedSigmaADC = 5, // Upper limit for sigma for AstroPix
             .resolutionTDC = 3.25 * dd4hep::nanosecond,
             .corrMeanScale = 1.0,
           },
           app   // TODO: Remove me once fixed
        ));
        app->Add(new JChainMultifactoryGeneratorT<CalorimeterHitReco_factoryT>(
          "EcalBarrelImagingRecHits", {"EcalBarrelImagingRawHits"}, {"EcalBarrelImagingRecHits"},
          {
            .capADC = 8192,
            .dyRangeADC = 3 * dd4hep::MeV,
            .pedMeanADC = 14,
            .resolutionTDC = 3.25 * dd4hep::nanosecond,
            .thresholdValue = 41, // 8192 ADC counts/3 MeV * 0.015 MeV (desired threshold) = 41
            .sampFrac = 0.00619766,
            .readout = "EcalBarrelImagingHits",
            .layerField = "layer",
            .sectorField = "sector",
          },
           app   // TODO: Remove me once fixed
        ));
        app->Add(new JChainMultifactoryGeneratorT<ImagingTopoCluster_factoryT>(
          "EcalBarrelImagingProtoClusters", {"EcalBarrelImagingRecHits"}, {"EcalBarrelImagingProtoClusters"},
          {
            .neighbourLayersRange = 2,                    //  # id diff for adjacent layer
            .localDistXY          = {2.0 * dd4hep::mm, 2 * dd4hep::mm},     //  # same layer
            .layerDistEtaPhi      = {10 * dd4hep::mrad, 10 * dd4hep::mrad}, //  # adjacent layer
            .sectorDist           = 3.0 * dd4hep::cm,
            .minClusterHitEdep    = 0,
            .minClusterCenterEdep = 0,
            .minClusterEdep       = 100 * dd4hep::MeV,
            .minClusterNhits      = 10,
          },
          app   // TODO: Remove me once fixed
        ));

        app->Add(new JChainMultifactoryGeneratorT<ImagingClusterReco_factoryT>(
           "EcalBarrelImagingClusters",
          {"EcalBarrelImagingProtoClusters",
           "EcalBarrelImagingHits"},
          {"EcalBarrelImagingClusters",
           "EcalBarrelImagingClusterAssociations",
           "EcalBarrelImagingLayers"
          },
          {
            .trackStopLayer = 6,
          },
          app   // TODO: Remove me once fixed
        ));
        app->Add(new JChainMultifactoryGeneratorT<EnergyPositionClusterMerger_factoryT>(
          "EcalBarrelClusters",
          {
            "EcalBarrelScFiClusters",
            "EcalBarrelScFiClusterAssociations",
            "EcalBarrelImagingClusters",
            "EcalBarrelImagingClusterAssociations"
          },
          {
            "EcalBarrelClusters",
            "EcalBarrelClusterAssociations"
          },
          {
            .energyRelTolerance = 0.5,
            .phiTolerance = 0.1,
            .etaTolerance = 0.2,
          },
          app   // TODO: Remove me once fixed
        ));
        app->Add(new JChainMultifactoryGeneratorT<TruthEnergyPositionClusterMerger_factoryT>(
          "EcalBarrelTruthClusters",
          {
            "MCParticles",
            "EcalBarrelScFiClusters",
            "EcalBarrelScFiClusterAssociations",
            "EcalBarrelImagingClusters",
            "EcalBarrelImagingClusterAssociations"
          },
          {
            "EcalBarrelTruthClusters",
            "EcalBarrelTruthClusterAssociations"
          },
          app   // TODO: Remove me once fixed
        ));
    }
}
