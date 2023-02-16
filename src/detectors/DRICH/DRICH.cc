// Copyright 2022, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>
#include <JANA/JFactoryGenerator.h>
#include <extensions/jana/JChainFactoryGeneratorT.h>

// factories
#include <global/digi/PhotoMultiplierHitDigi_factory.h>
#include <global/pid/RichTrack_factory.h>
#include <global/pid/IrtCherenkovParticleID_factory.h>

// algorithm configurations
#include <algorithms/digi/PhotoMultiplierHitDigiConfig.h>
#include <algorithms/pid/IrtCherenkovParticleIDConfig.h>


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
    digi_cfg.timeStep        = 16.0; // [ns^-1]
    digi_cfg.speMean         = 80.0;
    digi_cfg.speError        = 16.0;
    digi_cfg.pedMean         = 200.0;
    digi_cfg.pedError        = 3.0;
    digi_cfg.enablePixelGaps = true;
    digi_cfg.pixelSize       = 3.0; // [mm]
    digi_cfg.safetyFactor    = 0.7;
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

    // track propagation to each radiator
    /* FIXME: algorithm configuration currently set in RichTrack factory; need
     *        to write independent RichTrack algorithm with a config class, then we
     *        can move the user-level settings to here
     * FIXME: set irt_cfg.radiators.at(*).zbins below to match RichTrack config
     */

    // PID
    IrtCherenkovParticleIDConfig irt_cfg;
    // - refractive index interpolation
    irt_cfg.numRIndexBins = 100;
    // - aerogel
    irt_cfg.radiators.insert({"Aerogel", RadiatorConfig{}});
    irt_cfg.radiators.at("Aerogel").zbins           = 5;
    irt_cfg.radiators.at("Aerogel").referenceRIndex = 1.0190;
    irt_cfg.radiators.at("Aerogel").attenuation     = 48; // [mm]
    irt_cfg.radiators.at("Aerogel").smearingMode    = "gaussian";
    irt_cfg.radiators.at("Aerogel").smearing        = 2e-3; // [radians]
    // - gas
    irt_cfg.radiators.insert({"Gas", RadiatorConfig{}});
    irt_cfg.radiators.at("Gas").zbins           = 10;
    irt_cfg.radiators.at("Gas").referenceRIndex = 1.00076;
    irt_cfg.radiators.at("Gas").attenuation     = 0; // [mm]
    irt_cfg.radiators.at("Gas").smearingMode    = "gaussian";
    irt_cfg.radiators.at("Gas").smearing        = 5e-3; // [radians]
    // - PDG list
    irt_cfg.pdgList.insert(irt_cfg.pdgList.end(), { -11, 211, 321, 2212 });
    // - cheat modes
    irt_cfg.cheatPhotonVertex  = true;
    irt_cfg.cheatTrueRadiator  = true;
    irt_cfg.cheatPhotonPinning = true;


    // wiring between factories and data ///////////////////////////////////////

    // digitization
    app->Add(new JChainFactoryGeneratorT<PhotoMultiplierHitDigi_factory>(
          {"DRICHHits"},
          "DRICHRawHits",
          digi_cfg
          ));
    // track projections
    app->Add(new JChainFactoryGeneratorT<RichTrack_factory>(
          {"CentralCKFTrajectories"},
          "DRICHAerogelTracks"
          ));
    app->Add(new JChainFactoryGeneratorT<RichTrack_factory>(
          {"CentralCKFTrajectories"},
          "DRICHGasTracks"
          ));
    // PID
    app->Add(new JChainFactoryGeneratorT<IrtCherenkovParticleID_factory>(
          {"DRICHRawHits","DRICHAerogelTracks","DRICHGasTracks"},
          "DRICHIrtCherenkovParticleID",
          irt_cfg
          ));
  }
}
