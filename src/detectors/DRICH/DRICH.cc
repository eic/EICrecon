// Copyright (C) 2022, 2023, Christopher Dilks, Luigi Dello Stritto
// Subject to the terms in the LICENSE file found in the top-level directory.

#include <JANA/JApplication.h>
#include <JANA/JFactoryGenerator.h>
#include <extensions/jana/JChainFactoryGeneratorT.h>
#include <extensions/jana/JChainMultifactoryGeneratorT.h>

// factories
#include <global/digi/PhotoMultiplierHitDigi_factory.h>
#include <global/pid/RichTrack_factory.h>
#include <global/pid/MergeTrack_factory.h>
#include <global/pid/IrtCherenkovParticleID_factory.h>
#include <global/pid/MergeCherenkovParticleID_factory.h>

// algorithm configurations
#include <algorithms/digi/PhotoMultiplierHitDigiConfig.h>
#include <global/pid/RichTrackConfig.h>
#include <algorithms/pid/IrtCherenkovParticleIDConfig.h>
#include <algorithms/pid/MergeParticleIDConfig.h>

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
    track_cfg.numPlanes.insert({ "Gas",     30 });

    // IRT PID
    IrtCherenkovParticleIDConfig irt_cfg;
    // - refractive index interpolation
    irt_cfg.numRIndexBins = 100;
    // - aerogel
    irt_cfg.radiators.insert({"Aerogel", RadiatorConfig{}});
    irt_cfg.radiators.at("Aerogel").referenceRIndex = 1.0190;
    irt_cfg.radiators.at("Aerogel").attenuation     = 48; // [mm]
    irt_cfg.radiators.at("Aerogel").smearingMode    = "gaussian";
    irt_cfg.radiators.at("Aerogel").smearing        = 2e-3; // [radians]
    // - gas
    irt_cfg.radiators.insert({"Gas", RadiatorConfig{}});
    irt_cfg.radiators.at("Gas").referenceRIndex = 1.00076;
    irt_cfg.radiators.at("Gas").attenuation     = 0; // [mm]
    irt_cfg.radiators.at("Gas").smearingMode    = "gaussian";
    irt_cfg.radiators.at("Gas").smearing        = 5e-3; // [radians]
    // - PDG list
    irt_cfg.pdgList.insert(irt_cfg.pdgList.end(), { 11, 211, 321, 2212 });
    // - cheat modes
    irt_cfg.cheatPhotonVertex  = true;
    irt_cfg.cheatTrueRadiator  = true;

    // Merge PID from radiators
    MergeParticleIDConfig merge_cfg;
    merge_cfg.mergeMode = MergeParticleIDConfig::kAddWeights;

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

    // PID algorithm
    app->Add(new JChainMultifactoryGeneratorT<IrtCherenkovParticleID_factory>(
          "DRICHIrtCherenkovParticleID",
          {
            "DRICHAerogelTracks", "DRICHGasTracks", "DRICHMergedTracks",
            "DRICHRawHits",
            "DRICHRawHitsAssociations"
          },
          {"DRICHAerogelIrtCherenkovParticleID", "DRICHGasIrtCherenkovParticleID"},
          irt_cfg,
          app
          ));

    // merge aerogel and gas PID results
    app->Add(new JChainFactoryGeneratorT<MergeCherenkovParticleID_factory>(
          {"DRICHAerogelIrtCherenkovParticleID", "DRICHGasIrtCherenkovParticleID"},
          "DRICHMergedIrtCherenkovParticleID",
          merge_cfg
          ));

    // clang-format on
  }
}
