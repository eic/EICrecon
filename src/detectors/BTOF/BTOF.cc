// Copyright 2022, Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, Dmitry Kalinkin

#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplicationFwd.h>
#include <TMath.h>
#include <edm4eic/unit_system.h>
#include <memory>

#include "algorithms/interfaces/WithPodConfig.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/digi/EICROCDigitization_factory.h"
#include "factories/digi/PulseCombiner_factory.h"
#include "factories/digi/SiliconChargeSharing_factory.h"
#include "factories/digi/SiliconPulseDiscretization_factory.h"
#include "factories/digi/SiliconPulseGeneration_factory.h"
#include "factories/digi/SiliconTrackerDigi_factory.h"
#include "factories/tracking/TrackerHitReconstruction_factory.h"

extern "C" {
void InitPlugin(JApplication* app) {
  InitJANAPlugin(app);

  using namespace eicrecon;

  // Digitization
  app->Add(new JOmniFactoryGeneratorT<SiliconTrackerDigi_factory>(
      "TOFBarrelRawHits", {"TOFBarrelHits"}, {"TOFBarrelRawHits", "TOFBarrelRawHitAssociations"},
      {
          .threshold      = 6.0 * dd4hep::keV,
          .timeResolution = 0.025, // [ns]
      },
      app));

  // Convert raw digitized hits into hits with geometry info (ready for tracking)
  app->Add(new JOmniFactoryGeneratorT<TrackerHitReconstruction_factory>(
      "TOFBarrelRecHits", {"TOFBarrelRawHits"}, // Input data collection tags
      {"TOFBarrelRecHits"},                     // Output data tag
      {
          .timeResolution = 10,
      },
      app)); // Hit reco default config for factories

  app->Add(new JOmniFactoryGeneratorT<SiliconChargeSharing_factory>(
      "TOFBarrelSharedHits", {"TOFBarrelHits"}, {"TOFBarrelSharedHits"},
      {
          .sigma_sharingx = 0.1 * dd4hep::cm,
          .sigma_sharingy = 0.5 * dd4hep::cm,
          .min_edep       = 0.0 * edm4eic::unit::GeV,
          .readout        = "TOFBarrelHits",
      },
      app));

  // calculation of the extreme values for Landau distribution can be found on lin 514-520 of
  // https://root.cern.ch/root/html524/src/TMath.cxx.html#fsokrB Landau reaches minimum for mpv =
  // 0 and sigma = 1 at x = -0.22278
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
      "LGADPulseGeneration", {"TOFBarrelSharedHits"}, {"TOFBarrelSmoothPulses"},
      {
          .pulse_shape_function = "LandauPulse",
          .pulse_shape_params   = {gain, sigma_analog, offset * sigma_analog},
          .ignore_thres         = 0.05 * adc_range,
          .timestep             = 0.01 * edm4eic::unit::ns,
      },
      app));

  app->Add(new JOmniFactoryGeneratorT<PulseCombiner_factory>(
      "TOFBarrelPulseCombiner", {"TOFBarrelSmoothPulses"}, {"TOFBarrelCombinedPulses"},
      {
          .minimum_separation = 25 * edm4eic::unit::ns,
      },
      app));

  double risetime = 0.45 * edm4eic::unit::ns;
  app->Add(new JOmniFactoryGeneratorT<SiliconPulseDiscretization_factory>(
      "SiliconPulseDiscretization", {"TOFBarrelCombinedPulses"}, {"TOFBarrelPulses"},
      {
          .EICROC_period = 25 * edm4eic::unit::ns,
          .local_period  = 25 * edm4eic::unit::ns / 1024,
          .global_offset = -offset * sigma_analog + risetime,
      },
      app));

  app->Add(new JOmniFactoryGeneratorT<EICROCDigitization_factory>(
      "EICROCDigitization", {"TOFBarrelPulses"}, {"TOFBarrelADCTDC"}, {}, app));
}
} // extern "C"
