// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 - 2025 Friederike Bock, Wouter Deconinck

#include <DD4hep/Detector.h>
#include <DD4hep/Readout.h>
#include <DD4hep/Segmentations.h>
#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplication.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/Utils/JTypeInfo.h>
#include <TString.h>
#include <edm4eic/unit_system.h>
#include <edm4hep/SimCalorimeterHit.h>
#include <string>
#include <variant>
#include <vector>

#include "algorithms/calorimetry/CALOROCToRawCalorimeterHitConfig.h"
#include "algorithms/calorimetry/CalorimeterHitDigiConfig.h"
#include "algorithms/calorimetry/EdepToSiPMConversionConfig.h"
#include "algorithms/calorimetry/ImagingTopoClusterConfig.h"
#include "algorithms/digi/CALOROCDigitizationConfig.h"
#include "algorithms/digi/PulseCombinerConfig.h"
#include "algorithms/digi/PulseGenerationConfig.h"
#include "algorithms/digi/PulseNoiseConfig.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/calorimetry/CalorimeterClusterRecoCoG_factory.h"
#include "factories/calorimetry/CalorimeterClusterShape_factory.h"
#include "factories/calorimetry/CALOROCToRawCalorimeterHit_factory.h"
#include "factories/calorimetry/CalorimeterHitReco_factory.h"
#include "factories/calorimetry/CalorimeterHitsMerger_factory.h"
#include "factories/calorimetry/CalorimeterIslandCluster_factory.h"
#include "factories/calorimetry/CalorimeterTruthClustering_factory.h"
#include "factories/calorimetry/EdepToSiPMConversion_factory.h"
#include "factories/calorimetry/HEXPLIT_factory.h"
#include "factories/calorimetry/ImagingTopoCluster_factory.h"
#include "factories/calorimetry/TrackClusterMergeSplitter_factory.h"
#include "factories/digi/CALOROCDigitization_factory.h"
#include "factories/digi/PulseCombiner_factory.h"
#include "factories/digi/PulseGeneration_factory.h"
#include "factories/digi/PulseNoise_factory.h"
#include "services/geometry/dd4hep/DD4hep_service.h"

