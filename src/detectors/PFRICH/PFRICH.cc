// Copyright (C) 2022, 2023, Christopher Dilks, Luigi Dello Stritto
// Subject to the terms in the LICENSE file found in the top-level directory.

// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, Dmitry Kalinkin

#include <DD4hep/Detector.h>
#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplication.h>
#include <math.h>
#include <algorithm>
#include <gsl/pointers>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

// algorithm configurations
#include "algorithms/digi/PhotoMultiplierHitDigiConfig.h"
#include "algorithms/pid_lut/PIDLookupConfig.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"
// factories
#include "global/digi/PhotoMultiplierHitDigi_factory.h"
#include "global/pid_lut/PIDLookup_factory.h"
#include "services/geometry/dd4hep/DD4hep_service.h"

extern "C" {
  void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);

    using namespace eicrecon;

    // configuration parameters ///////////////////////////////////////////////

    // digitization
    PhotoMultiplierHitDigiConfig digi_cfg;
    digi_cfg.seed            = 5; // FIXME: set to 0 for a 'unique' seed, but
                                  // that seems to delay the RNG from actually randomizing
    digi_cfg.hitTimeWindow   = 20.0; // [ns]
    digi_cfg.timeResolution  = 1/16.0; // [ns]
    digi_cfg.speMean         = 80.0;
    digi_cfg.speError        = 16.0;
    digi_cfg.pedMean         = 200.0;
    digi_cfg.pedError        = 3.0;
    digi_cfg.enablePixelGaps = true;
    digi_cfg.safetyFactor    = 0.7;
    digi_cfg.enableNoise     = false;
    digi_cfg.noiseRate       = 20000; // [Hz]
    digi_cfg.noiseTimeWindow = 20.0 * dd4hep::ns; // [ns]
    digi_cfg.quantumEfficiency.clear();
    digi_cfg.quantumEfficiency = { // wavelength units are [nm]
      {315,  0.00},
      {325,  0.04},
      {340,  0.10},
      {350,  0.20},
      {370,  0.30},
      {400,  0.35},
      {450,  0.40},
      {500,  0.38},
      {550,  0.35},
      {600,  0.27},
      {650,  0.20},
      {700,  0.15},
      {750,  0.12},
      {800,  0.08},
      {850,  0.06},
      {900,  0.04},
      {1000, 0.00}
    };

    // digitization
    app->Add(new JOmniFactoryGeneratorT<PhotoMultiplierHitDigi_factory>(
          "RICHEndcapNRawHits",
          {"RICHEndcapNHits"},
          {"RICHEndcapNRawHits", "RICHEndcapNRawHitsAssociations"},
          digi_cfg,
          app
          ));

    int BackwardRICH_ID = 0;
    try {
        auto detector = app->GetService<DD4hep_service>()->detector();
        BackwardRICH_ID = detector->constant<int>("BackwardRICH_ID");
    } catch(const std::runtime_error&) {
        // Nothing
    }
    PIDLookupConfig pid_cfg {
      .filename="calibrations/pfrich.lut",
      .system=BackwardRICH_ID,
      .pdg_values={11, 211, 321, 2212},
      .charge_values={1},
      .momentum_edges={0.4, 0.8, 1.2, 1.6, 2, 2.4, 2.8, 3.2, 3.6, 4, 4.4, 4.8, 5.2, 5.6, 6, 6.4, 6.8, 7.2, 7.6, 8, 8.4, 8.8, 9.2, 9.6, 10, 10.4, 10.8, 11.2, 11.6, 12, 12.4, 12.8, 13.2, 13.6, 14, 14.4, 14.8, 15.2},
      .polar_edges={2.65, 2.6725, 2.695, 2.7175, 2.74, 2.7625, 2.785, 2.8075, 2.83, 2.8525, 2.875, 2.8975, 2.92, 2.9425, 2.965, 2.9875, 3.01, 3.0325, 3.055, 3.0775},
      .azimuthal_binning={0., 2 * M_PI, 2 * M_PI / 120.}, // lower, upper, step
      .azimuthal_bin_centers_in_lut=true,
      .momentum_bin_centers_in_lut=true,
      .polar_bin_centers_in_lut=true,
      .use_radians=true,
    };

    app->Add(new JOmniFactoryGeneratorT<PIDLookup_factory>(
          "RICHEndcapNTruthSeededLUTPID",
          {
          "ReconstructedTruthSeededChargedWithoutPIDParticles",
          "ReconstructedTruthSeededChargedWithoutPIDParticleAssociations",
          },
          {
          "ReconstructedTruthSeededChargedWithPFRICHPIDParticles",
          "ReconstructedTruthSeededChargedWithPFRICHPIDParticleAssociations",
          "RICHEndcapNTruthSeededParticleIDs",
          },
          pid_cfg,
          app
          ));

    app->Add(new JOmniFactoryGeneratorT<PIDLookup_factory>(
          "RICHEndcapNLUTPID",
          {
          "ReconstructedChargedWithoutPIDParticles",
          "ReconstructedChargedWithoutPIDParticleAssociations",
          },
          {
          "ReconstructedChargedWithPFRICHPIDParticles",
          "ReconstructedChargedWithPFRICHPIDParticleAssociations",
          "RICHEndcapNParticleIDs",
          },
          pid_cfg,
          app
          ));
  }
}
