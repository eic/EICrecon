// Copyright 2022, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>
#include <JANA/JFactoryGenerator.h>
#include <extensions/jana/JChainFactoryGeneratorT.h>

#include <global/digi/PhotoMultiplierHitDigi_factory.h>
// #include <global/pid/RichTrack_factory.h>
// #include <global/pid/IrtCherenkovParticleID_factory.h>

extern "C" {
  void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);

    using namespace eicrecon;

    // Digitization
    app->Add(new JChainFactoryGeneratorT<PhotoMultiplierHitDigi_factory>({"DRICHHits"}, "DRICHRawHits"));

    // // Track Propagation to each radiator
    // app->Add(new JChainFactoryGeneratorT<RichTrack_factory>({"CentralCKFTrajectories"}, "DRICHAerogelTracks"));
    // app->Add(new JChainFactoryGeneratorT<RichTrack_factory>({"CentralCKFTrajectories"}, "DRICHGasTracks"));

    // // PID
    // app->Add(new JChainFactoryGeneratorT<IrtCherenkovParticleID_factory>(
    //       {"DRICHRawHits","DRICHAerogelTracks","DRICHGasTracks"},
    //       "DRICHIrtCherenkovParticleID"
    //       ));
  }
}
