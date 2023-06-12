// Copyright (C) 2022, 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>
#include <JANA/JFactoryGenerator.h>
#include <extensions/jana/JChainFactoryGeneratorT.h>
#include <extensions/jana/JChainMultifactoryGeneratorT.h>

// factories
#include <global/digi/PhotoMultiplierHitDigi_factory.h>
#include <global/pid/RichTrack_factory.h>
#include <global/pid/MergeTrack_factory.h>

// algorithm configurations
#include <algorithms/digi/PhotoMultiplierHitDigiConfig.h>
#include <global/pid/RichTrackConfig.h>


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
    digi_cfg.pixelSize       = 3.0; // [mm]
    digi_cfg.safetyFactor    = 0.7;
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

    // track propagation to each radiator
    RichTrackConfig track_cfg;
    track_cfg.numPlanes.insert({ "Aerogel", 5  });
    track_cfg.numPlanes.insert({ "Gas",     10 });


    // wiring between factories and data ///////////////////////////////////////
    // clang-format off

    // digitization
    app->Add(new JChainMultifactoryGeneratorT<PhotoMultiplierHitDigi_factory>(
          "DRICHRawHits",
          {"DRICHHits"},
          {"DRICHRawHits", "DRICHRawHitsAssociations"},
          digi_cfg,
          app
          ));

    // charged particle tracks
    app->Add(new JChainMultifactoryGeneratorT<RichTrack_factory>(
          "DRICHTracks",
          {"CentralCKFTrajectories"},
          {"DRICHAerogelTracks", "DRICHGasTracks"},
          track_cfg,
          app
          ));
    app->Add(new JChainFactoryGeneratorT<MergeTrack_factory>(
          {"DRICHAerogelTracks", "DRICHGasTracks"},
          "DRICHMergedTracks"
          ));

    // clang-format on
  }
}
