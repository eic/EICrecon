// Copyright (C) 2022, 2023, Christopher Dilks, Luigi Dello Stritto
// Subject to the terms in the LICENSE file found in the top-level directory.

// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, Dmitry Kalinkin

#include <Evaluator/DD4hepUnits.h>
#include <JANA/JApplication.h>
#include <algorithm>
#include <utility>
#include <vector>

// algorithm configurations
#include "algorithms/digi/PhotoMultiplierHitDigiConfig.h"
#include "extensions/jana/JOmniFactoryGeneratorT.h"
// factories
#include "global/digi/PhotoMultiplierHitDigi_factory.h"
#include "global/pid_lut/PIDLookup_factory.h"

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
    for (auto qualifier : std::vector<std::string>({"", "Seeded"}))
    app->Add(new JOmniFactoryGeneratorT<PIDLookup_factory>(
          fmt::format("RICHEndcapN{}LUTPID", qualifier),
          {
          fmt::format("Reconstructed{}ChargedWithoutPIDParticles", qualifier),
          fmt::format("Reconstructed{}ChargedWithoutPIDParticleAssociations", qualifier),
          },
          {
          fmt::format("Reconstructed{}ChargedWithPFRICHPIDParticles", qualifier),
          fmt::format("Reconstructed{}ChargedWithPFRICHPIDParticleAssociations", qualifier),
          fmt::format("RICHEndcapN{}ParticleIDs", qualifier),
          },
          {
            .filename="calibrations/pfrich.lut",
            .system=BackwardRICH_ID,
            .pdg_values={11, 211, 321, 2212},
            .charge_values={1},
            .momentum_edges={0.08,  0.15,  0.25,  0.35,  0.45,  0.55,  0.65,  0.75,  0.85,  0.95,  1.50,  2.50,  3.50,  4.50,  5.50,  6.50,  7.50,  8.50,  9.50, 10.50, 11.50, 12.50, 13.50, 14.50},
            .polar_edges={2.703, 2.873, 3.020, 3.042, 3.081/* missing: , 3.097*/},
            .azimuthal_binning={0., 2 * M_PI, 2 * M_PI}, // lower, upper, step
            .polar_bin_centers_in_lut=true,
            .use_radians=true,
          },
          app
          ));
  }
}
