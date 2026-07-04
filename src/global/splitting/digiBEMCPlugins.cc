// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2025 Whitney Armstrong, Sylvester Joosten, Chao Peng, David Lawrence, Thomas Britton, Wouter Deconinck, Maria Zurek, Akshaya Vijay, Nathan Brei, Dmitry Kalinkin, Derek Anderson, Minho Kim

#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplication.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/Utils/JTypeInfo.h>
#include <edm4eic/EDM4eicVersion.h>
#include <edm4eic/unit_system.h>
#include <edm4hep/SimCalorimeterHit.h>
#include <cmath>
#include <map>
#include <memory>
#include <string>
#include <variant>
#include <vector>

#include "algorithms/calorimetry/CalorimeterHitDigiConfig.h"
#include "algorithms/calorimetry/ImagingTopoClusterConfig.h"
#include "algorithms/calorimetry/SimCalorimeterHitProcessorConfig.h"
#include "algorithms/digi/CALOROCDigitizationConfig.h"
#include "algorithms/digi/PulseCombinerConfig.h"
#include "algorithms/digi/PulseGenerationConfig.h"
#include "algorithms/digi/PulseNoiseConfig.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/calorimetry/CalorimeterClusterRecoCoG_factory.h"
#include "factories/calorimetry/CalorimeterClusterShape_factory.h"
#include "factories/calorimetry/CalorimeterHitDigi_factory.h"
#include "factories/calorimetry/CalorimeterHitReco_factory.h"
#include "factories/calorimetry/CalorimeterIslandCluster_factory.h"
#include "factories/calorimetry/EnergyPositionClusterMerger_factory.h"
#include "factories/calorimetry/ImagingClusterReco_factory.h"
#include "factories/calorimetry/ImagingTopoCluster_factory.h"
#include "factories/calorimetry/SimCalorimeterHitProcessor_factory.h"
#include "factories/calorimetry/TruthEnergyPositionClusterMerger_factory.h"
#include "factories/digi/PulseCombiner_factory.h"
#include "factories/digi/PulseGeneration_factory.h"
#include "factories/digi/PulseNoise_factory.h"

#if EDM4EIC_VERSION_MAJOR > 8 || (EDM4EIC_VERSION_MAJOR == 8 && EDM4EIC_VERSION_MINOR >= 7)
#include "factories/digi/CALOROCDigitization_factory.h"
#endif

