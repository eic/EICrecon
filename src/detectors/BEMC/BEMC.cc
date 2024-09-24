// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <edm4eic/EDM4eicVersion.h>
#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplication.h>
#include <math.h>
#include <string>

#include "algorithms/calorimetry/CalorimeterHitDigiConfig.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/calorimetry/CalorimeterClusterRecoCoG_factory.h"
#include "factories/calorimetry/CalorimeterHitDigi_factory.h"
#include "factories/calorimetry/CalorimeterHitReco_factory.h"
#include "factories/calorimetry/CalorimeterIslandCluster_factory.h"
#include "factories/calorimetry/EnergyPositionClusterMerger_factory.h"
#include "factories/calorimetry/ImagingClusterReco_factory.h"
#include "factories/calorimetry/ImagingTopoCluster_factory.h"
#include "factories/calorimetry/TruthEnergyPositionClusterMerger_factory.h"


extern "C" {
    void InitPlugin(JApplication *app) {

        using namespace eicrecon;

        InitJANAPlugin(app);


        // Make sure digi and reco use the same value
        decltype(CalorimeterHitDigiConfig::capADC)        EcalBarrelScFi_capADC = 16384; //16384,  14bit ADC
        decltype(CalorimeterHitDigiConfig::dyRangeADC)    EcalBarrelScFi_dyRangeADC = 1500 * dd4hep::MeV;
        decltype(CalorimeterHitDigiConfig::pedMeanADC)    EcalBarrelScFi_pedMeanADC = 100;
        decltype(CalorimeterHitDigiConfig::pedSigmaADC)   EcalBarrelScFi_pedSigmaADC = 1;
        decltype(CalorimeterHitDigiConfig::resolutionTDC) EcalBarrelScFi_resolutionTDC = 10 * dd4hep::picosecond;
        app->Add(new JOmniFactoryGeneratorT<CalorimeterHitDigi_factory>(
           "EcalBarrelScFiRawHits",
           {"EcalBarrelScFiHits"},
#if EDM4EIC_VERSION_MAJOR >= 7
           {"EcalBarrelScFiRawHits", "EcalBarrelScFiRawHitAssociations"},
#else
           {"EcalBarrelScFiRawHits"},
#endif
           {
             .eRes = {0.0 * sqrt(dd4hep::GeV), 0.0, 0.0 * dd4hep::GeV},
             .tRes = 0.0 * dd4hep::ns,
             .threshold = 0.0*dd4hep::keV, // threshold is set in ADC in reco
             .capADC        = EcalBarrelScFi_capADC,
             .dyRangeADC    = EcalBarrelScFi_dyRangeADC,
             .pedMeanADC    = EcalBarrelScFi_pedMeanADC,
             .pedSigmaADC   = EcalBarrelScFi_pedSigmaADC,
             .resolutionTDC = EcalBarrelScFi_resolutionTDC,
             .corrMeanScale = "1.0",
             .readout = "EcalBarrelScFiHits",
             .fields = {"fiber", "z"},
           },
           app   // TODO: Remove me once fixed
        ));
        app->Add(new JOmniFactoryGeneratorT<CalorimeterHitReco_factory>(
          "EcalBarrelScFiRecHits", {"EcalBarrelScFiRawHits"}, {"EcalBarrelScFiRecHits"},
          {
            .capADC        = EcalBarrelScFi_capADC,
            .dyRangeADC    = EcalBarrelScFi_dyRangeADC,
            .pedMeanADC    = EcalBarrelScFi_pedMeanADC,
            .pedSigmaADC   = EcalBarrelScFi_pedSigmaADC, // not needed; use only thresholdValue
            .resolutionTDC = EcalBarrelScFi_resolutionTDC,
            .thresholdFactor = 0.0, // use only thresholdValue
            .thresholdValue = 5.0, // 16384 ADC counts/1500 MeV * 0.5 MeV (desired threshold) = 5.46
            .sampFrac = "0.10200085",
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
        app->Add(new JOmniFactoryGeneratorT<CalorimeterIslandCluster_factory>(
          "EcalBarrelScFiProtoClusters", {"EcalBarrelScFiRecHits"}, {"EcalBarrelScFiProtoClusters"},
          {
            .sectorDist = 50. * dd4hep::mm,
            .localDistXZ = {80 * dd4hep::mm, 80 * dd4hep::mm},
            .splitCluster = false,
            .minClusterHitEdep = 5.0 * dd4hep::MeV,
            .minClusterCenterEdep = 100.0 * dd4hep::MeV,
          },
          app   // TODO: Remove me once fixed
        ));
        app->Add(
          new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
             "EcalBarrelScFiClusters",
            {"EcalBarrelScFiProtoClusters",        // edm4eic::ProtoClusterCollection
#if EDM4EIC_VERSION_MAJOR >= 7
             "EcalBarrelScFiRawHitAssociations"},  // edm4eic::MCRecoCalorimeterHitAssociation
#else
             "EcalBarrelScFiHits"},                // edm4hep::SimCalorimeterHitCollection
#endif
            {"EcalBarrelScFiClusters",             // edm4eic::Cluster
             "EcalBarrelScFiClusterAssociations"}, // edm4eic::MCRecoClusterParticleAssociation
             {
               .energyWeight = "log",
               .sampFrac = 1.0,
               .logWeightBase = 6.2,
               .longitudinalShowerInfoAvailable = true,
               .enableEtaBounds = false
             },
            app   // TODO: Remove me once fixed
          )
        );

        // Make sure digi and reco use the same value
        decltype(CalorimeterHitDigiConfig::capADC)        EcalBarrelImaging_capADC = 8192; //8192,  13bit ADC
        decltype(CalorimeterHitDigiConfig::dyRangeADC)    EcalBarrelImaging_dyRangeADC = 3 * dd4hep::MeV;
        decltype(CalorimeterHitDigiConfig::pedMeanADC)    EcalBarrelImaging_pedMeanADC = 14; // Noise floor at 5 keV: 8192 / 3 * 0.005
        decltype(CalorimeterHitDigiConfig::pedSigmaADC)   EcalBarrelImaging_pedSigmaADC = 5; // Upper limit for sigma for AstroPix
        decltype(CalorimeterHitDigiConfig::resolutionTDC) EcalBarrelImaging_resolutionTDC = 3.25 * dd4hep::nanosecond;
        app->Add(new JOmniFactoryGeneratorT<CalorimeterHitDigi_factory>(
           "EcalBarrelImagingRawHits",
          {"EcalBarrelImagingHits"},
#if EDM4EIC_VERSION_MAJOR >= 7
          {"EcalBarrelImagingRawHits", "EcalBarrelImagingRawHitAssociations"},
#else
          {"EcalBarrelImagingRawHits"},
#endif
          {
             .eRes = {0.0 * sqrt(dd4hep::GeV), 0.02, 0.0 * dd4hep::GeV},
             .tRes = 0.0 * dd4hep::ns,
             .capADC        = EcalBarrelImaging_capADC,
             .dyRangeADC    = EcalBarrelImaging_dyRangeADC,
             .pedMeanADC    = EcalBarrelImaging_pedMeanADC,
             .pedSigmaADC   = EcalBarrelImaging_pedSigmaADC,
             .resolutionTDC = EcalBarrelImaging_resolutionTDC,
             .corrMeanScale = "1.0",
             .readout = "EcalBarrelImagingHits",
           },
           app   // TODO: Remove me once fixed
        ));
        app->Add(new JOmniFactoryGeneratorT<CalorimeterHitReco_factory>(
          "EcalBarrelImagingRecHits", {"EcalBarrelImagingRawHits"}, {"EcalBarrelImagingRecHits"},
          {
            .capADC     = EcalBarrelImaging_capADC,
            .dyRangeADC = EcalBarrelImaging_dyRangeADC,
            .pedMeanADC    = EcalBarrelImaging_pedMeanADC,
            .pedSigmaADC   = EcalBarrelImaging_pedSigmaADC, // not needed; use only thresholdValue
            .resolutionTDC = EcalBarrelImaging_resolutionTDC,
            .thresholdFactor = 0.0, // use only thresholdValue
            .thresholdValue = 41, // 8192 ADC counts/3 MeV * 0.015 MeV (desired threshold) = 41
            .sampFrac = "0.00619766",
            .readout = "EcalBarrelImagingHits",
            .layerField = "layer",
            .sectorField = "sector",
          },
           app   // TODO: Remove me once fixed
        ));
        app->Add(new JOmniFactoryGeneratorT<ImagingTopoCluster_factory>(
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

        app->Add(new JOmniFactoryGeneratorT<ImagingClusterReco_factory>(
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
        app->Add(new JOmniFactoryGeneratorT<EnergyPositionClusterMerger_factory>(
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
        app->Add(new JOmniFactoryGeneratorT<TruthEnergyPositionClusterMerger_factory>(
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
