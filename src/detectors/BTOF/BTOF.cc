// Copyright 2022, Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, Dmitry Kalinkin

#include <DD4hep/Detector.h>
#include <edm4eic/unit_system.h>
#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplication.h>
#include <edm4eic/unit_system.h>
#include <algorithm>
#include <gsl/pointers>
#include <memory>
#include <stdexcept>
#include <TMath.h>

#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/pid_lut/PIDLookupConfig.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"
#include "factories/digi/EICROCDigitization_factory.h"
#include "factories/digi/LGADChargeSharing_factory.h"
#include "factories/digi/SiliconPulseDiscretization_factory.h"
#include "factories/digi/SiliconPulseGeneration_factory.h"
#include "factories/digi/SiliconTrackerDigi_factory.h"
#include "factories/tracking/TrackerHitReconstruction_factory.h"
#include "factories/digi/PulseCombiner_factory.h"
#include "global/pid_lut/PIDLookup_factory.h"
#include "services/geometry/dd4hep/DD4hep_service.h"

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

  app->Add(new JOmniFactoryGeneratorT<LGADChargeSharing_factory>(
      "LGADChargeSharing", {"TOFBarrelHits"}, {"TOFBarrelSharedHits"},
      {.sigma_sharingx        = 0.1 * dd4hep::cm,
       .sigma_sharingy        = 0.5 * dd4hep::cm,
       .readout               = "TOFBarrelHits",
       .same_sensor_condition = "sensor_1 == sensor_2",
       .neighbor_fields       = {"x", "y"}},
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

  int BarrelTOF_ID = 0;
  try {
    auto detector = app->GetService<DD4hep_service>()->detector();
    BarrelTOF_ID  = detector->constant<int>("BarrelTOF_ID");
  } catch (const std::runtime_error&) {
    // Nothing
  }
  PIDLookupConfig pid_cfg{
      .filename       = "calibrations/tof.lut",
      .system         = BarrelTOF_ID,
      .pdg_values     = {11, 211, 321, 2212},
      .charge_values  = {1},
      .momentum_edges = {0.0, 0.3, 0.6, 0.9, 1.2, 1.5, 1.8, 2.1, 2.4, 2.7, 3.0,
                         3.3, 3.6, 3.9, 4.2, 4.5, 4.8, 5.1, 5.4, 5.7, 6.0},
      .polar_edges    = {2.50, 10.95, 19.40, 27.85, 36.30, 44.75, 53.20, 61.65, 70.10, 78.55, 87.00,
                         95.45, 103.90, 112.35, 120.80, 129.25, 137.70, 146.15, 154.60},
      .azimuthal_binning           = {0., 360., 360.}, // lower, upper, step
      .momentum_cut_max            = 2.5*edm4eic::unit::GeV,
      .momentum_bin_centers_in_lut = true,
      .polar_bin_centers_in_lut    = true,
  };

  app->Add(new JOmniFactoryGeneratorT<PIDLookup_factory>(
      "CombinedTOFTruthSeededLUTPID",
      {
          "ReconstructedTruthSeededChargedWithPFRICHPIDParticles",
          "ReconstructedTruthSeededChargedWithPFRICHPIDParticleAssociations",
      },
      {
          "ReconstructedTruthSeededChargedWithPFRICHTOFPIDParticles",
          "ReconstructedTruthSeededChargedWithPFRICHTOFPIDParticleAssociations",
          "CombinedTOFTruthSeededParticleIDs",
      },
      pid_cfg, app));

  app->Add(new JOmniFactoryGeneratorT<PIDLookup_factory>(
      "CombinedTOFLUTPID",
      {
          "ReconstructedChargedWithPFRICHPIDParticles",
          "ReconstructedChargedWithPFRICHPIDParticleAssociations",
      },
      {
          "ReconstructedChargedWithPFRICHTOFPIDParticles",
          "ReconstructedChargedWithPFRICHTOFPIDParticleAssociations",
          "CombinedTOFParticleIDs",
      },
      pid_cfg, app));
}
} // extern "C"