// extern "C" {
void InitPlugin_digiBEMC(JApplication* app) {

  using namespace eicrecon;

  InitJANAPlugin(app);

  // Make sure left and right use the same value
  decltype(SimCalorimeterHitProcessorConfig::attenuationParameters) EcalBarrelScFi_attPars = {
      0.416212, 747.39875 * edm4eic::unit::mm, 7521.88383 * edm4eic::unit::mm};
  decltype(SimCalorimeterHitProcessorConfig::hitMergeFields) EcalBarrelScFi_hitMergeFields = {
      "fiber", "z"};
  decltype(SimCalorimeterHitProcessorConfig::contributionMergeFields)
      EcalBarrelScFi_contributionMergeFields = {"fiber"};
  decltype(SimCalorimeterHitProcessorConfig::inversePropagationSpeed)
      EcalBarrelScFi_inversePropagationSpeed = {(1. / 160) * edm4eic::unit::ns / edm4eic::unit::mm};
  decltype(SimCalorimeterHitProcessorConfig::fixedTimeDelay) EcalBarrelScFi_fixedTimeDelay = {
      2 * edm4eic::unit::ns};
  decltype(SimCalorimeterHitProcessorConfig::timeWindow) EcalBarrelScFi_timeWindow = {
      100 * edm4eic::unit::ns};

  decltype(PulseGenerationConfig::pulse_shape_function) EcalBarrelScFi_pulse_shape_function = {
      "LandauPulse"};
  decltype(PulseGenerationConfig::pulse_shape_params) EcalBarrelScFi_pulse_shape_params = {
      5.0, 10 * edm4eic::unit::ns};
  decltype(PulseGenerationConfig::ignore_thres) EcalBarrelScFi_ignore_thres = {5.0e-5};
  decltype(PulseGenerationConfig::timestep) EcalBarrelScFi_timestep = {0.5 * edm4eic::unit::ns};

  decltype(PulseCombinerConfig::combine_field) EcalBarrelScFi_combine_field           = {"grid"};
  decltype(PulseCombinerConfig::minimum_separation) EcalBarrelScFi_minimum_separation = {
      100 * edm4eic::unit::ns};
  decltype(PulseNoiseConfig::poles) EcalBarrelScFi_poles                  = {2};
  decltype(PulseNoiseConfig::variance) EcalBarrelScFi_variance            = {0.5};
  decltype(PulseNoiseConfig::alpha) EcalBarrelScFi_alpha                  = {0};
  decltype(PulseNoiseConfig::scale) EcalBarrelScFi_scale                  = {5.4e-5};
  decltype(PulseNoiseConfig::pedestal) EcalBarrelScFi_pedestal            = {1.6e-4};
  decltype(CALOROCDigitizationConfig::adc_phase) EcalBarrelScFi_adc_phase = {10 *
                                                                             edm4eic::unit::ns};
  decltype(CALOROCDigitizationConfig::toa_thres) EcalBarrelScFi_toa_thres = {4.0e-4};
  decltype(CALOROCDigitizationConfig::tot_thres) EcalBarrelScFi_tot_thres = {8.0e-4};
  decltype(CALOROCDigitizationConfig::dyRangeSingleGainADC) EcalBarrelScFi_dyRangeSingleGainADC = {
      1.0e-3};
  decltype(CALOROCDigitizationConfig::dyRangeHighGainADC) EcalBarrelScFi_dyRangeHighGainADC = {
      1.0e-3};
  decltype(CALOROCDigitizationConfig::dyRangeLowGainADC) EcalBarrelScFi_dyRangeLowGainADC = {
      1.5e-2};

  // Make sure digi and reco use the same value
  decltype(CalorimeterHitDigiConfig::capADC) EcalBarrelScFi_capADC = 16384; //16384,  14bit ADC
  decltype(CalorimeterHitDigiConfig::dyRangeADC) EcalBarrelScFi_dyRangeADC   = 1500 * dd4hep::MeV;
  decltype(CalorimeterHitDigiConfig::pedMeanADC) EcalBarrelScFi_pedMeanADC   = 100;
  decltype(CalorimeterHitDigiConfig::pedSigmaADC) EcalBarrelScFi_pedSigmaADC = 1;
  decltype(CalorimeterHitDigiConfig::resolutionTDC) EcalBarrelScFi_resolutionTDC =
      10 * dd4hep::picosecond;
  app->Add(new JOmniFactoryGeneratorT<SimCalorimeterHitProcessor_factory>(
      JOmniFactoryGeneratorT<SimCalorimeterHitProcessor_factory>::TypedWiring{
          .m_tag                 = "EcalBarrelScFiPAttenuatedHits_TK",
          .m_default_input_tags  = {"EcalBarrelScFiHits"},
          .m_default_output_tags = {"EcalBarrelScFiPAttenuatedHits_TK",
                                    "EcalBarrelScFiPAttenuatedHitContributions_TK"},
          .m_default_cfg =
              {
                  .attenuationParameters            = EcalBarrelScFi_attPars,
                  .readout                          = "EcalBarrelScFiHits",
                  .attenuationReferencePositionName = "EcalBarrel_LightGuide_PositivePosZ",
                  .hitMergeFields                   = EcalBarrelScFi_hitMergeFields,
                  .contributionMergeFields          = EcalBarrelScFi_contributionMergeFields,
                  .inversePropagationSpeed          = EcalBarrelScFi_inversePropagationSpeed,
                  .fixedTimeDelay                   = EcalBarrelScFi_fixedTimeDelay,
                  .timeWindow                       = EcalBarrelScFi_timeWindow,
              },
          .level = JEventLevel::Timeslice},
      app));
  app->Add(new JOmniFactoryGeneratorT<SimCalorimeterHitProcessor_factory>(
      JOmniFactoryGeneratorT<SimCalorimeterHitProcessor_factory>::TypedWiring{
          .m_tag                 = "EcalBarrelScFiNAttenuatedHits_TK",
          .m_default_input_tags  = {"EcalBarrelScFiHits"},
          .m_default_output_tags = {"EcalBarrelScFiNAttenuatedHits_TK",
                                    "EcalBarrelScFiNAttenuatedHitContributions_TK"},
          .m_default_cfg =
              {
                  .attenuationParameters            = EcalBarrelScFi_attPars,
                  .readout                          = "EcalBarrelScFiHits",
                  .attenuationReferencePositionName = "EcalBarrel_LightGuide_NegativePosZ",
                  .hitMergeFields                   = EcalBarrelScFi_hitMergeFields,
                  .contributionMergeFields          = EcalBarrelScFi_contributionMergeFields,
                  .inversePropagationSpeed          = EcalBarrelScFi_inversePropagationSpeed,
                  .fixedTimeDelay                   = EcalBarrelScFi_fixedTimeDelay,
                  .timeWindow                       = EcalBarrelScFi_timeWindow,
              },
          .level = JEventLevel::Timeslice},
      app));
  app->Add(new JOmniFactoryGeneratorT<PulseGeneration_factory<edm4hep::SimCalorimeterHit>>(
      JOmniFactoryGeneratorT<PulseGeneration_factory<edm4hep::SimCalorimeterHit>>::TypedWiring{
          .m_tag                 = "EcalBarrelScFiPPulses_TK",
          .m_default_input_tags  = {"EcalBarrelScFiPAttenuatedHits_TK"},
          .m_default_output_tags = {"EcalBarrelScFiPPulses_TK"},
          .m_default_cfg =
              {
                  .pulse_shape_function = EcalBarrelScFi_pulse_shape_function,
                  .pulse_shape_params   = EcalBarrelScFi_pulse_shape_params,
                  .ignore_thres         = EcalBarrelScFi_ignore_thres,
                  .timestep             = EcalBarrelScFi_timestep,
              },
          .level = JEventLevel::Timeslice},
      app));
  app->Add(new JOmniFactoryGeneratorT<PulseGeneration_factory<edm4hep::SimCalorimeterHit>>(
      JOmniFactoryGeneratorT<PulseGeneration_factory<edm4hep::SimCalorimeterHit>>::TypedWiring{
          .m_tag                 = "EcalBarrelScFiNPulses_TK",
          .m_default_input_tags  = {"EcalBarrelScFiNAttenuatedHits_TK"},
          .m_default_output_tags = {"EcalBarrelScFiNPulses_TK"},
          .m_default_cfg =
              {
                  .pulse_shape_function = EcalBarrelScFi_pulse_shape_function,
                  .pulse_shape_params   = EcalBarrelScFi_pulse_shape_params,
                  .ignore_thres         = EcalBarrelScFi_ignore_thres,
                  .timestep             = EcalBarrelScFi_timestep,
              },
          .level = JEventLevel::Timeslice},
      app));
  app->Add(new JOmniFactoryGeneratorT<PulseCombiner_factory>(
      JOmniFactoryGeneratorT<PulseCombiner_factory>::TypedWiring{
          .m_tag                 = "EcalBarrelScFiPCombinedPulses_TK",
          .m_default_input_tags  = {"EcalBarrelScFiPPulses_TK"},
          .m_default_output_tags = {"EcalBarrelScFiPCombinedPulses_TK"},
          .m_default_cfg =
              {
                  .minimum_separation = EcalBarrelScFi_minimum_separation,
                  .readout            = "EcalBarrelScFiHits",
                  .combine_field      = EcalBarrelScFi_combine_field,
              },
          .level = JEventLevel::Timeslice},
      app));
  app->Add(new JOmniFactoryGeneratorT<PulseCombiner_factory>(
      JOmniFactoryGeneratorT<PulseCombiner_factory>::TypedWiring{
          .m_tag                 = "EcalBarrelScFiNCombinedPulses_TK",
          .m_default_input_tags  = {"EcalBarrelScFiNPulses_TK"},
          .m_default_output_tags = {"EcalBarrelScFiNCombinedPulses_TK"},
          .m_default_cfg =
              {
                  .minimum_separation = EcalBarrelScFi_minimum_separation,
                  .readout            = "EcalBarrelScFiHits",
                  .combine_field      = EcalBarrelScFi_combine_field,
              },
          .level = JEventLevel::Timeslice},
      app));
  app->Add(new JOmniFactoryGeneratorT<PulseNoise_factory>(
      JOmniFactoryGeneratorT<PulseNoise_factory>::TypedWiring{
          .m_tag                 = "EcalBarrelScFiPCombinedPulsesWithNoise_TK",
          .m_default_input_tags  = {"EventHeader", "EcalBarrelScFiPCombinedPulses_TK"},
          .m_default_output_tags = {"EcalBarrelScFiPCombinedPulsesWithNoise_TK"},
          .m_default_cfg =
              {
                  .poles    = EcalBarrelScFi_poles,
                  .variance = EcalBarrelScFi_variance,
                  .alpha    = EcalBarrelScFi_alpha,
                  .scale    = EcalBarrelScFi_scale,
                  .pedestal = EcalBarrelScFi_pedestal,
              },
          .level = JEventLevel::Timeslice},
      app));
  app->Add(new JOmniFactoryGeneratorT<PulseNoise_factory>(
      JOmniFactoryGeneratorT<PulseNoise_factory>::TypedWiring{
          .m_tag                 = "EcalBarrelScFiNCombinedPulsesWithNoise_TK",
          .m_default_input_tags  = {"EventHeader", "EcalBarrelScFiNCombinedPulses_TK"},
          .m_default_output_tags = {"EcalBarrelScFiNCombinedPulsesWithNoise_TK"},
          .m_default_cfg =
              {
                  .poles    = EcalBarrelScFi_poles,
                  .variance = EcalBarrelScFi_variance,
                  .alpha    = EcalBarrelScFi_alpha,
                  .scale    = EcalBarrelScFi_scale,
                  .pedestal = EcalBarrelScFi_pedestal,
              },
          .level = JEventLevel::Timeslice},
      app));
#if EDM4EIC_VERSION_MAJOR > 8 || (EDM4EIC_VERSION_MAJOR == 8 && EDM4EIC_VERSION_MINOR >= 7)
  app->Add(new JOmniFactoryGeneratorT<CALOROCDigitization_factory>(
      JOmniFactoryGeneratorT<CALOROCDigitization_factory>::TypedWiring{
          .m_tag                 = "EcalBarrelScFiPCALOROCHits_TK",
          .m_default_input_tags  = {"EcalBarrelScFiPCombinedPulsesWithNoise_TK"},
          .m_default_output_tags = {"EcalBarrelScFiPCALOROCHits_TK"},
          .m_default_cfg =
              {
                  .adc_phase            = EcalBarrelScFi_adc_phase,
                  .toa_thres            = EcalBarrelScFi_toa_thres,
                  .tot_thres            = EcalBarrelScFi_tot_thres,
                  .dyRangeSingleGainADC = EcalBarrelScFi_dyRangeSingleGainADC,
                  .dyRangeHighGainADC   = EcalBarrelScFi_dyRangeHighGainADC,
                  .dyRangeLowGainADC    = EcalBarrelScFi_dyRangeLowGainADC,
              },
          .level = JEventLevel::Timeslice},
      app));
  app->Add(new JOmniFactoryGeneratorT<CALOROCDigitization_factory>(
      JOmniFactoryGeneratorT<CALOROCDigitization_factory>::TypedWiring{
          .m_tag                 = "EcalBarrelScFiNCALOROCHits_TK",
          .m_default_input_tags  = {"EcalBarrelScFiNCombinedPulsesWithNoise_TK"},
          .m_default_output_tags = {"EcalBarrelScFiNCALOROCHits_TK"},
          .m_default_cfg =
              {
                  .adc_phase            = EcalBarrelScFi_adc_phase,
                  .toa_thres            = EcalBarrelScFi_toa_thres,
                  .tot_thres            = EcalBarrelScFi_tot_thres,
                  .dyRangeSingleGainADC = EcalBarrelScFi_dyRangeSingleGainADC,
                  .dyRangeHighGainADC   = EcalBarrelScFi_dyRangeHighGainADC,
                  .dyRangeLowGainADC    = EcalBarrelScFi_dyRangeLowGainADC,
              },
          .level = JEventLevel::Timeslice},
      app));
#endif
  app->Add(new JOmniFactoryGeneratorT<CalorimeterHitDigi_factory>(
      JOmniFactoryGeneratorT<CalorimeterHitDigi_factory>::TypedWiring{
          .m_tag                 = "EcalBarrelScFiRawHits_TK",
          .m_default_input_tags  = {"EventHeader", "EcalBarrelScFiHits"},
          .m_default_output_tags = {"EcalBarrelScFiRawHits_TK",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                                    "EcalBarrelScFiRawHitLinks_TK",
#endif
                                    "EcalBarrelScFiRawHitAssociations_TK"},
          .m_default_cfg =
              {
                  .eRes          = {0.0 * sqrt(dd4hep::GeV), 0.0, 0.0 * dd4hep::GeV},
                  .tRes          = 0.0 * dd4hep::ns,
                  .threshold     = 0.0 * dd4hep::keV, // threshold is set in ADC in reco
                  .capADC        = EcalBarrelScFi_capADC,
                  .dyRangeADC    = EcalBarrelScFi_dyRangeADC,
                  .pedMeanADC    = EcalBarrelScFi_pedMeanADC,
                  .pedSigmaADC   = EcalBarrelScFi_pedSigmaADC,
                  .resolutionTDC = EcalBarrelScFi_resolutionTDC,
                  .corrMeanScale = "1.0",
                  .readout       = "EcalBarrelScFiHits",
                  .fields        = {"fiber", "z"},
              },
          .level = JEventLevel::Timeslice},
      app));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterHitReco_factory>(
      JOmniFactoryGeneratorT<CalorimeterHitReco_factory>::TypedWiring{
          .m_tag                 = "EcalBarrelScFiRecHits_TK",
          .m_default_input_tags  = {"EcalBarrelScFiRawHits_TK"},
          .m_default_output_tags = {"EcalBarrelScFiRecHits_TK"},
          .m_default_cfg =
              {
                  .capADC      = EcalBarrelScFi_capADC,
                  .dyRangeADC  = EcalBarrelScFi_dyRangeADC,
                  .pedMeanADC  = EcalBarrelScFi_pedMeanADC,
                  .pedSigmaADC = EcalBarrelScFi_pedSigmaADC, // not needed; use only thresholdValue
                  .resolutionTDC   = EcalBarrelScFi_resolutionTDC,
                  .thresholdFactor = 0.0, // use only thresholdValue
                  .thresholdValue =
                      5.0, // 16384 ADC counts/1500 MeV * 0.5 MeV (desired threshold) = 5.46
                  .sampFrac       = "0.09285755",
                  .readout        = "EcalBarrelScFiHits",
                  .layerField     = "layer",
                  .sectorField    = "sector",
                  .localDetFields = {"system", "sector"},
                  // here we want to use grid center position (XY) but keeps the z information from fiber-segment
                  // TODO: a more realistic way to get z is to reconstruct it from timing
                  .maskPos       = "xy",
                  .maskPosFields = {"fiber", "z"},
              },
          .level = JEventLevel::Timeslice},
      app));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterIslandCluster_factory>(
      JOmniFactoryGeneratorT<CalorimeterIslandCluster_factory>::TypedWiring{
          .m_tag                 = "EcalBarrelScFiProtoClusters_TK",
          .m_default_input_tags  = {"EcalBarrelScFiRecHits_TK"},
          .m_default_output_tags = {"EcalBarrelScFiProtoClusters_TK"},
          .m_default_cfg =
              {
                  .adjacencyMatrix{},
                  .peakNeighbourhoodMatrix{},
                  .readout{},
                  .sectorDist = 50. * dd4hep::mm,
                  .localDistXY{},
                  .localDistXZ = {80 * dd4hep::mm, 80 * dd4hep::mm},
                  .localDistYZ{},
                  .globalDistRPhi{},
                  .globalDistEtaPhi{},
                  .dimScaledLocalDistXY{},
                  .splitCluster         = false,
                  .minClusterHitEdep    = 5.0 * dd4hep::MeV,
                  .minClusterCenterEdep = 100.0 * dd4hep::MeV,
                  .transverseEnergyProfileMetric{},
                  .transverseEnergyProfileScale{},
                  .transverseEnergyProfileScaleUnits{},
              },
          .level = JEventLevel::Timeslice},
      app));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>(
      JOmniFactoryGeneratorT<CalorimeterClusterRecoCoG_factory>::TypedWiring{
          .m_tag = "EcalBarrelScFiClustersWithoutShapes_TK",
          .m_default_input_tags =
              {
                  "EcalBarrelScFiProtoClusters_TK", // edm4eic::ProtoClusterCollection
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                  "EcalBarrelScFiRawHitLinks_TK", // edm4eic::MCRecoCalorimeterHitLink
#endif
                  "EcalBarrelScFiRawHitAssociations_TK" // edm4eic::MCRecoCalorimeterHitAssociation
              },
          .m_default_output_tags = {"EcalBarrelScFiClustersWithoutShapes_TK",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                                    "EcalBarrelScFiClusterLinksWithoutShapes_TK",
#endif
                                    "EcalBarrelScFiClusterAssociationsWithoutShapes_TK"},
          .m_default_cfg =
              {
                  .energyWeight    = "log",
                  .sampFrac        = 1.0,
                  .logWeightBase   = 6.2,
                  .enableEtaBounds = false,
              },
          .level = JEventLevel::Timeslice},
      app));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>(
      JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>::TypedWiring{
          .m_tag                 = "EcalBarrelScFiClusters_TK",
          .m_default_input_tags  = {"EcalBarrelScFiClustersWithoutShapes_TK",
                                    "EcalBarrelScFiClusterAssociationsWithoutShapes_TK"},
          .m_default_output_tags = {"EcalBarrelScFiClusters_TK",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                                    "EcalBarrelScFiClusterLinks_TK",
#endif
                                    "EcalBarrelScFiClusterAssociations_TK"},
          .m_default_cfg = {.longitudinalShowerInfoAvailable = true,
                            .energyWeight                    = "log",
                            .logWeightBase                   = 6.2},
          .level         = JEventLevel::Timeslice},
      app));

  // Make sure digi and reco use the same value
  decltype(SimCalorimeterHitProcessorConfig::timeWindow) EcalBarrelImaging_timeWindow = {
      100 * edm4eic::unit::ns};

  decltype(CalorimeterHitDigiConfig::capADC) EcalBarrelImaging_capADC = 8192; //8192,  13bit ADC
  decltype(CalorimeterHitDigiConfig::dyRangeADC) EcalBarrelImaging_dyRangeADC = 3 * dd4hep::MeV;
  decltype(CalorimeterHitDigiConfig::pedMeanADC) EcalBarrelImaging_pedMeanADC =
      14; // Noise floor at 5 keV: 8192 / 3 * 0.005
  decltype(CalorimeterHitDigiConfig::pedSigmaADC) EcalBarrelImaging_pedSigmaADC =
      5; // Upper limit for sigma for AstroPix
  decltype(CalorimeterHitDigiConfig::resolutionTDC) EcalBarrelImaging_resolutionTDC =
      3.25 * dd4hep::nanosecond;
  app->Add(new JOmniFactoryGeneratorT<SimCalorimeterHitProcessor_factory>(
      JOmniFactoryGeneratorT<SimCalorimeterHitProcessor_factory>::TypedWiring{
          .m_tag                 = "EcalBarrelImagingProcessedHits_TK",
          .m_default_input_tags  = {"EcalBarrelImagingHits"},
          .m_default_output_tags = {"EcalBarrelImagingProcessedHits_TK",
                                    "EcalBarrelImagingProcessedHitContributions_TK"},
          .m_default_cfg =
              {
                  .readout    = "EcalBarrelImagingHits",
                  .timeWindow = EcalBarrelImaging_timeWindow,
              },
          .level = JEventLevel::Timeslice},
      app));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterHitDigi_factory>(
      JOmniFactoryGeneratorT<CalorimeterHitDigi_factory>::TypedWiring{
          .m_tag                 = "EcalBarrelImagingRawHits_TK",
          .m_default_input_tags  = {"EventHeader", "EcalBarrelImagingProcessedHits_TK"},
          .m_default_output_tags = {"EcalBarrelImagingRawHits_TK",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                                    "EcalBarrelImagingRawHitLinks_TK",
#endif
                                    "EcalBarrelImagingRawHitAssociations_TK"},
          .m_default_cfg =
              {
                  .eRes          = {0.0 * sqrt(dd4hep::GeV), 0.02, 0.0 * dd4hep::GeV},
                  .tRes          = 0.0 * dd4hep::ns,
                  .capADC        = EcalBarrelImaging_capADC,
                  .dyRangeADC    = EcalBarrelImaging_dyRangeADC,
                  .pedMeanADC    = EcalBarrelImaging_pedMeanADC,
                  .pedSigmaADC   = EcalBarrelImaging_pedSigmaADC,
                  .resolutionTDC = EcalBarrelImaging_resolutionTDC,
                  .corrMeanScale = "1.0",
                  .readout       = "EcalBarrelImagingHits",
              },
          .level = JEventLevel::Timeslice},
      app));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterHitReco_factory>(
      JOmniFactoryGeneratorT<CalorimeterHitReco_factory>::TypedWiring{
          .m_tag                 = "EcalBarrelImagingRecHits_TK",
          .m_default_input_tags  = {"EcalBarrelImagingRawHits_TK"},
          .m_default_output_tags = {"EcalBarrelImagingRecHits_TK"},
          .m_default_cfg =
              {
                  .capADC     = EcalBarrelImaging_capADC,
                  .dyRangeADC = EcalBarrelImaging_dyRangeADC,
                  .pedMeanADC = EcalBarrelImaging_pedMeanADC,
                  .pedSigmaADC =
                      EcalBarrelImaging_pedSigmaADC, // not needed; use only thresholdValue
                  .resolutionTDC   = EcalBarrelImaging_resolutionTDC,
                  .thresholdFactor = 0.0, // use only thresholdValue
                  .thresholdValue =
                      41, // 8192 ADC counts/3 MeV * 0.015 MeV (desired threshold) = 41
                  .sampFrac    = "0.00429453",
                  .readout     = "EcalBarrelImagingHits",
                  .layerField  = "layer",
                  .sectorField = "sector",
              },
          .level = JEventLevel::Timeslice},
      app));
  app->Add(new JOmniFactoryGeneratorT<ImagingTopoCluster_factory>(
      JOmniFactoryGeneratorT<ImagingTopoCluster_factory>::TypedWiring{
          .m_tag                 = "EcalBarrelImagingProtoClusters_TK",
          .m_default_input_tags  = {"EcalBarrelImagingRecHits_TK"},
          .m_default_output_tags = {"EcalBarrelImagingProtoClusters_TK"},
          .m_default_cfg =
              {
                  .neighbourLayersRange = 2, //  # id diff for adjacent layer
                  .sameLayerDistTZ      = {2.0 * dd4hep::mm, 2 * dd4hep::mm}, //  # same layer
                  .diffLayerDistEtaPhi  = {10 * dd4hep::mrad,
                                           10 * dd4hep::mrad}, //  # adjacent layer
                  .sameLayerMode        = eicrecon::ImagingTopoClusterConfig::ELayerMode::tz,
                  .diffLayerMode        = eicrecon::ImagingTopoClusterConfig::ELayerMode::etaphi,
                  .sectorDist           = 3.0 * dd4hep::cm,
                  .minClusterHitEdep    = 0,
                  .minClusterCenterEdep = 0,
                  .minClusterEdep       = 100 * dd4hep::MeV,
                  .minClusterNhits      = 10,
              },
          .level = JEventLevel::Timeslice},
      app));

  app->Add(new JOmniFactoryGeneratorT<ImagingClusterReco_factory>(
      JOmniFactoryGeneratorT<ImagingClusterReco_factory>::TypedWiring{
          .m_tag                = "EcalBarrelImagingClustersWithoutShapes_TK",
          .m_default_input_tags = {"EcalBarrelImagingProtoClusters_TK",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                                   "EcalBarrelImagingRawHitLinks_TK",
#endif
                                   "EcalBarrelImagingRawHitAssociations_TK"},
          .m_default_output_tags = {"EcalBarrelImagingClustersWithoutShapes_TK",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                                    "EcalBarrelImagingClusterLinksWithoutShapes_TK",
#endif
                                    "EcalBarrelImagingClusterAssociationsWithoutShapes_TK",
                                    "EcalBarrelImagingLayers_TK"},
          .m_default_cfg =
              {
                  .trackStopLayer = 6,
              },
          .level = JEventLevel::Timeslice},
      app));
  app->Add(new JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>(
      JOmniFactoryGeneratorT<CalorimeterClusterShape_factory>::TypedWiring{
          .m_tag                 = "EcalBarrelImagingClusters_TK",
          .m_default_input_tags  = {"EcalBarrelImagingClustersWithoutShapes_TK",
                                    "EcalBarrelImagingClusterAssociationsWithoutShapes_TK"},
          .m_default_output_tags = {"EcalBarrelImagingClusters_TK",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                                    "EcalBarrelImagingClusterLink_TK",
#endif
                                    "EcalBarrelImagingClusterAssociations_TK"},
          .m_default_cfg = {.longitudinalShowerInfoAvailable = false,
                            .energyWeight                    = "log",
                            .logWeightBase                   = 6.2},
          .level         = JEventLevel::Timeslice},
      app));
  app->Add(new JOmniFactoryGeneratorT<EnergyPositionClusterMerger_factory>(
      JOmniFactoryGeneratorT<EnergyPositionClusterMerger_factory>::TypedWiring{
          .m_tag                 = "EcalBarrelClusters_TK",
          .m_default_input_tags  = {"EcalBarrelScFiClusters_TK",
                                    "EcalBarrelScFiClusterAssociations_TK",
                                    "EcalBarrelImagingClusters_TK",
                                    "EcalBarrelImagingClusterAssociations_TK"},
          .m_default_output_tags = {"EcalBarrelClusters_TK",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                                    "EcalBarrelClusterLinks_TK",
#endif
                                    "EcalBarrelClusterAssociations_TK"},
          .m_default_cfg =
              {
                  .energyRelTolerance = 0.5,
                  .phiTolerance       = 0.1,
                  .etaTolerance       = 0.2,
              },
          .level = JEventLevel::Timeslice},
      app));
  app->Add(new JOmniFactoryGeneratorT<TruthEnergyPositionClusterMerger_factory>(
      JOmniFactoryGeneratorT<TruthEnergyPositionClusterMerger_factory>::TypedWiring{
          .m_tag                 = "EcalBarrelTruthClusters_TK",
          .m_default_input_tags  = {"MCParticles", "EcalBarrelScFiClusters_TK",
                                    "EcalBarrelScFiClusterAssociations_TK",
                                    "EcalBarrelImagingClusters_TK",
                                    "EcalBarrelImagingClusterAssociations_TK"},
          .m_default_output_tags = {"EcalBarrelTruthClusters_TK",
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
                                    "EcalBarrelTruthClusterLinks_TK",
#endif
                                    "EcalBarrelTruthClusterAssociations_TK"},
          .m_default_cfg = {},
          .level         = JEventLevel::Timeslice},
      app));
}
// }
