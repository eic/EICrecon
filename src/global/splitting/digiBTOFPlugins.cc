// Copyright 2024, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.
// kuma edit

#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplicationFwd.h>
#include <TMath.h>
#include <edm4eic/unit_system.h>
#include <memory>

#include "algorithms/interfaces/WithPodConfig.h"
#include "JANA/Components/JOmniFactoryGeneratorT.h"
#include "factories/digi/EICROCDigitization_factory.h"
#include "factories/digi/PulseCombiner_factory.h"
#include "factories/digi/SiliconChargeSharing_factory.h"
#include "factories/digi/SiliconPulseDiscretization_factory.h"
#include "factories/digi/SiliconPulseGeneration_factory.h"
#include "factories/digi/SiliconTrackerDigi_factory.h"
#include "factories/tracking/TrackerHitReconstruction_factory.h"

// extern "C" {
void InitPlugin_digiBTOF(JApplication* app) {

  InitJANAPlugin(app);

  // == s == Register all digitization factories  =================================================

  using namespace eicrecon;

  // // Digitization
  app->Add(new JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>(
      JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>::TypedWiring{
          .tag          = "TOFBarrelRawHits_TK",
          .level        = JEventLevel::Timeslice,
          .input_names  = {"EventHeader", "TOFBarrelHits"},
          .output_names = {"TOFBarrelRawHits_TK", "TOFBarrelRawHitAssociations_TK"},
          .configs      = {
                   .threshold      = 6.0 * dd4hep::keV,
                   .timeResolution = 0.025, // [ns]
          }}));

  // Convert raw digitized Hits_TK into Hits_TK with geometry info (ready for tracking)
  app->Add(new JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>(
      {.tag          = "TOFBarrelRecHits_TK",
       .level        = JEventLevel::Timeslice,
       .input_names  = {"TOFBarrelRawHits_TK"},
       .output_names = {"TOFBarrelRecHits_TK"},
       .configs      = {
                .timeResolution = 10,
       }}));

  app->Add(new JOmniFactoryGeneratorT<SiliconChargeSharing_factory>(
      {.tag          = "TOFBarrelSharedHits_TK",
       .level        = JEventLevel::Timeslice,
       .input_names  = {"TOFBarrelHits"},
       .output_names = {"TOFBarrelSharedHits_TK"},
       .configs      = {
                .sigma_sharingx = 0.1 * dd4hep::cm,
                .sigma_sharingy = 0.5 * dd4hep::cm,
                .min_edep       = 0.0 * edm4eic::unit::GeV,
                .readout        = "TOFBarrelHits_TK",
       }}));

  // // calculation of the extreme values for Landau distribution can be found on lin 514-520 of
  // // https://root.cern.ch/root/html524/src/TMath.cxx.html#fsokrB Landau reaches minimum for mpv =
  // // 0 and sigma = 1 at x = -0.22278
  const double x_when_landau_min = -0.22278;
  const double landau_min        = TMath::Landau(x_when_landau_min, 0, 1, true);
  const double sigma_analog      = 0.293951 * edm4eic::unit::ns;
  const double Vm                = 1e-4 * dd4hep::GeV;
  const double adc_range         = 256;
  // gain is set such that pulse reaches a height of adc_range when EDep = Vm
  // gain is negative as LGAD voltage is always negative
  const double gain = -adc_range / Vm / landau_min;
  const int offset  = 3;
  app->Add(new JOmniFactoryGeneratorT<SiliconPulseGeneration_factory>(
      JOmniFactoryGeneratorT<SiliconPulseGeneration_factory>::TypedWiring{
          .tag          = "LGADPulseGeneration_TK",
          .level        = JEventLevel::Timeslice,
          .input_names  = {"TOFBarrelSharedHits_TK"},
          .output_names = {"TOFBarrelSmoothPulses_TK"},
          .configs      = {
                   .pulse_shape_function = "LandauPulse",
                   .pulse_shape_params   = {gain, sigma_analog, offset * sigma_analog},
                   .ignore_thres         = 0.05 * adc_range,
                   .timestep             = 0.01 * edm4eic::unit::ns,
          }}));

  app->Add(new JOmniFactoryGeneratorT<PulseCombiner_factory>(
      JOmniFactoryGeneratorT<PulseCombiner_factory>::TypedWiring{
          .tag          = "TOFBarrelPulseCombiner_TK",
          .level        = JEventLevel::Timeslice,
          .input_names  = {"TOFBarrelSmoothPulses_TK"},
          .output_names = {"TOFBarrelCombinedPulses_TK"},
          .configs      = {
                   .minimum_separation = 25 * edm4eic::unit::ns,
          }}));

  double risetime = 0.45 * edm4eic::unit::ns;
  app->Add(new JOmniFactoryGeneratorT<SiliconPulseDiscretization_factory>(
      JOmniFactoryGeneratorT<SiliconPulseDiscretization_factory>::TypedWiring{
          .tag          = "SiliconPulseDiscretization_TK",
          .level        = JEventLevel::Timeslice,
          .input_names  = {"TOFBarrelCombinedPulses_TK"},
          .output_names = {"TOFBarrelPulses_TK"},
          .configs =
              {
                  .EICROC_period = 25 * edm4eic::unit::ns,
                  .local_period  = 25 * edm4eic::unit::ns / 1024,
                  .global_offset = -offset * sigma_analog + risetime,
              },
      }));

  app->Add(new JOmniFactoryGeneratorT<EICROCDigitization_factory>(
      JOmniFactoryGeneratorT<EICROCDigitization_factory>::TypedWiring{
          .tag          = "EICROCDigitization_TK",
          .level        = JEventLevel::Timeslice,
          .input_names  = {"TOFBarrelPulses_TK"},
          .output_names = {"TOFBarrelADCTDC_TK"},
      }));
  // == e == Register all digitization factories  =================================================
}
// } // "C"