extern "C" {
void InitPlugin(JApplication* app) {

  using namespace eicrecon;

  InitJANAPlugin(app);

  // Select the insert clustering path from the loaded geometry's readout segmentation.
  bool insertUsesPhysicalTiles = false;
  try {
    auto detector = app->GetService<DD4hep_service>()->detector();
    const auto insertSegmentation = detector->readout("HcalEndcapPInsertHits").segmentation();
    insertUsesPhysicalTiles       = insertSegmentation.type() == "NoSegmentation";
  } catch (...) {
    // Preserve legacy reconstruction when the insert readout is unavailable.
  }

  // FIXME: Find reasonable values for the LFHCal/Insert.
  // SiPM response.
  decltype(EdepToSiPMConversionConfig::edep_to_npe) FHCal_edep_to_npe =
      12. / (160 * dd4hep::keV);
  decltype(EdepToSiPMConversionConfig::num_effective_sipm_pixels)
      FHCal_num_effective_sipm_pixels = 7284ULL;

  // Analog pulse generation from the fired-pixel response.
  decltype(PulseGenerationConfig::pulse_shape_function) LFHCAL_pulse_shape_function = {
      "LandauPulse"};
  decltype(PulseGenerationConfig::pulse_shape_params) LFHCAL_pulse_shape_params = {
      94.10, 17 * edm4eic::unit::ns, 2.15};
  decltype(PulseGenerationConfig::ignore_thres) LFHCAL_ignore_thres = {0.10};
  decltype(PulseGenerationConfig::timestep) LFHCAL_timestep = {0.2 * edm4eic::unit::ns};
  decltype(PulseGenerationConfig::min_sampling_time) LFHCAL_min_sampling_time = {
      250 * edm4eic::unit::ns};
  decltype(PulseGenerationConfig::max_time_bins) LFHCAL_max_time_bins = {32000};

  decltype(PulseGenerationConfig::pulse_shape_function) HcalEndcapPInsert_pulse_shape_function = {
      "LandauPulse"};
  decltype(PulseGenerationConfig::pulse_shape_params) HcalEndcapPInsert_pulse_shape_params = {
      71.96, 13 * edm4eic::unit::ns, 2.15};
  decltype(PulseGenerationConfig::ignore_thres) HcalEndcapPInsert_ignore_thres = {0.10};
  decltype(PulseGenerationConfig::timestep) HcalEndcapPInsert_timestep = {
      0.2 * edm4eic::unit::ns};
  decltype(PulseGenerationConfig::min_sampling_time) HcalEndcapPInsert_min_sampling_time = {
      200 * edm4eic::unit::ns};
  decltype(PulseGenerationConfig::max_time_bins) HcalEndcapPInsert_max_time_bins = {32000};

  // Time window for combining pulses from the same channel.
  decltype(PulseCombinerConfig::minimum_separation) FHCal_minimum_separation = {
      0 * edm4eic::unit::ns};

  // Electronics noise applied after pulse combination.
  decltype(PulseNoiseConfig::poles) FHCal_poles       = {2};
  decltype(PulseNoiseConfig::variance) FHCal_variance = {0.5};
  decltype(PulseNoiseConfig::alpha) FHCal_alpha       = {0};
  decltype(PulseNoiseConfig::scale) FHCal_scale       = {5.4e-5};
  decltype(PulseNoiseConfig::pedestal) FHCal_pedestal = {1.6e-4};

  // CALOROC sampling thresholds and ADC ranges.
  decltype(CALOROCDigitizationConfig::adc_phase) FHCal_adc_phase = {
      10 * edm4eic::unit::ns};
  decltype(CALOROCDigitizationConfig::toa_thres) FHCal_toa_thres = {7};
  decltype(CALOROCDigitizationConfig::tot_thres) FHCal_tot_thres = {200};
  decltype(CALOROCDigitizationConfig::dyRangeSingleGainADC) FHCal_dyRangeSingleGainADC = {
      250};
  decltype(CALOROCDigitizationConfig::dyRangeHighGainADC) FHCal_dyRangeHighGainADC = {250};
  decltype(CALOROCDigitizationConfig::dyRangeLowGainADC) FHCal_dyRangeLowGainADC = {2500};

  // Conversion from CALOROC response into reconstructed energy.
  decltype(CALOROCToRawCalorimeterHitConfig::calorocResponseToEnergy)
      FHCal_calorocResponseToEnergy = {1 * dd4hep::keV};
  decltype(CALOROCToRawCalorimeterHitConfig::calorocTOTToEnergy)
      FHCal_calorocTOTToEnergy = {1 * dd4hep::keV / dd4hep::ns};

  // Make sure digi and reco use the same value
  decltype(CalorimeterHitDigiConfig::capADC) HcalEndcapPInsert_capADC           = 32768;
  decltype(CalorimeterHitDigiConfig::dyRangeADC) HcalEndcapPInsert_dyRangeADC   = 200 * dd4hep::MeV;
  decltype(CalorimeterHitDigiConfig::pedMeanADC) HcalEndcapPInsert_pedMeanADC   = 10;
  decltype(CalorimeterHitDigiConfig::pedSigmaADC) HcalEndcapPInsert_pedSigmaADC = 2;
  decltype(CalorimeterHitDigiConfig::resolutionTDC) HcalEndcapPInsert_resolutionTDC =
      10 * dd4hep::picosecond;

  // Convert the Insert energy deposits into fired SiPM pixels.
  app->Add(new JOmniFactoryGeneratorT<EdepToSiPMConversion_factory>(
      "HcalEndcapPInsertSiPMHits", {"EventHeader", "HcalEndcapPInsertHits"},
      {"HcalEndcapPInsertSiPMHits"},
      {
          .edep_to_npe               = FHCal_edep_to_npe,
          .num_effective_sipm_pixels = FHCal_num_effective_sipm_pixels,
      },
      app // TODO: Remove me once fixed
      ));
  // Generate an analog pulse from each Insert SiPM response hit.
  app->Add(new JOmniFactoryGeneratorT<PulseGeneration_factory<edm4hep::SimCalorimeterHit>>(
      "HcalEndcapPInsertPulses", {"HcalEndcapPInsertSiPMHits"},
      {"HcalEndcapPInsertPulses"},
      {
          .pulse_shape_function = HcalEndcapPInsert_pulse_shape_function,
          .pulse_shape_params   = HcalEndcapPInsert_pulse_shape_params,
          .ignore_thres         = HcalEndcapPInsert_ignore_thres,
          .timestep             = HcalEndcapPInsert_timestep,
          .min_sampling_time    = HcalEndcapPInsert_min_sampling_time,
          .max_time_bins        = HcalEndcapPInsert_max_time_bins,
      },
      app // TODO: Remove me once fixed
      ));
  // Combine nearby Insert pulses from the same readout channel.
  app->Add(new JOmniFactoryGeneratorT<PulseCombiner_factory>(
      "HcalEndcapPInsertCombinedPulses", {"HcalEndcapPInsertPulses"},
      {"HcalEndcapPInsertCombinedPulses"},
      {
          .minimum_separation = FHCal_minimum_separation,
      },
      app // TODO: Remove me once fixed
      ));
  // Add electronics noise to the combined Insert pulses.
  app->Add(new JOmniFactoryGeneratorT<PulseNoise_factory>(
      "HcalEndcapPInsertCombinedPulsesWithNoise",
      {"EventHeader", "HcalEndcapPInsertCombinedPulses"},
      {"HcalEndcapPInsertCombinedPulsesWithNoise"},
      {
          .poles    = FHCal_poles,
          .variance = FHCal_variance,
          .alpha    = FHCal_alpha,
          .scale    = FHCal_scale,
          .pedestal = FHCal_pedestal,
      },
      app // TODO: Remove me once fixed
      ));
  // Digitize the noisy Insert pulses with the CALOROC frontend model.
  app->Add(new JOmniFactoryGeneratorT<CALOROCDigitization_factory>(
      "HcalEndcapPInsertCALOROCHits", {"HcalEndcapPInsertCombinedPulsesWithNoise"},
      {"HcalEndcapPInsertCALOROCHits"},
      {
          .adc_phase            = FHCal_adc_phase,
          .toa_thres            = FHCal_toa_thres,
          .tot_thres            = FHCal_tot_thres,
          .dyRangeSingleGainADC = FHCal_dyRangeSingleGainADC,
          .dyRangeHighGainADC   = FHCal_dyRangeHighGainADC,
          .dyRangeLowGainADC    = FHCal_dyRangeLowGainADC,
      },
      app // TODO: Remove me once fixed
      ));
  // Convert Insert CALOROC output into raw hits for CalorimeterHitReco.
  app->Add(new JOmniFactoryGeneratorT<CALOROCToRawCalorimeterHit_factory>(
      "HcalEndcapPInsertRawHits",
      {"HcalEndcapPInsertCALOROCHits", "HcalEndcapPInsertCombinedPulsesWithNoise"},
      {"HcalEndcapPInsertRawHits", "HcalEndcapPInsertRawHitLinks",
       "HcalEndcapPInsertRawHitAssociations"},
      {
          .calorocType             = "1A",
          .calorocResponseToEnergy = FHCal_calorocResponseToEnergy,
          .calorocTOTToEnergy      = FHCal_calorocTOTToEnergy,
          .caloroc = {
              .adc_phase            = FHCal_adc_phase,
              .toa_thres            = FHCal_toa_thres,
              .tot_thres            = FHCal_tot_thres,
              .dyRangeSingleGainADC = FHCal_dyRangeSingleGainADC,
              .dyRangeHighGainADC   = FHCal_dyRangeHighGainADC,
              .dyRangeLowGainADC    = FHCal_dyRangeLowGainADC,
          },
          .capADC        = HcalEndcapPInsert_capADC,
          .dyRangeADC    = HcalEndcapPInsert_dyRangeADC,
          .pedMeanADC    = HcalEndcapPInsert_pedMeanADC,
          .resolutionTDC = HcalEndcapPInsert_resolutionTDC,
      },
      app // TODO: Remove me once fixed
      ));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterHitReco_factory>(
      "HcalEndcapPInsertRecHits", {"HcalEndcapPInsertRawHits"}, {"HcalEndcapPInsertRecHits"},
      {
          .capADC          = HcalEndcapPInsert_capADC,
          .dyRangeADC      = HcalEndcapPInsert_dyRangeADC,
          .pedMeanADC      = HcalEndcapPInsert_pedMeanADC,
          .pedSigmaADC     = HcalEndcapPInsert_pedSigmaADC,
          .resolutionTDC   = HcalEndcapPInsert_resolutionTDC,
          .thresholdFactor = 0.,
          .thresholdValue  = 41.0, // 0.25 MeV --> 0.25 / 200 * 32768 = 41

          .sampFrac   = "1.0",
          .readout    = "HcalEndcapPInsertHits",
          .layerField = "layer",
      },
      app // TODO: Remove me once fixed
      ));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterHitsMerger_factory>(
      "HcalEndcapPInsertMergedHits", {"HcalEndcapPInsertRecHits"}, {"HcalEndcapPInsertMergedHits"},
      {.readout = "HcalEndcapPInsertHits", .fieldTransformations = {"layer:1", "slice:0"}},
      app // TODO: Remove me once fixed
      ));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterTruthClustering_factory>(
      "HcalEndcapPInsertTruthProtoClusters",
      {"HcalEndcapPInsertMergedHits", "HcalEndcapPInsertRawHitLinks"},
      {"HcalEndcapPInsertTruthProtoClusters"},
      app // TODO: Remove me once fixed
      ));

  // Clustering for the new insert design
  if (insertUsesPhysicalTiles) {
    app->Add(new JOmniFactoryGeneratorT<CalorimeterIslandCluster_factory>(
        "HcalEndcapPInsertImagingProtoClusters", {"HcalEndcapPInsertRecHits"},
        {"HcalEndcapPInsertImagingProtoClusters"},
        {
            .sectorDist           = 10.0 * dd4hep::cm,
            .dimScaledLocalDistXY = {1.5, 1.5},
            .splitCluster         = false,
            .minClusterHitEdep    = 5.0 * dd4hep::keV,
            .minClusterCenterEdep = 3.0 * dd4hep::MeV,
        },
        app // TODO: Remove me once fixed
        ));
  } else {
    // Also preserve the previous scheme
    app->Add(new JOmniFactoryGeneratorT<HEXPLIT_factory>(
        "HcalEndcapPInsertSubcellHits", {"HcalEndcapPInsertRecHits"},
        {"HcalEndcapPInsertSubcellHits"},
        {
            .MIP          = 480. * dd4hep::keV,
            .Emin_in_MIPs = 0.5,
            .tmax         = 162 * dd4hep::ns, //150 ns + (z at front face)/(speed of light)
        },
        app // TODO: Remove me once fixed
        ));

    app->Add(new JOmniFactoryGeneratorT<ImagingTopoCluster_factory>(
        "HcalEndcapPInsertImagingProtoClusters", {"HcalEndcapPInsertSubcellHits"},
        {"HcalEndcapPInsertImagingProtoClusters"},
        {
            .neighbourLayersRange = 1,
            .sameLayerDistXY =
                {"0.5 * max(HcalEndcapPInsertCellSizeLGRight, HcalEndcapPInsertCellSizeLGLeft)",
                 "0.5 * max(HcalEndcapPInsertCellSizeLGRight, HcalEndcapPInsertCellSizeLGLeft) * "
                 "sin(pi / 3)"},
            .diffLayerDistXY =
                {"0.25 * max(HcalEndcapPInsertCellSizeLGRight, HcalEndcapPInsertCellSizeLGLeft)",
                 "0.25 * max(HcalEndcapPInsertCellSizeLGRight, HcalEndcapPInsertCellSizeLGLeft) * "
                 "sin(pi / 3)"},

            .sameLayerMode        = eicrecon::ImagingTopoClusterConfig::ELayerMode::xy,
            .sectorDist           = 10.0 * dd4hep::cm,
            .minClusterHitEdep    = 5.0 * dd4hep::keV,
            .minClusterCenterEdep = 3.0 * dd4hep::MeV,
            .minClusterEdep       = 11.0 * dd4hep::MeV,
            .minClusterNhits      = 100,
        },
        app // TODO: Remove me once fixed
        ));
  }

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
      "HcalEndcapPInsertTruthClustersWithoutShapes",
      {
          "HcalEndcapPInsertTruthProtoClusters", // edm4eic::ProtoClusterCollection
          "HcalEndcapPInsertRawHitLinks",        // edm4eic::MCRecoCalorimeterHitLink
          "HcalEndcapPInsertRawHitAssociations" // edm4eic::MCRecoCalorimeterHitAssociationCollection
      },
      {"HcalEndcapPInsertTruthClustersWithoutShapes",
       "HcalEndcapPInsertTruthClusterLinksWithoutShapes",
       "HcalEndcapPInsertTruthClusterAssociationsWithoutShapes"}, // edm4eic::MCRecoClusterParticleAssociation
      {.energyWeight = "log", .sampFrac = 0.0257, .logWeightBase = 3.6, .enableEtaBounds = true},
      app // TODO: Remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>(
      "HcalEndcapPInsertTruthClusters",
      {"HcalEndcapPInsertTruthClustersWithoutShapes",
       "HcalEndcapPInsertTruthClusterLinksWithoutShapes"},
      {"HcalEndcapPInsertTruthClusters", "HcalEndcapPInsertTruthClusterLinks",
       "HcalEndcapPInsertTruthClusterAssociations"},
      {.longitudinalShowerInfoAvailable = true, .energyWeight = "log", .logWeightBase = 3.6}, app));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
      "HcalEndcapPInsertClustersWithoutShapes",
      {
          "HcalEndcapPInsertImagingProtoClusters", // edm4eic::ProtoClusterCollection
          "HcalEndcapPInsertRawHitLinks",          // edm4eic::MCRecoCalorimeterHitLink
          "HcalEndcapPInsertRawHitAssociations" // edm4eic::MCRecoCalorimeterHitAssociationCollection
      },
      {"HcalEndcapPInsertClustersWithoutShapes", "HcalEndcapPInsertClusterLinksWithoutShapes",
       "HcalEndcapPInsertClusterAssociationsWithoutShapes"}, // edm4eic::MCRecoClusterParticleAssociation
      {
          .energyWeight    = "log",
          .sampFrac        = 0.0257,
          .logWeightBase   = 6.2,
          .enableEtaBounds = false,
      },
      app // TODO: Remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>(
      "HcalEndcapPInsertClusters",
      {"HcalEndcapPInsertClustersWithoutShapes", "HcalEndcapPInsertClusterLinksWithoutShapes"},
      {"HcalEndcapPInsertClusters", "HcalEndcapPInsertClusterLinks",
       "HcalEndcapPInsertClusterAssociations"},
      {.longitudinalShowerInfoAvailable = true,
       .energyWeight                    = "log",
       .sampFrac                        = 0.0257,
       .logWeightBase                   = 6.2},
      app));

  // Make sure digi and reco use the same value
  decltype(CalorimeterHitDigiConfig::capADC) LFHCAL_capADC               = 65536;
  decltype(CalorimeterHitDigiConfig::dyRangeADC) LFHCAL_dyRangeADC       = 1 * dd4hep::GeV;
  decltype(CalorimeterHitDigiConfig::pedMeanADC) LFHCAL_pedMeanADC       = 50;
  decltype(CalorimeterHitDigiConfig::pedSigmaADC) LFHCAL_pedSigmaADC     = 10;
  decltype(CalorimeterHitDigiConfig::resolutionTDC) LFHCAL_resolutionTDC = 10 * dd4hep::picosecond;

  // Convert the individual LFHCAL energy deposits into fired SiPM pixels.
  app->Add(new JOmniFactoryGeneratorT<EdepToSiPMConversion_factory>(
      "LFHCALSiPMHits", {"EventHeader", "LFHCALHits"}, {"LFHCALSiPMHits"},
      {
          .edep_to_npe               = FHCal_edep_to_npe,
          .num_effective_sipm_pixels = FHCal_num_effective_sipm_pixels,
      },
      app // TODO: Remove me once fixed
      ));
  // Generate an analog pulse from each LFHCAL SiPM response hit.
  app->Add(new JOmniFactoryGeneratorT<PulseGeneration_factory<edm4hep::SimCalorimeterHit>>(
      "LFHCALPulses", {"LFHCALSiPMHits"}, {"LFHCALPulses"},
      {
          .pulse_shape_function = LFHCAL_pulse_shape_function,
          .pulse_shape_params   = LFHCAL_pulse_shape_params,
          .ignore_thres         = LFHCAL_ignore_thres,
          .timestep             = LFHCAL_timestep,
          .min_sampling_time    = LFHCAL_min_sampling_time,
          .max_time_bins        = LFHCAL_max_time_bins,
      },
      app // TODO: Remove me once fixed
      ));
  // Combine nearby LFHCAL pulses across longitudinal slices in the same readout channel.
  app->Add(new JOmniFactoryGeneratorT<PulseCombiner_factory>(
      "LFHCALCombinedPulses", {"LFHCALPulses"}, {"LFHCALCombinedPulses"},
      {
          .minimum_separation = FHCal_minimum_separation,
          .readout            = "LFHCALHits",
          .combine_field      = "rlayerz",
      },
      app // TODO: Remove me once fixed
      ));
  // Add electronics noise to the combined LFHCAL pulses.
  app->Add(new JOmniFactoryGeneratorT<PulseNoise_factory>(
      "LFHCALCombinedPulsesWithNoise", {"EventHeader", "LFHCALCombinedPulses"},
      {"LFHCALCombinedPulsesWithNoise"},
      {
          .poles    = FHCal_poles,
          .variance = FHCal_variance,
          .alpha    = FHCal_alpha,
          .scale    = FHCal_scale,
          .pedestal = FHCal_pedestal,
      },
      app // TODO: Remove me once fixed
      ));
  // Digitize the noisy LFHCAL pulses with the CALOROC frontend model.
  app->Add(new JOmniFactoryGeneratorT<CALOROCDigitization_factory>(
      "LFHCALCALOROCHits", {"LFHCALCombinedPulsesWithNoise"}, {"LFHCALCALOROCHits"},
      {
          .adc_phase            = FHCal_adc_phase,
          .toa_thres            = FHCal_toa_thres,
          .tot_thres            = FHCal_tot_thres,
          .dyRangeSingleGainADC = FHCal_dyRangeSingleGainADC,
          .dyRangeHighGainADC   = FHCal_dyRangeHighGainADC,
          .dyRangeLowGainADC    = FHCal_dyRangeLowGainADC,
      },
      app // TODO: Remove me once fixed
      ));
  // Convert LFHCAL CALOROC output into raw hits for CalorimeterHitReco.
  app->Add(new JOmniFactoryGeneratorT<CALOROCToRawCalorimeterHit_factory>(
      "LFHCALRawHits", {"LFHCALCALOROCHits", "LFHCALCombinedPulsesWithNoise"},
      {"LFHCALRawHits", "LFHCALRawHitLinks", "LFHCALRawHitAssociations"},
      {
          .calorocType             = "1B",
          .calorocResponseToEnergy = FHCal_calorocResponseToEnergy,
          .calorocTOTToEnergy      = FHCal_calorocTOTToEnergy,
          .caloroc = {
              .adc_phase            = FHCal_adc_phase,
              .toa_thres            = FHCal_toa_thres,
              .tot_thres            = FHCal_tot_thres,
              .dyRangeSingleGainADC = FHCal_dyRangeSingleGainADC,
              .dyRangeHighGainADC   = FHCal_dyRangeHighGainADC,
              .dyRangeLowGainADC    = FHCal_dyRangeLowGainADC,
          },
          .capADC        = LFHCAL_capADC,
          .dyRangeADC    = LFHCAL_dyRangeADC,
          .pedMeanADC    = LFHCAL_pedMeanADC,
          .resolutionTDC = LFHCAL_resolutionTDC,
      },
      app // TODO: Remove me once fixed
      ));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterHitReco_factory>(
      "LFHCALRecHits", {"LFHCALRawHits"}, {"LFHCALRecHits"},
      {
          .capADC          = LFHCAL_capADC,
          .dyRangeADC      = LFHCAL_dyRangeADC,
          .pedMeanADC      = LFHCAL_pedMeanADC,
          .pedSigmaADC     = LFHCAL_pedSigmaADC,
          .resolutionTDC   = LFHCAL_resolutionTDC,
          .thresholdFactor = 0.0,
          .thresholdValue  = 20, // 0.3 MeV deposition --> adc = 50 + 0.3 / 1000 * 65536 == 70
          .sampFrac        = "(rlayerz == 0) ? 0.019 : 0.037", // 0.019 only in the 0-th tile
          .readout         = "LFHCALHits",
          .layerField      = "rlayerz",
      },
      app // TODO: Remove me once fixed
      ));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterTruthClustering_factory>(
      "LFHCALTruthProtoClusters", {"LFHCALRecHits", "LFHCALRawHitLinks"},
      {"LFHCALTruthProtoClusters"},
      app // TODO: Remove me once fixed
      ));

  // Magic constants:
  //  54 - number of modules in a row/column
  //  2  - number of towers in a module
  // sign for towerx and towery are *negative* to maintain linearity with global X and Y
  std::string cellIdx_1 = "(54*2-moduleIDx_1*2-towerx_1)";
  std::string cellIdx_2 = "(54*2-moduleIDx_2*2-towerx_2)";
  std::string cellIdy_1 = "(54*2-moduleIDy_1*2-towery_1)";
  std::string cellIdy_2 = "(54*2-moduleIDy_2*2-towery_2)";
  std::string cellIdz_1 = "rlayerz_1";
  std::string cellIdz_2 = "rlayerz_2";
  std::string deltaX    = Form("abs(%s-%s)", cellIdx_2.data(), cellIdx_1.data());
  std::string deltaY    = Form("abs(%s-%s)", cellIdy_2.data(), cellIdy_1.data());
  std::string deltaZ    = Form("abs(%s-%s)", cellIdz_2.data(), cellIdz_1.data());
  std::string neighbor  = Form("(%s+%s+%s==1)", deltaX.data(), deltaY.data(), deltaZ.data());
  std::string corner2D =
      Form("((%s==0&&%s==1&&%s==1)||(%s==1&&%s==0&&%s==1)||(%s==1&&%s==1&&%s==0))", deltaZ.data(),
           deltaX.data(), deltaY.data(), deltaZ.data(), deltaX.data(), deltaY.data(), deltaZ.data(),
           deltaX.data(), deltaY.data());

  app->Add(new JOmniFactoryGeneratorT<CalorimeterIslandCluster_factory>(
      "LFHCALIslandProtoClusters", {"LFHCALRecHits"}, {"LFHCALIslandProtoClusters"},
      {
          .adjacencyMatrix = Form("%s||%s", neighbor.data(), corner2D.data()),
          .peakNeighbourhoodMatrix{},
          .readout    = "LFHCALHits",
          .sectorDist = 0 * dd4hep::cm,
          .localDistXY{},
          .localDistXZ{},
          .localDistYZ{},
          .globalDistRPhi{},
          .globalDistEtaPhi{},
          .dimScaledLocalDistXY{},
          .splitCluster                  = false,
          .minClusterHitEdep             = 1 * dd4hep::MeV,
          .minClusterCenterEdep          = 100.0 * dd4hep::MeV,
          .transverseEnergyProfileMetric = "globalDistEtaPhi",
          .transverseEnergyProfileScale  = 1.,
          .transverseEnergyProfileScaleUnits{},
      },
      app // TODO: Remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
      "LFHCALTruthClustersWithoutShapes",
      {
          "LFHCALTruthProtoClusters", // edm4eic::ProtoClusterCollection
          "LFHCALRawHitLinks",        // edm4eic::MCRecoCalorimeterHitLink
          "LFHCALRawHitAssociations"  // edm4eic::MCRecoCalorimeterHitAssociationCollection
      },
      {"LFHCALTruthClustersWithoutShapes", "LFHCALTruthClusterLinksWithoutShapes",
       "LFHCALTruthClusterAssociationsWithoutShapes"}, // edm4eic::MCRecoClusterParticleAssociation
      {.energyWeight = "log", .sampFrac = 1.0, .logWeightBase = 4.5, .enableEtaBounds = false},
      app // TODO: Remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>(
      "LFHCALTruthClusters",
      {"LFHCALTruthClustersWithoutShapes", "LFHCALTruthClusterLinksWithoutShapes"},
      {"LFHCALTruthClusters", "LFHCALTruthClusterLinks", "LFHCALTruthClusterAssociations"},
      {.longitudinalShowerInfoAvailable = true, .energyWeight = "log", .logWeightBase = 4.5}, app));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
      "LFHCALClustersWithoutShapes",
      {
          "LFHCALIslandProtoClusters", // edm4eic::ProtoClusterCollection
          "LFHCALRawHitLinks",         // edm4eic::MCRecoCalorimeterHitLink
          "LFHCALRawHitAssociations"   // edm4eic::MCRecoCalorimeterHitAssociationCollection
      },
      {"LFHCALClustersWithoutShapes", "LFHCALClusterLinksWithoutShapes",
       "LFHCALClusterAssociationsWithoutShapes"}, // edm4eic::MCRecoClusterParticleAssociation
      {
          .energyWeight    = "log",
          .sampFrac        = 1.0,
          .logWeightBase   = 4.5,
          .enableEtaBounds = false,
      },
      app // TODO: Remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>(
      "LFHCALClusters", {"LFHCALClustersWithoutShapes", "LFHCALClusterLinksWithoutShapes"},
      {"LFHCALClusters", "LFHCALClusterLinks", "LFHCALClusterAssociations"},
      {.longitudinalShowerInfoAvailable = true, .energyWeight = "log", .logWeightBase = 4.5}, app));

  app->Add(new JOmniFactoryGeneratorT<TrackClusterMergeSplitter_factory>(
      "LFHCALSplitMergeProtoClusters",
      {"LFHCALTrackClusterMatches", "LFHCALClusters", "CalorimeterTrackProjections"},
      {"LFHCALSplitMergeProtoClusters", "LFHCALTrackSplitMergeProtoClusterLinks"},
      {.minSigCut                    = -2.0,
       .avgEP                        = 0.50,
       .sigEP                        = 0.25,
       .drAdd                        = 0.30,
       .surfaceToUse                 = 1,
       .transverseEnergyProfileScale = 1.0},
      app // TODO: remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
      "LFHCALSplitMergeClustersWithoutShapes",
      {
          "LFHCALSplitMergeProtoClusters", // edm4eic::ProtoClusterCollection
          "LFHCALRawHitLinks",             // edm4eic::MCRecoCalorimeterHitLink
          "LFHCALRawHitAssociations"       // edm4hep::MCRecoCalorimeterHitAssociationCollection
      },
      {"LFHCALSplitMergeClustersWithoutShapes", "LFHCALSplitMergeClusterLinksWithoutShapes",
       "LFHCALSplitMergeClusterAssociationsWithoutShapes"}, // edm4eic::MCRecoClusterParticleAssociation
      {
          .energyWeight    = "log",
          .sampFrac        = 1.0,
          .logWeightBase   = 4.5,
          .enableEtaBounds = false,
      },
      app // TODO: Remove me once fixed
      ));

  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>(
      "LFHCALSplitMergeClusters",
      {"LFHCALSplitMergeClustersWithoutShapes", "LFHCALSplitMergeClusterLinksWithoutShapes"},
      {"LFHCALSplitMergeClusters", "LFHCALSplitMergeClusterLinks",
       "LFHCALSplitMergeClusterAssociations"},
      {.longitudinalShowerInfoAvailable = true}, app));
}
}
