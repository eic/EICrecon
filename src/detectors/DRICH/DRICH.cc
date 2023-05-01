// Copyright (C) 2022, 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#include <JANA/JApplication.h>
#include <JANA/JFactoryGenerator.h>
#include <extensions/jana/JChainFactoryGeneratorT.h>

// factories
#include <global/digi/PhotoMultiplierHitDigi_factory.h>
#include <global/pid/RichTrack_factory.h>
// #include <global/pid/IrtParticleID_factory.h>

// algorithm configurations
#include <algorithms/digi/PhotoMultiplierHitDigiConfig.h>


extern "C" {
  void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);

    using namespace eicrecon;

    // Digitization
    PhotoMultiplierHitDigiConfig digi_cfg;
    digi_cfg.seed            = 37;
    digi_cfg.hitTimeWindow   = 20.0; // [ns]
    digi_cfg.timeStep        = 0.0625; // [ns]
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
    app->Add(new JChainFactoryGeneratorT<PhotoMultiplierHitDigi_factory>({"DRICHHits"}, "DRICHRawHits", digi_cfg));

    // Track Propagation to each radiator
    // FIXME: algorithm configuration currently set in RichTrack factory; need to write independent RichTrack algorithm
    app->Add(new JChainFactoryGeneratorT<RichTrack_factory>({"CentralCKFTrajectories"}, "DRICHAerogelTracks"));
    app->Add(new JChainFactoryGeneratorT<RichTrack_factory>({"CentralCKFTrajectories"}, "DRICHGasTracks"));

    /* TODO: transform PhotoElectrons to Cherenkov Particle Identification
     * - Run the Indirect Ray Tracing (IRT) algorithm
     * - Cherenkov angle measurement
     * - PID hypotheses
     */
    // app->Add(new JFactoryGeneratorT<IrtParticleID_factory>());
  }
}
