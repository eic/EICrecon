// Copyright (C) 2022, 2023, Christopher Dilks, Luigi Dello Stritto
// Subject to the terms in the LICENSE file found in the top-level directory.

#include <JANA/JApplication.h>
#include <JANA/JFactoryGenerator.h>
#include <extensions/jana/JChainFactoryGeneratorT.h>
#include <extensions/jana/JChainMultifactoryGeneratorT.h>

// factories
#include <global/digi/PhotoMultiplierHitDigi_factory.h>
#include <global/pid/RichTrack_factory.h>

// algorithm configurations
#include <algorithms/digi/PhotoMultiplierHitDigiConfig.h>


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
    digi_cfg.enableNoise     = false;
    digi_cfg.noiseRate       = 20000; // [Hz]
    digi_cfg.noiseTimeWindow = 20.0 * dd4hep::ns; // [ns]
    digi_cfg.quantumEfficiency.clear();
    digi_cfg.quantumEfficiency.push_back({325, 0.04}); // wavelength units are [nm]
    digi_cfg.quantumEfficiency.push_back({340, 0.10});
    digi_cfg.quantumEfficiency.push_back({350, 0.20});
    digi_cfg.quantumEfficiency.push_back({370, 0.30});
    digi_cfg.quantumEfficiency.push_back({400, 0.35});
    digi_cfg.quantumEfficiency.push_back({450, 0.40});
    digi_cfg.quantumEfficiency.push_back({500, 0.38});
    digi_cfg.quantumEfficiency.push_back({550, 0.35});
    digi_cfg.quantumEfficiency.push_back({600, 0.27});
    digi_cfg.quantumEfficiency.push_back({650, 0.20});
    digi_cfg.quantumEfficiency.push_back({700, 0.15});
    digi_cfg.quantumEfficiency.push_back({750, 0.12});
    digi_cfg.quantumEfficiency.push_back({800, 0.08});
    digi_cfg.quantumEfficiency.push_back({850, 0.06});
    digi_cfg.quantumEfficiency.push_back({900, 0.04});


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
    app->Add(new JChainFactoryGeneratorT<RichTrack_factory>(
          {"CentralCKFTrajectories"},
          "DRICHAerogelTracks"
          ));
    app->Add(new JChainFactoryGeneratorT<RichTrack_factory>(
          {"CentralCKFTrajectories"},
          "DRICHGasTracks"
          ));

    // clang-format on
  }
}
